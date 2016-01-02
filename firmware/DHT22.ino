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
#include "HttpClient.h"

// system defines
#define DHTTYPE               DHT22    // Sensor type DHT11/21/22/AM2301/AM2302
#define DHTPIN                4        // Digital pin for communications
<<<<<<< HEAD
#define DHT_SAMPLE_INTERVAL   60000    // Sample every minute
=======
#define DHT_SAMPLE_INTERVAL   300000   // Sample interval (60000 = 1 minute)
>>>>>>> origin/master

//DANGER - DO NOT SHARE!!!!
#define UBIDOTS_TEMP_VARIABLE_ID "UBIDOTSVARIABLEID1"
#define UBIDOTS_HUMID_VARIABLE_ID "UBIDOTSVARIABLEID2"
#define UBIDOTS_TOKEN "UBIDOTSTOKEN"
//DANGER - DO NOT SHARE!!!!

// Declaration
void dht_wrapper(); // must be declared before the lib initialization

// Headers currently need to be set at init, useful for API keys etc.
http_header_t headers[] = {
    { "Content-Type", "application/json" },
    { NULL, NULL } // NOTE: Always terminate headers will NULL
};

http_request_t request;
http_response_t response;

// Lib instantiate
PietteTech_DHT DHT(DHTPIN, DHTTYPE, dht_wrapper);

// globals
unsigned int DHTnextSampleTime;	       // Next time we want to start sample
bool bDHTstarted;		                   // flag to indicate we started acquisition
int n;                                 // counter
double TempC;                          // Temperature from the sensor
double Humid;                          // Humidity from the sensor
int readStatus;

HttpClient http;

// this is coming from http://www.instructables.com/id/Datalogging-with-Spark-Core-Google-Drive/?ALLSTEPS
char resultstr[64]; //String to store the sensor data

//DANGER - DO NOT SHARE!!!!
char auth[] = "WILLBECHANGEDTOBLYNKTOKEN";  // Put your blynk token here
//DANGER - DO NOT SHARE!!!!

void setup()
{
  request.hostname = "things.ubidots.com";
  request.port = 80;

  Blynk.begin(auth);

  DHTnextSampleTime = 0;  // Start the first sample immediately
  Spark.variable("result", resultstr, STRING);

  Spark.variable("readStatus", readStatus);
  Spark.variable("Temperature", TempC);
  Spark.variable("Humidity", Humid);
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

       switch (result) {
         case DHTLIB_OK:
         Spark.publish("Status", "Read sensor: OK", 60, PRIVATE);
         readStatus = 0;
         break;
         case DHTLIB_ERROR_CHECKSUM:
         Spark.publish("Status", "Read sensor: Error - Checksum error", 60, PRIVATE);
         readStatus = 1;
         break;
         case DHTLIB_ERROR_ISR_TIMEOUT:
         Spark.publish("Status", "Read sensor: Error - ISR time out error", 60, PRIVATE);
         readStatus = 2;
         break;
         case DHTLIB_ERROR_RESPONSE_TIMEOUT:
         Spark.publish("Status", "Read sensor: Error - Response time out error", 60, PRIVATE);
         readStatus = 3;
         break;
         case DHTLIB_ERROR_DATA_TIMEOUT:
         Spark.publish("Status", "Read sensor: Error - Data time out error", 60, PRIVATE);
         readStatus = 4;
         break;
         case DHTLIB_ERROR_ACQUIRING:
         Spark.publish("Status", "Read sensor: Error - Acquiring", 60, PRIVATE);
         readStatus = 5;
         break;
         case DHTLIB_ERROR_DELTA:
         Spark.publish("Status", "Read sensor: Error - Delta time to small", 60, PRIVATE);
         readStatus = 6;
         break;
         case DHTLIB_ERROR_NOTSTARTED:
         Spark.publish("Status", "Read sensor: Error - Not started", 60, PRIVATE);
         readStatus = 7;
         break;
         default:
         Spark.publish("Status", "Read sensor: Unknown error", 60, PRIVATE);
         readStatus = 8;
         break;
       }

       float temp = (float)DHT.getCelsius();
       TempC = round(DHT.getCelsius()*10)/10;
       int temp1 = (temp - (int)temp) * 100;

       // Ubidots temp variable
       request.path = "/api/v1.6/variables/"UBIDOTS_TEMP_VARIABLE_ID"/values?token="UBIDOTS_TOKEN;
       request.body = "{\"value\":" + String(TempC) + "}";
       http.post(request, response, headers);

       char tempInChar[32];
       sprintf(tempInChar,"%0d.%d", (int)temp, temp1);
       Spark.publish("Temperature", tempInChar, 60, PRIVATE);

       // Virtual pin 1 will be the temperature
       Blynk.virtualWrite(V1, tempInChar);

       float humid = (float)DHT.getHumidity();
       Humid = round(DHT.getHumidity()*10)/10;
       int humid1 = (humid - (int)humid) * 100;

       // Ubidots
       request.path = "/api/v1.6/variables/"UBIDOTS_HUMID_VARIABLE_ID"/values?token="UBIDOTS_TOKEN;
       request.body = "{\"value\":" + String(Humid) + "}";
       http.post(request, response, headers);

       sprintf(tempInChar,"%0d.%d", (int)humid, humid1);
       Spark.publish("Humidity", tempInChar, 60, PRIVATE);

       // Virtual pin 2 will be the humidity
       Blynk.virtualWrite(V2, tempInChar);

       n++;  // increment counter
       bDHTstarted = false;  // reset the sample flag so we can take another
       DHTnextSampleTime = millis() + DHT_SAMPLE_INTERVAL;  // set the time for next sample
     }
   }
}
