#include "mqtt_broker.h"

void MqttBroker::start(const unsigned int port) {

	Socket masterSocket;
	if (masterSocket.create(port) != 0) {
		return;
	}

	while (true) {
		Socket socket = masterSocket.listen();
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

	while (true) {
		Bytes totalData = client.recv();
		auto dataBegin = totalData.begin();
		auto dataEnd = (dataBegin + *(dataBegin + 1) + FIXED_HEADER_LENGTH);

		while (dataEnd < totalData.end()) {
			Bytes data(dataBegin, dataEnd);

			switch (peekType(data[0])) {
			case Type::SUBSCRIBE:
				std::cout << "Subscribe\n";
				handleSubscription(client, decodeSubscribe(data));
				break;

			case Type::UNSUBSCRIBE:
				std::cout << "Unsubscribe";
				break;

			case Type::PUBLISH:
				std::cout << "Publish";
				// handlePublish(client, decodePublish(data));
				handlePublish(client, data);
				break;

			case Type::PINGREQ:
				std::cout << "Ping\n";
				handlePing(client);
				break;

			case Type::DISCONNECT:
				std::cout << "Disconnect";
				// unsubscsribe

				client.close();
				return;

			case Type::CONNECT:
				std::cout << "Connect\n";
				handleConnection(client, decodeConnect(data));
			}

			if (!(dataEnd + 1 < totalData.end())) break;
			dataBegin = dataEnd;
			dataEnd = (dataBegin + *(dataBegin + 1) + FIXED_HEADER_LENGTH);
		}
	}
}

void MqttBroker::handleConnection(MqttClient& client, ConnectMessage recvMsg) {
	ConnectAckMessage sendMsg;
	sendMsg.setType(Type::CONNACK);
	client.send(encodeConnectAck(sendMsg));
}

void MqttBroker::handleSubscription(MqttClient client, SubscribeMessage subMsg) {
	SubscribeAckMessage sendMsg;
	sendMsg.setType(Type::SUBACK);
	sendMsg.setPacketIdentifier(subMsg.getPacketIdentifier());

	std::vector<int> returnCodes;
	for (auto topic : subMsg.getTopics()) {
		if (0 < topic.second < 3) {
			subscriptions[topic.first].push_back(std::make_pair(client, topic.second));
			returnCodes.push_back(topic.second);
		}
		else {
			returnCodes.push_back(128); // Magic failure, create enum maybe
		}
	}
	sendMsg.setReturnCodes(returnCodes);
	client.send(encodeSubscribeAck(sendMsg));
}

void MqttBroker::handlePublish(MqttClient& client, Bytes& data) {
	PublishMessage msg = decodePublish(data);
	for (auto subscription : subscriptions[msg.getTopic()]) {
		subscription.first.send(data);
	}
}

void MqttBroker::handlePing(MqttClient client) {
	MqttMessage sendMsg;
	sendMsg.setType(Type::PINGRESP);
	Bytes data(2);
	data[0] = encodeMqttMessage(sendMsg);
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

ConnectMessage MqttBroker::decodeConnect(Bytes& data) {
	ConnectMessage msg;
	msg.setType((Type)((data[0] & MASK_TYPE) >> SHIFT_TYPE));
	msg.setDup(((data[0] & MASK_DUP) >> SHIFT_DUP));
	msg.setQos((data[0] & MASK_QOS) >> SHIFT_QOS);
	msg.setRetain(data[0] & MASK_RETAIN);

	std::cout << "Decoding connect\n";

	const short remainingLength = data[1];
	msg.setMsb(data[2]);
	msg.setLsb(data[3]);
	const short protocolNameBegin = 4;
	const short protocolNameEnd = protocolNameBegin + msg.getLsb();
	msg.setProtocolName(std::string(data.begin() + protocolNameBegin, data.begin() + protocolNameEnd));

	return msg;
}

SubscribeMessage MqttBroker::decodeSubscribe(Bytes& data) {
	std::cout << "Decoding subscribe\n";
	auto bytePtr = data.begin();

	SubscribeMessage msg;
	msg.setType((Type)((*bytePtr & MASK_TYPE) >> SHIFT_TYPE));

	const short messageLength = *++bytePtr;
	const auto messageEnd = data.begin() + messageLength;
	msg.setPacketIdentifier((*++bytePtr << 8) | *++bytePtr);

	std::vector<Topic> topics;
	while (++bytePtr < messageEnd) {
		const uint16_t topicLength = (*bytePtr << 8) | *(++bytePtr);
		auto topicEnd = ++bytePtr + topicLength;
		const std::string topicName(bytePtr, topicEnd);
		bytePtr = topicEnd;
		const short qos = *bytePtr;
		topics.push_back(std::make_pair(topicName, qos));
	}
	msg.setTopics(topics);
	return msg;
}

PublishMessage MqttBroker::decodePublish(Bytes& data) {
	PublishMessage msg;
	auto bytePtr = data.begin();

	msg.setType((Type)((*bytePtr & MASK_TYPE) >> SHIFT_TYPE));
	msg.setDup(((*bytePtr & MASK_DUP) >> SHIFT_DUP));
	msg.setQos((*bytePtr & MASK_QOS) >> SHIFT_QOS);
	msg.setRetain(*bytePtr & MASK_RETAIN);

	const short messageLength = *++bytePtr;
	const auto messageEnd = data.begin() + messageLength;

	const uint16_t topicLength = (*++bytePtr << 8) | *(++bytePtr);
	auto topicEnd = ++bytePtr + topicLength;

	msg.setTopic(std::string(bytePtr, topicEnd));
	return msg;
}

Byte MqttBroker::encodeMqttMessage(MqttMessage& msg) {
	Byte data = 0;
	data = msg.getType() << SHIFT_TYPE;
	return data;
}

Bytes MqttBroker::encodeConnectAck(ConnectAckMessage& msg) {
	Bytes data;
	data.push_back(msg.getType() << SHIFT_TYPE);
	data.push_back(2);
	data.push_back(0);
	data.push_back(0);
	return data;
}

Bytes MqttBroker::encodeSubscribeAck(SubscribeAckMessage& msg) {
	Bytes data;
	data.push_back(msg.getType() << SHIFT_TYPE);

	const uint8_t packetIdentifierMsb = (msg.getPacketIdentifier() & 0b1111111100000000) >> 8;
	const uint8_t packetIdentifierLsb = msg.getPacketIdentifier() & 0b0000000011111111;

	data.push_back(packetIdentifierMsb);
	data.push_back(packetIdentifierLsb);
	for (auto returnCode : msg.getReturnCodes()) {
		data.push_back(returnCode);
	}

	const short messageLength = data.size() - 1;
	data.insert(data.begin() + 1, messageLength);

	return data;
}