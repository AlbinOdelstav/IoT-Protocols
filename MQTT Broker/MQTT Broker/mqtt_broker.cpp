#include "mqtt_broker.h"

void MqttBroker::start(const unsigned int port) {
	Socket listenSocket;

	if (listenSocket.create(port) != 0) {
		return;
	}

	while (true) {
		Socket socket = listenSocket.listen();
		if (socket.getSOCKET() != INVALID_SOCKET) {
			std::thread thread(&MqttBroker::handleClient, this, MqttClient(socket));
			thread.detach();
		} else {
			std::cout << "Accept failed with error: " << WSAGetLastError() << "\n";
			WSACleanup();
		}
	}
}

void MqttBroker::handleClient(MqttClient& client) {
	std::pair<Bytes, short> data = client.recv();

	if (data.second != 0) {
		client.close();
		return;
	}

	if (peekType(data.first[0]) == Type::CONNECT) {
		if (handleConnection(client, decodeConnect(data.first)) != CONNECT_ACCEPTED) {
			client.close();
			return;
		}
	}
	else {
		client.close();
		return;
	}

	while (true) {
		const unsigned short FIXED_HEADER_LENGTH = 2;
		std::pair<Bytes, short> totalData = client.recv();
		if (totalData.second != 0) {
			client.close();
			return;
		}
		auto dataBegin = totalData.first.begin();
		auto dataEnd = (dataBegin + *(dataBegin + 1) + FIXED_HEADER_LENGTH);

		while (dataEnd < totalData.first.end()) {
			Bytes data(dataBegin, dataEnd);

			switch (peekType(data[0])) {
			case Type::SUBSCRIBE:
				if (handleSubscribe(client, decodeSubscribe(data)) != 0) {
					client.close();
					return;
				};
				break;

			case Type::UNSUBSCRIBE:
				if (handleUnsubscribe(client, decodeUnsubscribe(data)) != 0) {
					client.close();
					return;
				};
				break;

			case Type::PUBLISH:
				if (handlePublish(client, data) != 0) {
					client.close();
					return;
				}
				break;

			case Type::PINGREQ:
				handlePing(client);
				break;

			case Type::DISCONNECT:
				client.close();
				return;
			}
			if (!(dataEnd + 1 < totalData.first.end())) break;
			dataBegin = dataEnd;
			dataEnd = (dataBegin + *(dataBegin + 1) + FIXED_HEADER_LENGTH);
		}
	}
}

int MqttBroker::handleConnection(MqttClient& client, ConnectMessage conMsg) {
	ConnectAckMessage connackMsg;
	connackMsg.type = Type::CONNACK;
	connackMsg.returnCode = CONNECT_ACCEPTED;

	if (conMsg.protocolName != "MQTT" || conMsg.protocolLevel != PROTOCOL_LEVEL) {
		connackMsg.returnCode = CONNECT_UNACCEPTABLE_PROTOCOL_VERSION;
	}

	if (!validateClientId(conMsg.clientId)) {
		connackMsg.returnCode = CONNECT_UNACCEPTABLE_IDENTIFIER;
	}

	connackMsg.sessionPresent = 1;
	client.setClientId(conMsg.clientId);
	client.send(encodeConnectAck(connackMsg));
	return connackMsg.returnCode;
}

int MqttBroker::handleSubscribe(MqttClient& client, SubscribeMessage subMsg) {
	SubscribeAckMessage subackMsg;
	subackMsg.type = Type::SUBACK;
	subackMsg.packetIdentifier = subMsg.packetIdentifier;

	if (subMsg.headerReserved != 2) {
		return -1;
	}

	subMtx.lock();
	std::vector<Subscribe_return_codes> returnCodes;
	for (const auto& topic : subMsg.topics) {
		if (0 < topic.second < 3) {
			if (topic.first.find('#') == std::string::npos && topic.first.find('+') == std::string::npos) {
				subscriptions[topic.first].push_back(std::make_pair(client, topic.second));
				returnCodes.push_back((Subscribe_return_codes)topic.second);
			}
			else {
				returnCodes.push_back(SUBSCRIBE_FAILURE);
			}
		}
		else {
			returnCodes.push_back(SUBSCRIBE_FAILURE);
		}
	}
	subMtx.unlock();

	subackMsg.returnCodes = returnCodes;
	client.send(encodeSubscribeAck(subackMsg));

	for (Topic topic : subMsg.topics) {
		if (retainedMessages.find(topic.first) != retainedMessages.end()) {
			client.send(retainedMessages[topic.first]);
		}
	}

	return 0;
}

