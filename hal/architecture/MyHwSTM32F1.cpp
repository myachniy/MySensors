/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2017 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */

#include "MyHwSTM32F1.h"
#include <EEPROM.h>


/*
* Pinout STM32F103C8 dev board:
* http://wiki.stm32duino.com/images/a/ae/Bluepillpinout.gif
*
* Wiring RFM69 radio / SPI1
* --------------------------------------------------
* CLK	PA5
* MISO	PA6
* MOSI	PA7
* CSN	PA4
* CE	NA
* IRQ	PA3 (default)
*
* Wiring RF24 radio / SPI1
* --------------------------------------------------
* CLK	PA5
* MISO	PA6
* MOSI	PA7
* CSN	PA4
* CE	PB0 (default)
* IRQ	NA
*
*/
bool hwInit(void)
{
#if !defined(MY_DISABLED_SERIAL)
	MY_SERIALDEVICE.begin(MY_BAUD_RATE);
#endif
	if (EEPROM.init() == EEPROM_OK) {

		uint16 cnt;
		EEPROM.count(&cnt);
		if(cnt>=EEPROM.maxcount()) {
			// tmp, WIP: format eeprom if full
			EEPROM.format();
		}
		return true;
	}
	return false;
}

void hwReadConfigBlock(void* buf, void* addr, size_t length)
{
	uint8_t* dst = static_cast<uint8_t*>(buf);
	int pos = reinterpret_cast<int>(addr);
	while (length-- > 0) {
		*dst++ = EEPROM.read(pos++);
	}
}

void hwWriteConfigBlock(void* buf, void* addr, size_t length)
{
	uint8_t* src = static_cast<uint8_t*>(buf);
	int pos = reinterpret_cast<int>(addr);
	while (length-- > 0) {
		EEPROM.write(pos++, *src++);
	}
}

uint8_t hwReadConfig(const int addr)
{
	uint8_t value;
	hwReadConfigBlock(&value, reinterpret_cast<void*>(addr), 1);
	return value;
}

void hwWriteConfig(const int addr, uint8_t value)
{
	hwWriteConfigBlock(&value, reinterpret_cast<void*>(addr), 1);
}

int8_t hwSleep(unsigned long ms)
{
	// TODO: Not supported!
	(void)ms;
	return MY_SLEEP_NOT_POSSIBLE;
}

int8_t hwSleep(uint8_t interrupt, uint8_t mode, unsigned long ms)
{
	// TODO: Not supported!
	(void)interrupt;
	(void)mode;
	(void)ms;
	return MY_SLEEP_NOT_POSSIBLE;
}

int8_t hwSleep(uint8_t interrupt1, uint8_t mode1, uint8_t interrupt2, uint8_t mode2,
               unsigned long ms)
{
	// TODO: Not supported!
	(void)interrupt1;
	(void)mode1;
	(void)interrupt2;
	(void)mode2;
	(void)ms;
	return MY_SLEEP_NOT_POSSIBLE;
}

bool hwUniqueID(unique_id_t* uniqueID)
{
	(void)memcpy((uint8_t*)uniqueID, (uint32_t*)0x1FFFF7E0, 16);
	return true;
}

uint16_t hwCPUVoltage()
{
	adc_reg_map *regs = ADC1->regs;
	regs->CR2 |= ADC_CR2_TSVREFE;    // enable VREFINT and temp sensor
	regs->SMPR1 =  ADC_SMPR1_SMP17;  // sample rate for VREFINT ADC channel
	return 1200 * 4096 / adc_read(ADC1, 17);
}

uint16_t hwCPUFrequency()
{
	return F_CPU/100000UL;
}

uint16_t hwFreeMem()
{
	//Not yet implemented
	return 0;
}

void hwDebugPrint(const char *fmt, ...)
{
	char fmtBuffer[MY_SERIAL_OUTPUT_SIZE];
#ifdef MY_GATEWAY_FEATURE
	// prepend debug message to be handled correctly by controller (C_INTERNAL, I_LOG_MESSAGE)
	snprintf_P(fmtBuffer, sizeof(fmtBuffer), PSTR("0;255;%d;0;%d;%lu "), C_INTERNAL, I_LOG_MESSAGE,
	           hwMillis());
	MY_SERIALDEVICE.print(fmtBuffer);
#else
	// prepend timestamp
	MY_SERIALDEVICE.print(hwMillis());
	MY_SERIALDEVICE.print(" ");
#endif
	va_list args;
	va_start(args, fmt);
#ifdef MY_GATEWAY_FEATURE
	// Truncate message if this is gateway node
	vsnprintf_P(fmtBuffer, sizeof(fmtBuffer), fmt, args);
	fmtBuffer[sizeof(fmtBuffer) - 2] = '\n';
	fmtBuffer[sizeof(fmtBuffer) - 1] = '\0';
#else
	vsnprintf_P(fmtBuffer, sizeof(fmtBuffer), fmt, args);
#endif
	va_end(args);
	MY_SERIALDEVICE.print(fmtBuffer);
	// Disable flush since current STM32duino implementation performs a reset
	// instead of an actual flush
	//MY_SERIALDEVICE.flush();
}
