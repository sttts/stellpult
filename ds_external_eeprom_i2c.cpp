#include "ds_external_eeprom_i2c.h"

#include <Wire.h>

#define BUFFER_LENGTH 16

ds_external_eeprom_i2c::ds_external_eeprom_i2c(EEPROM_MODEL model, byte eepromAddr){
	if (model==EEPROM_MODEL::MODEL_24_02) init(2,16,eepromAddr);
	if (model==EEPROM_MODEL::MODEL_24_04) init(4,16,eepromAddr);
	if (model==EEPROM_MODEL::MODEL_24_08) init(8,16,eepromAddr);
	if (model==EEPROM_MODEL::MODEL_24_16) init(16,16,eepromAddr);
	if (model==EEPROM_MODEL::MODEL_24_32) init(32,32,eepromAddr);
	if (model==EEPROM_MODEL::MODEL_24_64) init(64,32,eepromAddr);
	if (model==EEPROM_MODEL::MODEL_24_128) init(128,64,eepromAddr);
	if (model==EEPROM_MODEL::MODEL_24_256) init(256,64,eepromAddr);
	if (model==EEPROM_MODEL::MODEL_24_512) init(512,128,eepromAddr);
}

ds_external_eeprom_i2c::ds_external_eeprom_i2c(unsigned int eeprom_size_kbit, unsigned int pageSize, byte eepromAddr){
	init(eeprom_size_kbit,pageSize,eepromAddr);
}

void ds_external_eeprom_i2c::init(unsigned int eeprom_size_kbit, unsigned int pageSize, byte eepromAddr){
	// Wire.begin();

	_eeprom_size_kbit = eeprom_size_kbit;
	_pageSize = pageSize;
	_eepromAddr = eepromAddr;

	_nAddrBytes = eeprom_size_kbit>2 ? 2:1;
	if (_pageSize>BUFFER_LENGTH) _pageSize=BUFFER_LENGTH;
}

bool ds_external_eeprom_i2c::check(){
	Wire.beginTransmission(_eepromAddr);
	sendAddress(0);
	return (Wire.endTransmission()==0);
}

byte ds_external_eeprom_i2c::readByte(int address) {
	byte rdata = 0xFF;
	uint8_t countByte = 1;

	Wire.beginTransmission(_eepromAddr);
	sendAddress(address);
	Wire.endTransmission();
	Wire.requestFrom(_eepromAddr, countByte);
	if (Wire.available()) rdata = Wire.read();
	return rdata;
}

void ds_external_eeprom_i2c::writeByte(int address, byte data){
	Wire.beginTransmission(_eepromAddr);
	sendAddress(address);
	Wire.write(data);
	Wire.endTransmission();
	delay(10);
}

void ds_external_eeprom_i2c::readBytes(int address, int length, byte* p_buffer){
	int blocks = length/BUFFER_LENGTH;

	for (int i=0;i<blocks;i++){
		readBuffer(address+(i*BUFFER_LENGTH),BUFFER_LENGTH,i*BUFFER_LENGTH,p_buffer);
	}

	int pos = blocks*BUFFER_LENGTH;
	byte remainder_len_buffer=length-blocks*BUFFER_LENGTH;
	if (remainder_len_buffer!=0){
		readBuffer(address+pos,remainder_len_buffer,pos,p_buffer);
	}
}

void ds_external_eeprom_i2c::writeBytes(int address, int length, byte* p_data){
	int nPage = floor(address/_pageSize);		// ��������� ����� ��������

	int remainder_write = length;
	int page_size = _pageSize;
	int pos_buffer = 0;
	uint8_t countWrite = 0;

	countWrite=(_pageSize-(address-nPage*_pageSize));

	while(remainder_write>0){	
		writeBuffer(address+pos_buffer,countWrite,pos_buffer,p_data);		
		
		pos_buffer += countWrite;
		remainder_write -= countWrite;

		if (remainder_write>page_size){
			countWrite = page_size;
		}else{
			countWrite = remainder_write;
		}
	}
	
}



void ds_external_eeprom_i2c::readBuffer(int address, uint8_t length, int start_pos,byte* p_buffer){
	Wire.beginTransmission(_eepromAddr);
	sendAddress(address);
	Wire.endTransmission();
	Wire.requestFrom(_eepromAddr, length);
	while (Wire.available()) p_buffer[start_pos++] = Wire.read();
}

void ds_external_eeprom_i2c::writeBuffer(int address, uint8_t length, int start_pos, byte* p_data){
	Wire.beginTransmission(_eepromAddr);
	sendAddress(address);
	for (int i=0;i<length;i++) Wire.write(p_data[start_pos+i]);
	Wire.endTransmission();
	delay(10);
}

void ds_external_eeprom_i2c::sendAddress(int address){
	if (_nAddrBytes==2){
		Wire.write((address>>8));
		Wire.write(address);
	}else{
		Wire.write(address);
	}
}
