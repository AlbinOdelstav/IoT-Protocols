#include "mqtt_broker.h"

void MqttBroker::start(const unsigned int port) {
	Socket listenSocket;
	if (listenSocket.create(port) != 0) {
		return;
	}

	while (true) {
		Socket socket = listenSocket.listen();
		if (socket.getSOCKET() == INVALID_SOCKET) {
			std::cout << "Accept failed with error: " << WSAGetLastError() << "\n";
			WSACleanup();
		}

		std::thread thread(&MqttBroker::handleClient, this, MqttClient(socket));
		thread.detach();
	}
}

void MqttBroker::handleClient(MqttClient client) {
	const unsigned short FIXED_HEADER_LENGTH = 2;
	Bytes data = client.recv();

	if (peekType(data[0]) == Type::CONNECT) {
		std::cout << "Connect\n";
		if (handleConnection(client, decodeConnect(data)) != CONNECT_ACCEPTED) {
			std::cout << "Connection failed\n";
			client.close();
			return;
		}
	}

	while (true) {
		Bytes totalData = client.recv();
		auto dataBegin = totalData.begin();
		auto dataEnd = (dataBegin + *(dataBegin + 1) + FIXED_HEADER_LENGTH);

		while (dataEnd < totalData.end()) {
			Bytes data(dataBegin, dataEnd);

			switch (peekType(data[0])) {
			case Type::SUBSCRIBE:
				std::cout << "Subscribe\n";
				if (handleSubscribe(client, decodeSubscribe(data)) != 0) {
					client.close();
					return;
				};
				break;

			case Type::UNSUBSCRIBE:
				std::cout << "Unsubscribe\n";
				if (handleUnsubscribe(client, decodeUnsubscribe(data)) != 0) {
					client.close();
					return;
				};
				break;

			case Type::PUBLISH:
				std::cout << "Publish\n";
				if (handlePublish(client, data) != 0) {
					client.close();
					return;
				}
				break;

			case Type::PINGREQ:
				std::cout << "Ping\n";
				handlePing(client);
				break;

			case Type::DISCONNECT:
				std::cout << "Disconnect\n";
				// unsubscsribe

				client.close();
				return;
			}
			if (!(dataEnd + 1 < totalData.end())) break;
			dataBegin = dataEnd;
			dataEnd = (dataBegin + *(dataBegin + 1) + FIXED_HEADER_LENGTH);
		}
	}
}

int MqttBroker::handleConnection(MqttClient& client, ConnectMessage conMsg) {
	ConnectAckMessage connackMsg;
	connackMsg.type = Type::CONNACK;

	// Fix these magic numbers before death hits
	if (conMsg.protocolName != "MQTT" || conMsg.protocolLevel != 4) {
		connackMsg.returnCode = CONNECT_UNACCEPTABLE_PROTOCOL_VERSION;
		return CONNECT_UNACCEPTABLE_PROTOCOL_VERSION;
	}

	if (!validateClientId(conMsg.clientId)) {
		connackMsg.returnCode = CONNECT_UNACCEPTABLE_IDENTIFIER;
		return CONNECT_UNACCEPTABLE_IDENTIFIER;
	}

	client.setClientId(conMsg.clientId);

	connackMsg.sessionPresent = 1;
	client.send(encodeConnectAck(connackMsg));
	return CONNECT_ACCEPTED;
}

int MqttBroker::handleSubscribe(MqttClient& client, SubscribeMessage subMsg) {
	SubscribeAckMessage subackMsg;
	subackMsg.type = Type::SUBACK;
	subackMsg.packetIdentifier = subMsg.packetIdentifier;

	if (subMsg.headerReserved != 2) {
		return -1;
	}

	std::vector<Subscribe_return_codes> returnCodes;
	for (auto topic : subMsg.topics) {
		if (0 < topic.second < 3) {
			subscriptions[topic.first].push_back(std::make_pair(client, topic.second));
			returnCodes.push_back((Subscribe_return_codes)topic.second);
		}
		else {
			returnCodes.push_back(SUBSCRIBE_FAILURE);
		}
	}
	
	// Dont forget wildcard

	subackMsg.returnCodes = returnCodes;
	client.send(encodeSubscribeAck(subackMsg));
	return 0;
}

int MqttBroker::handleUnsubscribe(MqttClient& client, UnsubscribeMessage unsubMsg) {
	std::cout << "handle unsubscribe\n";

	if (unsubMsg.headerReserved != 2) {
		// error
		return -1;
	}

	UnsubscribeAckMessage sendMsg;
	sendMsg.type = Type::UNSUBACK;
	sendMsg.packetIdentifier = unsubMsg.packetIdentifier;

	// Dont forget wildcard
	for (auto topic : unsubMsg.topics) {
		for (size_t i = 0; i < subscriptions[topic].size(); ++i) {
			if (subscriptions[topic][i].first == client) {
				std::cout << "found ya\n";
				subscriptions[topic].erase(subscriptions[topic].begin() + i);
				break;
			}
		}
	}

	client.send(encodeUnsubscribeAck(sendMsg));
	return 0;
}

int MqttBroker::handlePublish(MqttClient& client, Bytes& data) {
	PublishMessage msg = decodePublish(data);
	
	if (msg.qos == QOS_LEVEL_UNEXPECTED) {
		return QOS_LEVEL_UNEXPECTED;
	}

	if (msg.retain) {
		// wait for it
	}
	
	// Dup flag should not be propagated
	data[0] = data[0] & (0b11110111);
	for (auto subscription : subscriptions[msg.topic]) {
		subscription.first.send(data);
	}
	
	/*
	*			Hol' on
	* 
	* 
	if (msg.qos == QOS_LEVEL_1) {
		Message ackMsg { PUBACK, msg.packetIdentifier };
		client.send(encodeMqttMessage(ackMsg));
	}
	else if (msg.qos == QOS_LEVEL_2) {
		Message recMsg { PUBREC, msg.packetIdentifier };
		client.send(encodeMqttMessage(recMsg));

		// Wait for PUBREL
		Bytes data = client.recv();
		if (peekType(data[0]) == PUBREL) {
			Message compMsg{ PUBCOMP, msg.packetIdentifier };
			client.send(encodeMqttMessage(compMsg));
		}
	}
	*/

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
	std::cout << "Decoding connect\n\n";

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
	std::cout << "Decoding subscribe\n";
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
	std::cout << "Decoding unsubscribe\n";
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
	std::cout << "decode publish\n";

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
		if (!std::isalpha(c) && (c < 0 && c > 9))
			return false;
	}
	return true;
}

Bytes MqttBroker::encodeMqttMessage(Message& msg) {
	std::cout << "msg.packetIdentifier: " << msg.packetIdentifier << "\n";
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
	data.push_back(0);
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