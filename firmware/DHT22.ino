/*
 * FILE:        DHT22.ino
 * PURPOSE:     DHT22 sensor usage with DHT library.
 * LICENSE:     GPL v3 (http://www.gnu.org/licenses/gpl.html)
 *
 * Based on Scott Piette example (https://github.com/piettetech/PietteTech_DHT)
 * and Gustavo Gonnet's project from hackster.io site
 * (https://particle.hackster.io/gusgonnet/temperature-humidity-monitor-with-blynk-7faa51?ref=tag&ref_id=blynk&offset=0)
 * Thanks guys, good work!
 *
 * Temperature and humidity monitorin with DHT22 sensor using
 * Particle Photon.
 *
 * Teemu Kulma teemu.kulma@iki.fi
 */

#include "blynk.h"
#include "PietteTech_DHT.h"

// system defines
#define DHTTYPE  DHT22                 // Sensor type DHT11/21/22/AM2301/AM2302
#define DHTPIN   4         	           // Digital pin for communications
#define DHT_SAMPLE_INTERVAL   5000     // Sample every minute

//declaration
void dht_wrapper(); // must be declared before the lib initialization

// Lib instantiate
PietteTech_DHT DHT(DHTPIN, DHTTYPE, dht_wrapper);

// globals
unsigned int DHTnextSampleTime;	       // Next time we want to start sample
bool bDHTstarted;		                   // flag to indicate we started acquisition
int n;                                 // counter

// this is coming from http://www.instructables.com/id/Datalogging-with-Spark-Core-Google-Drive/?ALLSTEPS
char resultstr[64]; //String to store the sensor data

//DANGER - DO NOT SHARE!!!!
char auth[] = "01234567890123456789";  // Put your blynk token here
//DANGER - DO NOT SHARE!!!!

void setup()
{

  Blynk.begin(auth);

  DHTnextSampleTime = 0;  // Start the first sample immediately
  Spark.variable("result", resultstr, STRING);
}


// This wrapper is in charge of calling
// must be defined like this for the lib work
void dht_wrapper() {
    DHT.isrCallback();
}


void loop()
{

  Blynk.run(); // all the Blynk magic happens here

  // Check if we need to start the next sample
  if (millis() > DHTnextSampleTime) {
	   if (!bDHTstarted) {		// start the sample
       DHT.acquire();
       bDHTstarted = true;
     }

     if (!DHT.acquiring()) {		// has sample completed?

       // get DHT status
       int result = DHT.getStatus();

       // Change Serial.print to Spark.Publish !!!!!
       Serial.print("Read sensor: ");
       switch (result) {
         case DHTLIB_OK:
         Serial.println("OK");
         break;
         case DHTLIB_ERROR_CHECKSUM:
         Serial.println("Error\n\r\tChecksum error");
         break;
         case DHTLIB_ERROR_ISR_TIMEOUT:
         Serial.println("Error\n\r\tISR time out error");
         break;
         case DHTLIB_ERROR_RESPONSE_TIMEOUT:
         Serial.println("Error\n\r\tResponse time out error");
         break;
         case DHTLIB_ERROR_DATA_TIMEOUT:
         Serial.println("Error\n\r\tData time out error");
         break;
         case DHTLIB_ERROR_ACQUIRING:
         Serial.println("Error\n\r\tAcquiring");
         break;
         case DHTLIB_ERROR_DELTA:
         Serial.println("Error\n\r\tDelta time to small");
         break;
         case DHTLIB_ERROR_NOTSTARTED:
         Serial.println("Error\n\r\tNot started");
         break;
         default:
         Serial.println("Unknown error");
         break;
       }

       float temp = (float)DHT.getCelsius();
       int temp1 = (temp - (int)temp) * 100;

       char tempInChar[32];
       sprintf(tempInChar,"%0d.%d", (int)temp, temp1);
       Spark.publish("Temperature: ", tempInChar, 60, PRIVATE);

       // virtual pin 1 will be the temperature
       Blynk.virtualWrite(V1, tempInChar);

       float humid = (float)DHT.getHumidity();
       int humid1 = (humid - (int)humid) * 100;

       sprintf(tempInChar,"%0d.%d", (int)humid, humid1);
       Spark.publish("Humidity:", tempInChar, 60, PRIVATE);

       // virtual pin 2 will be the humidity
       Blynk.virtualWrite(V2, tempInChar);

       n++;  // increment counter
       bDHTstarted = false;  // reset the sample flag so we can take another
       DHTnextSampleTime = millis() + DHT_SAMPLE_INTERVAL;  // set the time for next sample
     }
   }
}
