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

#include "PietteTech_DHT.h"

// system defines
#define DHTTYPE               DHT22    // Sensor type DHT11/21/22/AM2301/AM2302
#define DHTPIN                4        // Digital pin for communications
#define DHT_SAMPLE_INTERVAL   60000    // Sample every minute (1min = 60000)
#define PHOTON_PREFIX_1       "Outside_" // Prefix for sensor

// Declaration
void dht_wrapper(); // must be declared before the lib initialization

// Lib instantiate
PietteTech_DHT DHT(DHTPIN, DHTTYPE, dht_wrapper);

// globals
unsigned int DHTnextSampleTime;	       // Next time we want to start sample
bool bDHTstarted;		                   // flag to indicate we started acquisition
int n;                                 // counter
double TempC;                          // Temperature from the sensor
double Humid;                          // Humidity from the sensor
int readStatus;                        // Status (int) when reading sensor
int rssi;                              // WiFi RSSI signal strength
int led = D7;                          // Onboard led

// this is coming from http://www.instructables.com/id/Datalogging-with-Spark-Core-Google-Drive/?ALLSTEPS
char resultstr[64]; //String to store the sensor data

void setup()
{
  DHTnextSampleTime = 0;  // Start the first sample immediately
  Particle.variable("result", resultstr, STRING);

  Particle.variable("readStatus", readStatus);
  Particle.variable("Temperature", TempC);
  Particle.variable("Humidity", Humid);
  Particle.variable("RSSI", rssi);

  pinMode(led, OUTPUT);
}

// This wrapper is in charge of calling
// must be defined like this for the lib work
void dht_wrapper() {
  DHT.isrCallback();
}

void loop()
{
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
        Particle.publish((String)PHOTON_PREFIX_1 + "Status", "Read sensor: OK", 60, PRIVATE);
        readStatus = DHTLIB_OK;
        Particle.publish((String)PHOTON_PREFIX_1 + "StateCode", String(DHTLIB_OK), 60, PRIVATE);
        break;
        case DHTLIB_ERROR_CHECKSUM:
        Particle.publish((String)PHOTON_PREFIX_1 + "Status", "Read sensor: Error - Checksum error", 60, PRIVATE);
        readStatus = DHTLIB_ERROR_CHECKSUM;
        Particle.publish((String)PHOTON_PREFIX_1 + "StateCode", String(DHTLIB_ERROR_CHECKSUM), 60, PRIVATE);
        break;
        case DHTLIB_ERROR_ISR_TIMEOUT:
        Particle.publish((String)PHOTON_PREFIX_1 + "Status", "Read sensor: Error - ISR time out error", 60, PRIVATE);
        readStatus = DHTLIB_ERROR_ISR_TIMEOUT;
        Particle.publish((String)PHOTON_PREFIX_1 + "StateCode", String(DHTLIB_ERROR_ISR_TIMEOUT), 60, PRIVATE);
        System.reset();
        break;
        case DHTLIB_ERROR_RESPONSE_TIMEOUT:
        Particle.publish((String)PHOTON_PREFIX_1 + "Status", "Read sensor: Error - Response time out error", 60, PRIVATE);
        readStatus = DHTLIB_ERROR_RESPONSE_TIMEOUT;
        Particle.publish((String)PHOTON_PREFIX_1 + "StateCode", String(DHTLIB_ERROR_RESPONSE_TIMEOUT), 60, PRIVATE);
        break;
        case DHTLIB_ERROR_DATA_TIMEOUT:
        Particle.publish((String)PHOTON_PREFIX_1 + "Status", "Read sensor: Error - Data time out error", 60, PRIVATE);
        readStatus = DHTLIB_ERROR_DATA_TIMEOUT;
        Particle.publish((String)PHOTON_PREFIX_1 + "StateCode", String(DHTLIB_ERROR_DATA_TIMEOUT), 60, PRIVATE);
        break;
        case DHTLIB_ERROR_ACQUIRING:
        Particle.publish((String)PHOTON_PREFIX_1 + "Status", "Read sensor: Error - Acquiring", 60, PRIVATE);
        readStatus = DHTLIB_ERROR_ACQUIRING;
        Particle.publish((String)PHOTON_PREFIX_1 + "StateCode", String(DHTLIB_ERROR_ACQUIRING), 60, PRIVATE);
        break;
        case DHTLIB_ERROR_DELTA:
        Particle.publish((String)PHOTON_PREFIX_1 + "Status", "Read sensor: Error - Delta time to small", 60, PRIVATE);
        readStatus = DHTLIB_ERROR_DELTA;
        Particle.publish((String)PHOTON_PREFIX_1 + "StateCode", String(DHTLIB_ERROR_DELTA), 60, PRIVATE);
        break;
        case DHTLIB_ERROR_NOTSTARTED:
        Particle.publish((String)PHOTON_PREFIX_1 + "Status", "Read sensor: Error - Not started", 60, PRIVATE);
        readStatus = DHTLIB_ERROR_NOTSTARTED;
        Particle.publish((String)PHOTON_PREFIX_1 + "StateCode", String(DHTLIB_ERROR_NOTSTARTED), 60, PRIVATE);
        break;
        default:
        Particle.publish((String)PHOTON_PREFIX_1 + "Status", "Read sensor: Unknown error", 60, PRIVATE);
        readStatus = -8;
        Particle.publish((String)PHOTON_PREFIX_1 + "StateCode", String(-8), 60, PRIVATE);
        break;
      }

      if (readStatus == 0) {
        TempC = round(DHT.getCelsius()*10)/10.0;
        Particle.publish((String)PHOTON_PREFIX_1"Temperature", String(TempC), 60, PRIVATE);

        Humid = round(DHT.getHumidity()*10)/10.0;
        Particle.publish((String)PHOTON_PREFIX_1 + "Humidity", String(Humid), 60, PRIVATE);

        // WiFi signal strength
        rssi = WiFi.RSSI();

        // Put led on for a while after successfully read sensor data
        digitalWrite(led, HIGH);
        delay(5000);
        digitalWrite(led, LOW);
      }

      n++;  // increment counter
      bDHTstarted = false;  // reset the sample flag so we can take another
      DHTnextSampleTime = millis() + DHT_SAMPLE_INTERVAL;  // set the time for next sample
    }
  }
}
