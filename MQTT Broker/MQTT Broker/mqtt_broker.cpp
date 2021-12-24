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

int MqttBroker::handleClient(MqttClient client) {
	MqttMessage recvNessage = decode(client.recv());
	MqttMessage sendMsg;

	if (recvNessage.getType() == CONNECT) {
		std::cout << "Connect\n";
		sendMsg.setType(CONNACK);
		client.send(encode(sendMsg));
	}

	sendMsg = MqttMessage();

	while (true) {
		recvNessage = decode(client.recv());

		switch (recvNessage.getType()) {
		case SUBSCRIBE:
			std::cout << "Subscribe\n";
			handleSubscription(client, recvNessage);
			break;

		case UNSUBSCRIBE:
			std::cout << "Unsubscribe";
			break;

		case PUBLISH:
			std::cout << "Publish";
			break;

		case PINGREQ:
			std::cout << "Ping\n";
			sendMsg.setType(PINGREQ);
			client.send(encode(sendMsg));
			break;

		case DISCONNECT:
			std::cout << "Dsiconnect";
			break;
		}
	}
}

void MqttBroker::handleSubscription(MqttClient client, MqttMessage subMsg) {
	MqttMessage ackMsg;
	ackMsg.setType(SUBACK);
	client.send(encode(ackMsg));
}

MqttBroker::MqttBroker() {}

MqttBroker::~MqttBroker(){}

MqttMessage MqttBroker::decode(std::vector<unsigned char> data) {
	MqttMessage msg;
	msg.setType((Type)((data[0] & MASK_TYPE) >> SHIFT_TYPE));
	msg.setDup(((data[0] & MASK_DUP) >> SHIFT_DUP));
	msg.setQos((data[0] & MASK_QOS) >> SHIFT_QOS);
	msg.setRetain(data[0] & MASK_RETAIN);

	std::cout << "Decoding: " << msg.getType() << "\n";

	switch (msg.getType()) {
	case SUBSCRIBE:
		const short msb = data[1];
		const short lsb = data[2];
		std::cout << msb << " : " << lsb << "\n";
		break;
	}

	return msg;
}

std::vector<unsigned char> MqttBroker::encode(MqttMessage& msg) {
	std::vector<unsigned char> data;
	data.push_back(msg.getType() << 4);
	
	switch (msg.getType()) {
	case CONNACK:
		data.push_back(2);
		data.push_back(0);
		data.push_back(0);
	case SUBACK:

		break;
	}

	return data;
}
