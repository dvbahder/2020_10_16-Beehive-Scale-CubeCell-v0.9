// BeeScale with Temp over LoRaWAN - 2020-10-27 - DvBahder
// combined with Code from HX711-multi-LORA Test code - (c) Marc Wetzel 2019, 
// and willhelmx/LMSYSTEMS
// and more...
#include <Arduino.h>
#include <LoRaWan_APP.h>
#include <HX711-multi.h>
#include <math.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <../include/config_account_79.h> //Konfig LORAWAN-TTN
//*** Dont forget to edit the commissioning.h file for EUI codes *** 
#define BOOT_MESSAGE "2020_10_16_Test_CubeCell002 - ID 947279"
// Pins to the DS18B20
#define ONE_WIRE_BUS GPIO1 // for Data DS18B20 
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

//Measured values
#define debug_all 1
float temp1 = 0;
unsigned char *tempout;
int16 temp1_send;
uint16_t voltage;
uint16_t voltage_b;
int32_t weigth; //value for send
long w1; //measured weigth value var_type
float w3; // help for multiplikation weigth 
int32_t w2; //help for between weigth values
//float calibre_fact = 25.000; //separate fix value for only one scale, measured befor start 
float calibre_fact = 21.17804731;
int32_t offset = -8900; //separate fix value for only one scale, measured befor start 

// Pins to the load cell amp
#define CLK GPIO5      // clock pin to the load cell amp
#define DOUT1 GPIO0    // data pin to the first lca
#define DOUT2 GPIO2    // data pin to the second lca
#define DOUT3 GPIO3    // data pin to the third lca
#define TARE_TIMEOUT_SECONDS 4
byte DOUTS[1] = {DOUT1}; // enable for single HX711
//byte DOUTS[2] = {DOUT1, DOUT2}; // enable for dual HX711
//byte DOUTS[3] = {DOUT1, DOUT2, DOUT3}; // triple …
#define CHANNEL_COUNT sizeof(DOUTS)/sizeof(byte)
long int results[CHANNEL_COUNT];
HX711MULTI scales(CHANNEL_COUNT, DOUTS, CLK);
struct {
  unsigned long magic;
  int  count;
  float weight[3]; 
  float factor[3]; 
} settings;

void measure() {
  digitalWrite(Vext, LOW); // enable Sensor Power
  delay(1000);
  // Read Weigth from HX711
		scales.read(results);
		w1 = results[0]; //weigth = results[0]; //w2,w1 Hilfsvariable
		w2 =  (int32_t)w1;
		w3 = (w2-offset)/calibre_fact;
		weigth =(int32_t)w3;		
		Serial.print("read w1: ");
		Serial.println(w1);
		Serial.print("read w2: ");
		Serial.println(w2);
		Serial.print("read w3: ");
		Serial.println(w3);
		Serial.print("offset: ");
		Serial.println(offset);
		Serial.print("calibre_fact: ");
		Serial.println(calibre_fact);
  	delay(10);
  // Read temperatures from DS18xxx
		sensors.requestTemperatures(); // Send the command to get temperatures
		delay(750);
		temp1 = ((sensors.getTempCByIndex(0))+200)*100;
		//Serial.print("read temp1: ");
		//Serial.print(temp1);
		temp1_send = (int16)temp1;

	delay(1000); // Wait for a while before proceeding
  //ADC == ADC1
  		voltage=analogRead(ADC);//return the voltage in mV, max value can be read is 2400mV 
		voltage_b=getBatteryVoltage();
  	delay(10);
  digitalWrite(Vext, HIGH); // disable Sensor Power
}

/* Prepares the payload of the frame */
static void prepareTxFrame( uint8_t port )
{
	/*appData size is LORAWAN_APP_DATA_MAX_SIZE which is defined in "commissioning.h".
	*appDataSize max value is LORAWAN_APP_DATA_MAX_SIZE (default 128).
	*/
    // Messung aktualisieren
    measure();

    if (debug_all)
    {
        Serial.println(" ");
		Serial.println("Messwerte ");
        Serial.print("millis: ");
        Serial.print(millis());
        Serial.print("   voltage: ");
        Serial.print(voltage);
		Serial.print("   voltage_b: ");
        Serial.print(voltage_b);
        Serial.print("   weight: ");
        Serial.print(weigth);  
        Serial.print("   temp1: ");
        Serial.println(temp1_send-2000);
        delay(10);
    }

	appDataSize = 10;
    appData[0] = voltage;
    appData[1] = voltage >> 8;
    appData[2] = weigth;
    appData[3] = weigth >> 8;
    appData[4] = weigth >> 16;
	appData[5] = temp1_send;
	appData[6] = temp1_send >> 8;
	appData[7] = temp1_send >> 16;
	appData[8] = voltage_b;
    appData[9] = voltage_b >> 8;
 }

void setup() {
	pinMode(Vext, OUTPUT);
 	pinMode(CLK, OUTPUT);
	digitalWrite(Vext, LOW); // Sensor Power ON
	delay(100);
	boardInitMcu();
	Serial.begin(115200);
   	Serial.println(BOOT_MESSAGE);
  	delay(100);
  	Serial.flush();
	sensors.begin();
  	delay(1000);
	//scales.setDebugEnable();

#if(AT_SUPPORT)
	enableAt();
#endif
	deviceState = DEVICE_STATE_INIT;
	LoRaWAN.ifskipjoin();
}

void loop()
{
	switch( deviceState )
	{
		case DEVICE_STATE_INIT:
		{
#if(LORAWAN_DEVEUI_AUTO)
			LoRaWAN.generateDeveuiByChipID();
#endif
#if(AT_SUPPORT)
			getDevParam();
#endif
			printDevParam();
			LoRaWAN.init(loraWanClass,loraWanRegion);
			deviceState = DEVICE_STATE_JOIN;
			break;
		}
		case DEVICE_STATE_JOIN:
		{
			LoRaWAN.join();
			break;
		}
		case DEVICE_STATE_SEND:
		{
			prepareTxFrame( appPort );
			LoRaWAN.send();
			deviceState = DEVICE_STATE_CYCLE;
			break;
		}
		case DEVICE_STATE_CYCLE:
		{
			// Schedule next packet transmission
			txDutyCycleTime = appTxDutyCycle + randr( 0, APP_TX_DUTYCYCLE_RND );
			LoRaWAN.cycle(txDutyCycleTime);
			deviceState = DEVICE_STATE_SLEEP;
			break;
		}
		case DEVICE_STATE_SLEEP:
		{
			LoRaWAN.sleep();
			break;
		}
		default:
		{
			deviceState = DEVICE_STATE_INIT;
			break;
		}
	}
}