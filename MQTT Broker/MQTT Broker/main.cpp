#include <iostream>
#include <bitset>
#include <vector>
#include "mqtt_broker.h"

int main() {
	MqttBroker().start(1883);

	std::string hello = "vad fan";
	std::string data(hello.begin(), hello.begin() + 7);
	std::cout << data << "\n";
	system("pause");
}