#include <iostream>
#include "mqtt_broker.h"

int main() {
	MqttBroker().start(1883);
}