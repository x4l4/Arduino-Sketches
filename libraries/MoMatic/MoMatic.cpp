#include <arduino.h>
#include "MoMatic.h"
#include <SoftwareSerial.h>
#include <SimpleModbusSlave.h>

SoftwareSerial mySerial(RX_PIN, TX_PIN); 

void momatic_setup(unsigned int *registri, unsigned int size) {
	Serial.begin(9600);
 	pinMode(ID_PIN_1, INPUT);
 	pinMode(ID_PIN_2, INPUT);
 	pinMode(ID_PIN_3, INPUT);
 	pinMode(ID_PIN_4, INPUT);
 	pinMode(LED, OUTPUT);


 	int id = momatic_read_id();
 	if(id == 0 || id > 31) {
 		momatic_alarm();
 	}

 	modbus_configure(&mySerial, BAUD_RATE, SERIAL_8N2, id, TASMISSION_PIN, size, registri);
	Serial.println("MoSat Pronta!");
}

int momatic_read_id() {
	int id = 0;
	
	Serial.print("MoSat ID (");
	id+=(int)digitalRead(ID_PIN_4);
	Serial.print((int)digitalRead(ID_PIN_4));
	id = id << 1;
	id+=(int)digitalRead(ID_PIN_3);
	Serial.print((int)digitalRead(ID_PIN_3));
	id = id << 1;
	id+=(int)digitalRead(ID_PIN_2);
	Serial.print((int)digitalRead(ID_PIN_2));
	id = id << 1;
	id+=(int)digitalRead(ID_PIN_1);
	Serial.print((int)digitalRead(ID_PIN_1));

	Serial.print("):");
	Serial.print(id, DEC);
	Serial.println("");
	return id;
}

void momatic_alarm() {
	while(1) {
		digitalWrite(LED, HIGH);
		delay(500);
		digitalWrite(LED, LOW);
		delay(500);
		Serial.println("MoSat in allarme!!");
	}
}