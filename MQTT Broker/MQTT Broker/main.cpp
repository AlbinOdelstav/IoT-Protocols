#include <iostream>
#include <bitset>
#include <vector>
#include "mqtt_broker.h"

int main() {
	MqttBroker().start(1883);

	std::vector<unsigned char> data;
	data.push_back(0b00000001);
	data.push_back(0b00000001);

	std::cout << std::bitset<16>((data[0] << 8) | data[1]) << "\n";
	system("pause");
}