int MqttBroker::handleUnsubscribe(MqttClient& client, UnsubscribeMessage unsubMsg) {
	if (unsubMsg.headerReserved != 2) {
		return -1;
	}

	UnsubscribeAckMessage sendMsg;
	sendMsg.type = Type::UNSUBACK;
	sendMsg.packetIdentifier = unsubMsg.packetIdentifier;

	subMtx.lock();
	for (auto& topic : unsubMsg.topics) {
		for (size_t i = 0; i < subscriptions[topic].size(); ++i) {
			if (subscriptions[topic][i].first == client) {
				subscriptions[topic].erase(subscriptions[topic].begin() + i);
				break;
			}
		}
	}
	subMtx.unlock();

	client.send(encodeUnsubscribeAck(sendMsg));
	return 0;
}

int MqttBroker::handlePublish(MqttClient& client, Bytes& data) {
	PublishMessage msg = decodePublish(data);
	
	if (msg.qos == QOS_LEVEL_UNEXPECTED) {
		return QOS_LEVEL_UNEXPECTED;
	}

	// Dup flag should not be propagated
	data[0] = data[0] & (0b11110111);

	retainMtx.lock();
	if (msg.retain) {
		retainedMessages[msg.topic] = data;
	}
	retainMtx.unlock();
	
	for (auto& subscription : subscriptions[msg.topic]) {
		subscription.first.send(data);
	}
	return 0;
}

void MqttBroker::handlePing(MqttClient& client) {
	Message sendMsg;
	sendMsg.type = PINGRESP;
	Bytes data = encodeMqttMessage(sendMsg);
	client.send(data);
}

Type MqttBroker::peekType(const Byte& data) {
	return (Type)((data & MASK_TYPE) >> SHIFT_TYPE);
}

uint8_t MqttBroker::peekMessageLength(const Bytes& bytes) {
	return bytes[1];
}

MqttBroker::MqttBroker() {}

MqttBroker::~MqttBroker() {}

MqttBroker:: ConnectMessage MqttBroker::decodeConnect(Bytes& data) {
	ConnectMessage msg;
	auto bytePtr = data.begin();
	msg.type = (Type)((*bytePtr & MASK_TYPE) >> SHIFT_TYPE);

	const short messageLength = *++bytePtr;
	const auto messageEnd = data.begin() + messageLength;
	const auto protocolLength = (*++bytePtr << 8) | *++bytePtr;
	msg.protocolName = std::string(bytePtr, (++bytePtr + protocolLength));
	bytePtr = bytePtr + protocolLength;

	msg.protocolLevel =(*bytePtr);
	msg.flags.username = ((*++bytePtr & MASK_FLAG_USERNAME) << SHIFT_FLAG_USERNAME);
	msg.flags.password =((*bytePtr & MASK_FLAG_PASSWORD) << SHIFT_FLAG_PASSWORD);
	msg.flags.willRetain = ((*bytePtr & MASK_FLAG_WILL_RETAIN) << SHIFT_FLAG_WILL_RETAIN);
	msg.flags.willQos = ((*bytePtr & MASK_FLAG_WILL_QOS) << SHIFT_FLAG_WILL_QOS);
	msg.flags.will = ((*bytePtr & MASK_FLAG_WILL) << SHIFT_FLAG_WILL);
	msg.flags.cleanSession = ((*bytePtr & MASK_FLAG_CLEAN_SESSION) << SHIFT_FLAG_CLEAN_SESSION);
	msg.flags.reserved = (*bytePtr & MASK_FLAG_RESERVED);
	msg.keepAlive = ((*++bytePtr << 8) | *++bytePtr);

	uint8_t clientIdLength = (*++bytePtr << 8) | *++bytePtr;

	if (clientIdLength > 0) {
		bytePtr++;
		msg.clientId = std::string(bytePtr, bytePtr + clientIdLength);
		bytePtr = bytePtr + clientIdLength-1;
	} 

	if (msg.flags.will) {
		uint8_t willLength = (*++bytePtr << 8) | *++bytePtr;
		if (willLength > 0) {
			bytePtr++;
			msg.clientId = std::string(bytePtr, bytePtr + clientIdLength);
			bytePtr = bytePtr + clientIdLength - 1;
		}
	}

	if (msg.flags.username) {
		uint8_t usernameLength = (*++bytePtr << 8) | *++bytePtr;

		if (usernameLength > 0) {
			bytePtr++;
			msg.username = std::string(bytePtr, bytePtr + usernameLength);
			bytePtr = bytePtr + usernameLength - 1;
		}
	}

	if (msg.flags.password) {
		uint8_t passwordLength = (*++bytePtr << 8) | *++bytePtr;

		if (passwordLength > 0) {
			bytePtr++;
			msg.password = std::string(bytePtr, bytePtr + passwordLength);
		}
	}
	return msg;
}

