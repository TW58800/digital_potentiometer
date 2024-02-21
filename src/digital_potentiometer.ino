/***********************Notice********************************
   1.Resistor terminals A, B and W have no restrictions on
     polarity with respect to each other.
   2.Current through terminals A, B and W should not excceed ±1mA.
   3.Voltages on terminals A, B and W should be within 0 - VCC.
************************************************************/

// NTC resistances vs Dn vs RWB(Dn) resistance vs NTC curve temperatures at RWB res vs boiler flow temp (Tmax CH = 70°C, Tmin CH = 35°C, Tmin out = -10°C, Tmax out = 25°C)

// -10°C = 58880Ω    151	59109Ω	   -10.08°C		70°C
//  -5°C = 45950Ω    118	46219Ω		-5.12°C		65°C
//   0°C = 36130Ω    92		36063Ω		 0.03°C		60°C
//   5°C = 28600Ω    73		28640Ω		 4.97°C		55°C
//  10°C = 22800Ω    58		22781Ω		10.02°C		50°C	
//  15°C = 18300Ω    47		18484Ω		14.76°C		45°C
//  20°C = 14770Ω    38		14969Ω		19.69°C		40°C
//  25°C = 12000Ω    31		12234Ω		24.53°C		35°C


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
int POT0_Dn = 31;  // boiler thinks it's 25°C outside so lowers the flow temp to 35°C
int POT1_Dn = 128;
int BOTH_POT_Dn = 128;


//Function Declaration
//void DigitalPotTransfer(int cmd, int value);     //send the command and the wiper value through SPI
uint32_t val = 0;
int LED = D7;
byte payload = 0xff;
//uint32_t timer = millis();
TCPServer server = TCPServer(23);
TCPClient client;


void setup()
{
	// Connects to a network secured with WPA2 credentials.
	//WiFi.clearCredentials();
	//WiFi.setCredentials("Humpty", "ttIImm11&&");
	//WiFi.setConfig(NetworkInterfaceConfig()
  		//.source(NetworkInterfaceConfigSource::STATIC)
  		//.address({192,168,0,7}, {255,255,255,0}));
  		//.gateway({192,168,0,1})
  		//.dns({192,168,0,1}));

	Serial.begin(9600);
	pinMode(CS_PIN, OUTPUT);   // set the CS_PIN as an output:
	pinMode(LED, OUTPUT);   
  	SPI.begin();     // initialize SPI:
	WiFi.connect();
	waitFor(Serial.isConnected, 5000);
	Serial.println("connected");
	Serial.printlnf("localIP=%s", WiFi.localIP().toString().c_str());
	Serial.printlnf("subnetMask=%s", WiFi.subnetMask().toString().c_str());
	Serial.printlnf("gatewayIP=%s", WiFi.gatewayIP().toString().c_str());
	Particle.publish("IP", ("localIP=%s", WiFi.localIP().toString().c_str()));
	server.begin();
}

void loop()
{ 
  	digitalWrite(LED, HIGH); 
  	delay(200);              
  	digitalWrite(LED, LOW);

    DigitalPotWrite(POT0_SEL , POT0_Dn);
    delay(1000);

    val = analogRead(A0);
	val = (100000-(100000*val)/4095);

  	if (client.status()) { 

    	Serial.println("\nTCP connected");
		byte* Val = reinterpret_cast<byte*>(&val);
		int bytes_sent = server.write(Val, 4, 5000);
		int err = server.getWriteError();
		if (err != 0) {
  			Serial.printlnf("TCPServer::write() failed (error = %d), number of bytes written: %d\n", err, bytes_sent);
		}

		while (client.available()) {
    		payload = client.read();
			setDn(payload);
			Particle.publish("request", String::format("temp: %d°C, Dn: %d", payload, POT0_Dn));
			Serial.printlnf("TCP byte received: %i\n", payload);
		}
	}
	else {
		client = server.available();  // if no client is yet connected, check for a new connection
	}
  	delay(8800);              
}


void setDn(int temp)
{
	switch(temp) {
  		case 35:
    		POT0_Dn = 31;
    		break;
  		case 40:
    		POT0_Dn = 38;
    		break;
  		case 45:
    		POT0_Dn = 47;
    		break;
  		case 50:
    		POT0_Dn = 58;
    		break;
  		case 55:
    		POT0_Dn = 73;
    		break;
  		case 60:
    		POT0_Dn = 92;
    		break;
  		case 65:
    		POT0_Dn = 118;
    		break;
  		case 70:
    		POT0_Dn = 151;
    		break;
  		default:
    		POT0_Dn = 92;  // 60°C
	}
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