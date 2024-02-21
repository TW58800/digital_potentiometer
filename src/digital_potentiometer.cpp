/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "c:/Users/timwh/Projects/digital_potentiometer/src/digital_potentiometer.ino"
/***********************Notice********************************
   1.Resistor terminals A, B and W have no restrictions on
     polarity with respect to each other.
   2.Current through terminals A, B and W should not excceed ±1mA.
   3.Voltages on terminals A, B and W should be within 0 - VCC.
************************************************************/

//#include <SPI.h>

void setup();
void loop();
void DigitalPotWrite(int cmd, int val);
#line 10 "c:/Users/timwh/Projects/digital_potentiometer/src/digital_potentiometer.ino"
const int CS_PIN = A5;

/***********************MCP42XXX Commands************************/
//potentiometer select byte
const int POT0_SEL = 0x11;
const int POT1_SEL = 0x12;
const int BOTH_POT_SEL = 0x13;

//shutdown the device to put it into power-saving mode.
//In this mode, terminal A is open-circuited and the B and W terminals are shorted together.
//send new command and value to exit shutdowm mode.
const int POT0_SHUTDOWN = 0x21;
const int POT1_SHUTDOWN = 0x22;
const int BOTH_POT_SHUTDOWN = 0x23;

/***********************Customized Varialbes**********************/
//resistance value byte (0 - 255)
//The wiper is reset to the mid-scale position upon power-up, i.e. POT0_Dn = POT1_Dn = 128
int POT0_Dn = 128;
int POT1_Dn = 128;
int BOTH_POT_Dn = 128;

// NTC resistances vs Dn value

// -10°C = 58880    151
//  -5°C = 45950    118
//   0°C = 36130    92
//   5°C = 28600    73
//  10°C = 22800    58
//  15°C = 18300    47
//  20°C = 14770    38
//  25°C = 12000    31


//Function Declaration
//void DigitalPotTransfer(int cmd, int value);     //send the command and the wiper value through SPI
int val = 0;


void setup()
{
  Serial.begin(115200);
  pinMode(CS_PIN, OUTPUT);   // set the CS_PIN as an output:
  SPI.begin();     // initialize SPI:
}

void loop()
{
    POT0_Dn = 151;
    DigitalPotWrite(POT0_SEL, POT0_Dn);
    delay(2000);
    val = analogRead(A0);
	Particle.publish("POT0_res1", String::format("%d", val));
    
    POT0_Dn = 31;
    DigitalPotWrite(POT0_SEL , POT0_Dn);
    delay(2000);
    val = analogRead(A0);
	Particle.publish("POT0_res2", String::format("%d", val));
}

void DigitalPotWrite(int cmd, int val)
{
  // constrain input value within 0 - 255
  val = constrain(val, 0, 255);
  // set the CS pin to low to select the chip:
  digitalWrite(CS_PIN, LOW);
  // send the command and value via SPI:
  SPI.transfer(cmd);
  SPI.transfer(val);
  // Set the CS pin high to execute the command:
  digitalWrite(CS_PIN, HIGH);
}



/*
#include "math.h"

SYSTEM_MODE(AUTOMATIC);
//uint32_t timer = millis();
TCPServer server = TCPServer(23);
TCPClient client;
int LED = D7;
int ntcPin[3] = {A0, A1, A2};
bool TaRising[3] = {false, false, false};
int Vo = 0;
int counter = 0;
float R1 = 10000.0f;
float logR2, R2, T, Tc, Tf;
float Ta[3] = {0.0f, 0.0f, 0.0f};
float TaLast[3] = {0.0f, 0.0f, 0.0f};
float c1 = 0.001125308852122f, c2 = 0.000234711863267f, c3 = 0.000000085663516f;

void setup() {

	// Connects to a network secured with WPA2 credentials.
	//WiFi.setCredentials("Humpty", "ttIImm11&&");
	//WiFi.setConfig(NetworkInterfaceConfig()
  		//.source(NetworkInterfaceConfigSource::STATIC)
  		//.address({192,168,0,7}, {255,255,255,0}));
  		//.gateway({192,168,0,1})
  		//.dns({192,168,0,1}));

	pinMode(LED, OUTPUT);   
	WiFi.connect();
	Serial.begin(9600);
	waitFor(Serial.isConnected, 15000);
	Serial.println("connected");
	Serial.printlnf("localIP=%s", WiFi.localIP().toString().c_str());
	Serial.printlnf("subnetMask=%s", WiFi.subnetMask().toString().c_str());
	Serial.printlnf("gatewayIP=%s", WiFi.gatewayIP().toString().c_str());
	server.begin();
}


void loop() {

	for (int i=0; i<3; i++) {
		Vo = analogRead(ntcPin[i]);  
  		R2 = R1 * (4095.0f / (float)Vo - 1.0f);
  		logR2 = log(R2);
		T = (1.0f / (c1 + (c2*logR2) + (c3*logR2*logR2*logR2)));
		Tc = T - 273.15f;
		Ta[i] = Ta[i] + Tc;
	}
	if (counter >= 100) {
		for (int i=0; i<3; i++) {
			Ta[i] = Ta[i] / (float)counter;
			Serial.printlnf("\nTemperature %02i: %3.1f°C", i, Ta[i]);
			float TaDelta = Ta[i] - TaLast[i];
			if (TaDelta > 0.0f) {
				if ((TaRising[i] == false) & (TaDelta < 0.2f)) {
					Ta[i] = TaLast[i];
				}
				else TaRising[i] = true;
			}
			else {
				if ((TaRising[i] == true) & (TaDelta > -0.2f)) {
					Ta[i] = TaLast[i];
				}
				else TaRising[i] = false;
			}
			TaLast[i] = Ta[i]; 
		}
  		digitalWrite(LED, HIGH); 
  		delay(200);              
  		digitalWrite(LED, LOW);  
  		if (client.status()) { 
    		Serial.println("\nTCP connected");
			while (client.available()) {
    			byte payload = client.read(); 
    			Serial.printlnf("TCP byte received: %i\n", payload);
			}
			byte* ta = reinterpret_cast<byte*>(&Ta);
			int bytes_sent = server.write(ta, 12, 5000);
			int err = server.getWriteError();
			if (err != 0) {
  				Serial.printlnf("TCPServer::write() failed (error = %d), number of bytes written: %d\n", err, bytes_sent);
			}
    	  	delay(8800);              
			}
		else {
			// if no client is yet connected, check for a new connection
			client = server.available();
		}
		for (int i=0; i<3; i++) Ta[i] = 0;
    	counter = 0;
	}   
	counter++;
	delay(10);
	Serial.print(".");
}

*/