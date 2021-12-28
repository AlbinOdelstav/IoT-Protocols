#include <iostream>
#include <bitset>
#include "mqtt_broker.h"
#include "mqtt_header_codes.h"

int main() {
	MqttBroker().start(1883);
}