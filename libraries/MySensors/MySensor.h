/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2015 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */
#ifndef MySensor_h
#define MySensor_h

#define VALUE_TO_STRING(x) #x
#define VALUE(x) VALUE_TO_STRING(x)
#define VAR_NAME_VALUE(var) #var "="  VALUE(var)
// #pragma message(VAR_NAME_VALUE(MY_GATEWAY_FEATURE))

#include "MyConfig.h"

#if defined(MY_GATEWAY_SERIAL) || defined(MY_GATEWAY_W5100) || defined(MY_GATEWAY_ENC28J60)
	#define MY_GATEWAY_FEATURE
	#define MY_NODE_TYPE "gateway"
#elif defined(MY_REPEATER_FEATURE)
	#define MY_NODE_TYPE "repeater"
#else
	#define MY_NODE_TYPE "sensor"
#endif


// HARDWARE
#if defined(ARDUINO_ARCH_ESP8266)
	#include "drivers/ESP8266/SPI/SPI.cpp"
	#include "core/MyHwESP8266.cpp"
#elif defined(ARDUINO_ARCH_AVR)
	#include "drivers/AVR/SPI/SPI.cpp"
	#include "core/MyHwATMega328.cpp"
#endif

// SIGNING
#if defined(MY_SIGNING_ATSHA204) || defined(MY_SIGNING_SOFT)
	#include "MySigning.cpp"
	#define MY_SIGNING_FEATURE
	#if defined(MY_SIGNING_ATSHA204)
		#include "MySigningAtsha204.cpp"
		#include "drivers/ATSHA204/ATSHA204.cpp"
		MySigningAtsha204 signer;
	#elif defined(MY_SIGNING_SOFT)
		#include "MySigningAtsha204Soft.cpp"
		#include "drivers/ATSHA204/sha256.cpp"
		MySigningAtsha204Soft signer;
	#endif
#endif

// RADIO
#if defined(MY_RADIO_NRF24) || defined(MY_RADIO_RFM69)
	#define MY_RADIO_FEATURE
	// SOFTSPI
	#ifdef MY_SOFTSPI
		#if defined(ARDUINO_ARCH_ESP8266)
			#error Soft SPI is not available on ESP8266
		#endif
		#include "drivers/AVR/DigitalIO/SoftI2cMaster.cpp"
		#include "drivers/AVR/DigitalIO/PinIO.cpp"
	#endif

	// FLASH
	#ifdef MY_OTA_FIRMWARE_FEATURE
		#include "drivers/SPIFlash/SPIFlash.cpp"
		SPIFlash _flash(MY_OTA_FLASH_SS, MY_OTA_FLASH_JDECID);
	#endif

	#if defined(MY_RADIO_NRF24)
		#include "drivers/RF24/RF24.cpp"
		#include "core/MyTransportNRF24.cpp"
	#elif defined(MY_RADIO_RFM69)
		#include "drivers/RFM69/RFM69.cpp"
		#include "core/MyTransportRFM69.cpp"
	#endif
#endif


// GATEWAY - TRANSPORT
#if defined(MY_GATEWAY_FEATURE)
	// GATEWAY - PROTOCOL
	#if defined(MY_GATEWAY_PROTOCOL_DEFAULT)
		#include "core/MyProtocolDefault.cpp"
	#else
		#error No gateway protocol specified!
	#endif
	#if defined(MY_RADIO_FEATURE)
		// We assume that a gateway having a radio also should act as repeater
		#define MY_REPEATER_FEATURE
	#endif
	#if defined(MY_CONTROLLER_IP_ADDRESS)
		#define MY_GATEWAY_CLIENT_MODE
	#endif
	#if !defined(MY_PORT)
		#error You must define MY_PORT (cotroller or gatway port to open)
	#endif

	#if defined(MY_GATEWAY_W5100)
		#include "drivers/AVR/Ethernet_W5100/utility/socket.cpp"
		#include "drivers/AVR/Ethernet_W5100/utility/w5100.cpp"
		#include "drivers/AVR/Ethernet_W5100/DNS.cpp"
		#include "drivers/AVR/Ethernet_W5100/Ethernet.cpp"
		#include "drivers/AVR/Ethernet_W5100/EthernetUdp.cpp"
		#include "drivers/AVR/Ethernet_W5100/IPAddress.cpp"

		#if !defined(USE_UDP)
			#include "drivers/AVR/Ethernet_W5100/EthernetServer.cpp"
		#endif

		#if defined(MY_GATEWAY_CLIENT_MODE)
			#include "drivers/AVR/Ethernet_W5100/EthernetClient.cpp"
			#include "core/MyGatewayTransportEthernet.cpp"
		#else
			// What do we do here?
		#endif
	#elif defined(MY_GATEWAY_ENC28J60)
		#undef MY_USE_UDP // Will not fit (or compile) on ENC28J60
		#include "drivers/AVR/Ethernet_UIP/src/utility/uipethernet-conf.h"
		#include "drivers/AVR/Ethernet_UIP/src/utility/uip-conf.h"
		#include "drivers/AVR/Ethernet_UIP/src/UIPEthernet.h"
		#include "drivers/AVR/Ethernet_UIP/src/utility/Enc28J60Network.cpp"
		#include "drivers/AVR/Ethernet_UIP/src/utility/mempool_conf.h"
		#include "drivers/AVR/Ethernet_UIP/src/utility/mempool.cpp"
		#include "drivers/AVR/Ethernet_UIP/src/UIPEthernet.cpp"
		#include "drivers/AVR/Ethernet_UIP/src/UIPServer.cpp"
		#include "drivers/AVR/Ethernet_UIP/src/UIPClient.cpp"
		#include "drivers/AVR/Ethernet_UIP/src/utility/clock-arch.c"
		#include "drivers/AVR/Ethernet_UIP/src/utility/uip_timer.c"
		#include "drivers/AVR/Ethernet_UIP/src/utility/uip.c"
		#include "drivers/AVR/Ethernet_UIP/src/utility/uip_arp.c"

		#include "core/MyGatewayTransportEthernet.cpp"
	#elif defined(MY_GATEWAY_SERIAL)
		#include "core/MyGatewayTransportSerial.cpp"
	#endif
#else
	#undef MY_GATEWAY_FEATURE
#endif

// Make sure to disable child features when parent feature is disabled
#if !defined(MY_RADIO_FEATURE)
	#undef MY_OTA_FIRMWARE_FEATURE
	#undef MY_REPEATER_FEATURE
	#undef MY_SECURE_NODE_WHITELISTING
	#undef MY_SIGNING_FEATURE
#endif

#if !defined(MY_GATEWAY_FEATURE)
	#undef MY_INCLUSION_MODE_FEATURE
	#undef MY_INCLUSION_BUTTON_FEATURE
#endif

// Defines used in code for if-statements
#if defined(MY_REPEATER_FEATURE)
	#define	MY_IS_REPEATER true
#else
	#define	MY_IS_REPEATER false
#endif

#if defined(MY_GATEWAY_FEATURE)
	#define	MY_IS_GATEWAY true
#else
	#define	MY_IS_GATEWAY false
#endif


#include "core/MyMessage.cpp"
#include "core/MySensorCore.cpp"

extern void setup();
extern void loop();

#include <Arduino.h>
// Initialize library and handle sketch functions like we want to
int main() {
	init();  // Init Arduino
	begin(); // Startup MySensors library
	setup(); // Call sketch setup
	while(1) {
		process();  // Process incoming data
		loop(); // Call sketch loop
		if (serialEventRun) serialEventRun();
	}
	return 0;
}
#endif
