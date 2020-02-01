// 
// 
// 

#include "configuration.h"

bool configuration::hasConfig() 
{
	bool hasConfig = false;
	EEPROM.begin(EEPROM_SIZE);
	hasConfig = EEPROM.read(EEPROM_CONFIG_ADDRESS) >= 71;
	EEPROM.end();
	return hasConfig;
}

bool configuration::save()
{
	int address = EEPROM_CONFIG_ADDRESS;

	EEPROM.begin(EEPROM_SIZE);
	EEPROM.put(address, EEPROM_CHECK_SUM);
	address++;

	address += saveString(address, ssid);
	address += saveString(address, ssidPassword);
	address += saveByte(address, meterType);
	address += saveString(address, mqttHost);
	address += saveInt(address, mqttPort);
	address += saveString(address, mqttClientID);
	address += saveString(address, mqttPublishTopic);
	address += saveString(address, mqttSubscribeTopic);

	if (isSecure()) {
		address += saveBool(address, true);
		address += saveString(address, mqttUser);
		address += saveString(address, mqttPass);
	}
	else
		address += saveBool(address, false);


	address += saveByte(address, authSecurity);
	if (authSecurity > 0) {
		address += saveString(address, authUser);
		address += saveString(address, authPass);
	}

	address += saveInt(address, fuseSize);
	address += saveInt(address, distSys);

	bool success = EEPROM.commit();
	EEPROM.end();

	return success;
}


bool configuration::load()
{
	int address = EEPROM_CONFIG_ADDRESS;
	bool success = false;

	ssid = (char*)String("").c_str();
	ssidPassword = (char*)String("").c_str();
	meterType = (byte)0;
	mqttHost = (char*)String("").c_str();
	mqttClientID = (char*)String("").c_str();
	mqttPublishTopic = (char*)String("").c_str();
	mqttSubscribeTopic = (char*)String("").c_str();
	mqttUser = 0;
	mqttPass = 0;
	mqttPort = 1883;
	authSecurity = 0;
	authUser = 0;
	authPass = 0;
	fuseSize = 0;
	distSys = 0;

	EEPROM.begin(EEPROM_SIZE);
	int cs = EEPROM.read(address);
	if (cs >= 71)
	{
		address++;

		address += readString(address, &ssid);
		address += readString(address, &ssidPassword);
		address += readByte(address, &meterType);
		address += readString(address, &mqttHost);
		address += readInt(address, &mqttPort);
		address += readString(address, &mqttClientID);
		address += readString(address, &mqttPublishTopic);
		address += readString(address, &mqttSubscribeTopic);

		bool secure = false;
		address += readBool(address, &secure);

		if (secure)
		{
			address += readString(address, &mqttUser);
			address += readString(address, &mqttPass);
		}
		else
		{
			mqttUser = 0;
			mqttPass = 0;
		}

		success = true;
	}
	if(cs >= 72) {
		address += readByte(address, &authSecurity);
		if (authSecurity > 0) {
			address += readString(address, &authUser);
			address += readString(address, &authPass);
		} else {
			authUser = 0;
			authPass = 0;
		}
	}
	if(cs >= 73) {
		address += readInt(address, &fuseSize);
	}
	if(cs >= 74) {
		address += readByte(address, &distSys);
	}
	EEPROM.end();
	return success;
}

bool configuration::isSecure()
{
	return (mqttUser != 0) && (String(mqttUser).length() > 0);
}

int configuration::readInt(int address, int *value)
{
	int lower = EEPROM.read(address);
	int higher = EEPROM.read(address + 1);
	*value = lower + (higher << 8);
	return 2;
}
int configuration::saveInt(int address, int value)
{
	byte lowByte = value & 0xFF;
	byte highByte = ((value >> 8) & 0xFF);

	EEPROM.write(address, lowByte);
	EEPROM.write(address + 1, highByte);

	return 2;
}

int configuration::readBool(int address, bool *value)
{
	byte y = EEPROM.read(address);
	*value = (bool)y;
	return 1;
}

int configuration::saveBool(int address, bool value)
{
	byte y = (byte)value;
	EEPROM.write(address, y);
	return 1;
}

int configuration::readByte(int address, byte *value)
{
	*value = EEPROM.read(address);
	return 1;
}

int configuration::saveByte(int address, byte value)
{
	EEPROM.write(address, value);
	return 1;
}
void configuration::print(Stream* debugger)
{
	debugger->println("Configuration:");
	debugger->println("-----------------------------------------------");
	debugger->printf("ssid:                 %s\r\n", this->ssid);
	debugger->printf("ssidPassword:         %s\r\n", this->ssidPassword);
	debugger->printf("meterType:            %i\r\n", this->meterType);
	if(this->mqttHost) {
		debugger->printf("mqttHost:             %s\r\n", this->mqttHost);
		debugger->printf("mqttPort:             %i\r\n", this->mqttPort);
		debugger->printf("mqttClientID:         %s\r\n", this->mqttClientID);
		debugger->printf("mqttPublishTopic:     %s\r\n", this->mqttPublishTopic);
		debugger->printf("mqttSubscribeTopic:   %s\r\n", this->mqttSubscribeTopic);
	}

	if (this->isSecure())
	{
		debugger->printf("SECURE MQTT CONNECTION:\r\n");
		debugger->printf("mqttUser:             %s\r\n", this->mqttUser);
		debugger->printf("mqttPass:             %s\r\n", this->mqttPass);
	}

	if (this->authSecurity > 0) {
		debugger->printf("WEB AUTH:\r\n");
		debugger->printf("authSecurity:         %i\r\n", this->authSecurity);
		debugger->printf("authUser:             %s\r\n", this->authUser);
		debugger->printf("authPass:             %s\r\n", this->authPass);
	}
	debugger->printf("fuseSize:             %i\r\n", this->fuseSize);
	debugger->printf("distSys:              %i\r\n", this->distSys);

	debugger->println("-----------------------------------------------");
}

template <class T> int configuration::writeAnything(int ee, const T& value)
{
	const byte* p = (const byte*)(const void*)&value;
	unsigned int i;
	for (i = 0; i < sizeof(value); i++)
		EEPROM.write(ee++, *p++);
	return i;
}

template <class T> int configuration::readAnything(int ee, T& value)
{
	byte* p = (byte*)(void*)&value;
	unsigned int i;
	for (i = 0; i < sizeof(value); i++)
		*p++ = EEPROM.read(ee++);
	return i;
}

int configuration::readString(int pAddress, char* pString[])
{
	int address = 0;
	byte length = EEPROM.read(pAddress + address);
	address++;

	char* buffer = new char[length];
	for (int i = 0; i<length; i++)
	{
		buffer[i] = EEPROM.read(pAddress + address++);
	}
	*pString = buffer;
	return address;
}
int configuration::saveString(int pAddress, char* pString)
{
	int address = 0;
	int length = pString ? strlen(pString) + 1 : 0;
	EEPROM.put(pAddress + address, length);
	address++;

	for (int i = 0; i < length; i++)
	{
		EEPROM.put(pAddress + address, pString[i]);
		address++;
	}

	return address;
}