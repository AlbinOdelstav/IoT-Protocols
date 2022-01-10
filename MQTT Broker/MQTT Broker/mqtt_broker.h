#pragma once
#include <iostream>
#include <thread>
#include <vector>>
#include "mqtt_header_codes.h"
#include "mqtt_client.h"
#include "socket.h"
#include <map>
#include <bitset>
#include <mutex>

using Subscription = std::pair<MqttClient, int>;
using Topic = std::pair<std::string, int>;
using Bytes = std::vector<unsigned char>;
using Byte = unsigned char;

class MqttBroker {
public:

	MqttBroker();
	~MqttBroker();

	void start(const unsigned int port);

private:
	struct Message {
		Type type;
		uint16_t packetIdentifier;
	};

	struct ConnectMessage : Message {
		std::string protocolName;
		short protocolLevel;
		struct Flags {
			bool username;
			bool password;
			bool willRetain;
			short willQos;
			bool will;
			bool cleanSession;
			bool reserved;
		} flags;

		uint8_t keepAlive;
		std::string clientId = "";
		std::string willTopic = "";
		std::string username = "";
		std::string password = "";
	};

	struct SubscribeMessage : Message {
		uint16_t headerReserved;
		std::vector<Topic> topics;
	};

	struct SubscribeAckMessage : Message {
		std::vector<Subscribe_return_codes> returnCodes;
	};

	struct UnsubscribeMessage : Message {
		uint16_t headerReserved;
		std::vector<std::string> topics;
	};

	struct UnsubscribeAckMessage : Message {
		std::vector<Subscribe_return_codes> returnCodes;
	};

	struct ConnectAckMessage : Message {
		bool sessionPresent;
		uint8_t returnCode;
	};

	struct PublishMessage : Message {
		bool dup;
		unsigned short qos;
		bool retain;
		std::string topic;
		Bytes payload;
	};

	void handleClient(MqttClient&);
	int handleConnection(MqttClient&, ConnectMessage);
	int handleSubscribe(MqttClient&, SubscribeMessage);
	int handleUnsubscribe(MqttClient&, UnsubscribeMessage);
	int handlePublish(MqttClient&, Bytes&);
	void handlePing(MqttClient&);
	Type peekType(const Byte&);
	uint8_t peekMessageLength(const Bytes&);

	Bytes encodeMqttMessage(Message&);
	Bytes encodeConnectAck(ConnectAckMessage&);
	Bytes encodeSubscribeAck(SubscribeAckMessage&);
	Bytes encodeUnsubscribeAck(UnsubscribeAckMessage&);

	ConnectMessage decodeConnect(Bytes&);
	SubscribeMessage decodeSubscribe(Bytes&);
	UnsubscribeMessage decodeUnsubscribe(Bytes&);
	PublishMessage decodePublish(Bytes&);

	bool validateClientId(std::string&);

	std::map<std::string, std::vector<Subscription>> subscriptions;
	std::map<std::string, Bytes> retainedMessages;
	std::mutex subMtx;
	std::mutex retainMtx;
};