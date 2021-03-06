#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include "Adafruit_Si7021.h"
#include <WiFiNINA.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include "Log_data.h"

byte mac_addr[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };                                                       // MAC Address of the uC
IPAddress server_addr(10,10,0,16);                                                                              // Destination IP to MySQL DataBase
char user[] = DB_USR;                                                                                                    
char password[] = DB_PSSW;                                                                                      

WiFiClient client;
MySQL_Connection conn((Client *)&client);                                                                       

Adafruit_BMP280 bmp;
Adafruit_Si7021 sensor = Adafruit_Si7021();

int sensor_temp, sensor_temp_env, sensor_pres, sensor_humi;
int status = WL_IDLE_STATUS;

char INSERT_DATA[] = "INSERT INTO sensors.sensors_data (temperature,temperature_env,humidity,pressure) VALUES (%d,%d,%d,%d)";      
char query[128];                                                                                                

unsigned long timer = millis(), saved_timer, diff_time, refresh_time = 600000; // <= SET THE FREQUENCY OF COLLECTING AND SENDING DATA (ms)

void ethernetConnection(){
  while (status != WL_CONNECTED) {
  status = WiFi.begin(WIFI_SSID, WIFI_PSSW);
  delay(10000);
  }
}

void databaseConnection()
{
  if (conn.connect(server_addr, 3306, user, password)) 
  {
    delay(1000);
    MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
    sprintf(query, INSERT_DATA, sensor_temp,sensor_temp_env, sensor_humi, sensor_pres);
    cur_mem->execute(query);
    delete cur_mem;
  }
  conn.close();
}

void setup() 
{
  //Serial.begin(9600);
  ethernetConnection();
  sensor.begin();
  bmp.begin(0x76);
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
  
}

void debugOutputData() //DEBUG FUNCTION TO READ RAW DATA FROM SENSORS
{

  Serial.println("=======[ DEBUG DATA ]=======");

  Serial.print("BMP280 | TEMP: ");
  Serial.print(bmp.readTemperature());
  Serial.println(" *C");

  Serial.print("BMP280 | PRES: ");
  Serial.print(sensor_pres);
  Serial.println(" hPa");

  Serial.print("SI7021 | HUMI: ");
  Serial.print(sensor_humi);
  Serial.println(" %");
  Serial.print("SI7021 | TEMP: ");
  Serial.print(sensor.readTemperature(), 2);
  Serial.println(" *C");
  delay(2000);
}

void getDataFromSensors()
{
  sensor_temp = bmp.readTemperature();
  sensor_temp_env = sensor.readTemperature();
  sensor_pres = ((bmp.readPressure()/100)+20.34);
  sensor_humi = sensor.readHumidity();
}

void loop() 
{

  timer = millis();
  diff_time = timer - saved_timer;

  if(diff_time >= refresh_time)
  {
      saved_timer = timer;
      getDataFromSensors();
      databaseConnection();
  }

//delay(2000);
//getDataFromSensors();
//debugOutputData();
}