MqttBroker::SubscribeMessage MqttBroker::decodeSubscribe(Bytes& data) {
	auto bytePtr = data.begin();

	SubscribeMessage msg;
	msg.type = Type((*bytePtr & MASK_TYPE) >> SHIFT_TYPE);
	msg.headerReserved = (Type(*bytePtr & MASK_FIXED_HEADER_RESERVED));

	const short messageLength = *++bytePtr;
	const auto messageEnd = (data.begin() + 2) + messageLength;
	msg.packetIdentifier = ((*++bytePtr << 8) | *++bytePtr);

	std::vector<Topic> topics;
	while (++bytePtr < messageEnd) {
		const uint16_t topicLength = (*bytePtr << 8) | *(++bytePtr);
		const auto topicEnd = ++bytePtr + topicLength;
		const std::string topicName(bytePtr, topicEnd);
		bytePtr = topicEnd;
		const short qos = *bytePtr;
		topics.push_back(std::make_pair(topicName, qos));
	}
	msg.topics = topics;
	return msg;
}

MqttBroker::UnsubscribeMessage MqttBroker::decodeUnsubscribe(Bytes& data) {
	auto bytePtr = data.begin();

	UnsubscribeMessage msg;
	msg.type = (Type((*bytePtr & MASK_TYPE) >> SHIFT_TYPE));
	msg.headerReserved = (Type(*bytePtr & MASK_FIXED_HEADER_RESERVED));

	const short messageLength = *++bytePtr;
	const auto messageEnd = (data.begin() + 2) + messageLength;
	msg.packetIdentifier = (*++bytePtr << 8) | *++bytePtr;

	std::vector<std::string> topics;
	while (++bytePtr < messageEnd) {
		const uint16_t topicLength = (*bytePtr << 8) | *++bytePtr;
		const auto topicEnd = ++bytePtr + topicLength;
		topics.push_back(std::string(bytePtr, topicEnd));
		bytePtr = topicEnd -1;
	}
	msg.topics = topics;
	return msg;
}

MqttBroker::PublishMessage MqttBroker::decodePublish(Bytes& data) {
	PublishMessage msg;
	auto bytePtr = data.begin();

	msg.type = ((Type)((*bytePtr & MASK_TYPE) >> SHIFT_TYPE));
	msg.dup = (((*bytePtr & MASK_DUP) >> SHIFT_DUP));
	msg.qos = ((*bytePtr & MASK_QOS) >> SHIFT_QOS);
	msg.retain = (*bytePtr & MASK_RETAIN);

	const unsigned int messageLength = *++bytePtr;
	const auto messageEnd = (data.begin() + 2) + messageLength;
	const unsigned int topicLength = (*++bytePtr << 8) | *(++bytePtr);
	
	auto topicEnd = ++bytePtr + topicLength;

	msg.topic = std::string(bytePtr, topicEnd);
	if (msg.qos == QOS_LEVEL_1 || msg.qos == QOS_LEVEL_2) {
		bytePtr = topicEnd;
		msg.packetIdentifier = (*bytePtr << 8) | *++bytePtr;
	}

	return msg;
}

// Returns false if str contains other characters than
// 0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ
bool MqttBroker::validateClientId(std::string& str) {
	for (char c : str) {
		if (!std::isalpha(c) && (c < '0' || c > '9'))
			return false;
	}
	return true;
}

Bytes MqttBroker::encodeMqttMessage(Message& msg) {
	Bytes data;
	data.push_back(msg.type << SHIFT_TYPE);

	if (msg.packetIdentifier) {
		data.push_back(2);
		data.push_back((msg.packetIdentifier & 0b1111111100000000) >> 8);
		data.push_back(msg.packetIdentifier & 0b0000000011111111);
	}
	else {
		data.push_back(0);
	}

	return data;
}

Bytes MqttBroker::encodeConnectAck(ConnectAckMessage& msg) {
	Bytes data;
	data.push_back(msg.type << SHIFT_TYPE);
	data.push_back(2);
	data.push_back(0);
	data.push_back(msg.returnCode);
	return data;
}

Bytes MqttBroker::encodeSubscribeAck(SubscribeAckMessage& msg) {
	Bytes data;
	data.push_back(msg.type << SHIFT_TYPE);

	const uint8_t packetIdentifierMsb = (msg.packetIdentifier & 0b1111111100000000) >> 8;
	const uint8_t packetIdentifierLsb = msg.packetIdentifier & 0b0000000011111111;

	data.push_back(packetIdentifierMsb);
	data.push_back(packetIdentifierLsb);
	for (auto returnCode : msg.returnCodes) {
		data.push_back(returnCode);
	}

	const short messageLength = data.size() - 1;
	data.insert(data.begin() + 1, messageLength);

	return data;
}

Bytes MqttBroker::encodeUnsubscribeAck(UnsubscribeAckMessage& msg) {
	Bytes data;
	data.push_back(msg.type << SHIFT_TYPE);

	const uint8_t packetIdentifierMsb = (msg.packetIdentifier & 0b1111111100000000) >> 8;
	const uint8_t packetIdentifierLsb = msg.packetIdentifier & 0b0000000011111111;

	data.push_back(packetIdentifierMsb);
	data.push_back(packetIdentifierLsb);
	for (auto returnCode : msg.returnCodes) {
		data.push_back(returnCode);
	}

	const short messageLength = data.size() - 1;
	data.insert(data.begin() + 1, messageLength);

	return data;
}