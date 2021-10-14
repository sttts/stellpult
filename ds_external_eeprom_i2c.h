#ifndef DS_EXTERNAL_EEPROM_I2C_H
#define DS_EXTERNAL_EEPROM_I2C_H
/*
	���������� ��� ������ � ������� EEPROM �� I2C ��������� (24XX02 - 24XX512)
	������ � ������������ ����� ��� AT24C04, AT24C16, AT24C32 � �.�.

	email: dik@Inbox.ru
*/

#include <Arduino.h>

class ds_external_eeprom_i2c {
	public:
		enum EEPROM_MODEL {
			MODEL_24_02, MODEL_24_04, MODEL_24_08, MODEL_24_16, MODEL_24_32, MODEL_24_64, MODEL_24_128, MODEL_24_256, MODEL_24_512
		};

		ds_external_eeprom_i2c(EEPROM_MODEL model, byte eepromAddr);
		ds_external_eeprom_i2c(unsigned int eeprom_size_kbit, unsigned int pageSize, byte eepromAddr);

		// check device. true - valid
		bool check();

		// read/write byte
		byte readByte(int address);
		void writeByte(int address, byte data);

		// read/write bytes
    void readBytes(int address, int length, byte* p_buffer);
		void writeBytes(int address, int length, byte* p_data);

    inline void read(int address, uint8_t* p_buffer, int length) {
      readBytes(address, length, p_buffer);
    }
    inline void write(int address, uint8_t* p_data, int length) {
      writeBytes(address, length, p_data);
    }

	private:
		void init(unsigned int eeprom_size_kbit, unsigned int pageSize, byte eepromAddr);

		void sendAddress(int address);
		void readBuffer(int address, uint8_t length, int start_pos, byte* p_buffer);
		void writeBuffer(int address, uint8_t length, int start_pos, byte* p_data);


		unsigned int _eeprom_size_kbit;
		unsigned int _pageSize;
		byte _eepromAddr;
		unsigned int _nAddrBytes;

};


#endif
