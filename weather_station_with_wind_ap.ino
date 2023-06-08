 #include "DHT.h" 
 #include <ESP8266WiFi.h>
 #include <ESP8266WebServer.h>
 #include <SFE_BMP180.h>
 #include <Wire.h> // Library for I2C communication
 #include <Adafruit_Sensor.h>
 #include <Adafruit_HMC5883_U.h>
 
 #define ALTITUDE 20 // Altitude of Vonitsa in meters 

/* Put your SSID & Password */
const char* ssid = "Weather_station_Vonitsa";  // Enter SSID here
const char* password = "12345";  //Enter Password here

/* Put IP Address details */
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

ESP8266WebServer server(80);  
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);
SFE_BMP180 BMP_pres;
DHT dht2(14, DHT11); 

float temperature, humidity, pressure, wind_speed;
String wind_direction;
 
 void setup()  
 {  

  Serial.begin(115200);

  //Set access point
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);
  
  server.on("/", handle_OnConnect);
  
  server.onNotFound(handle_NotFound);
  
  server.begin();
  Serial.println("HTTP server started");


 // Initialize the I2C bus (BH1750 library doesn't do this automatically)
  Wire.begin();
  // On esp8266 you can select SCL and SDA pins using Wire.begin(D4, D3);
  // For Wemos / Lolin D1 Mini Pro and the Ambient Light shield use 
  //Wire.begin(D2, D1);

  dht2.begin();
  
  if (BMP_pres.begin())
    Serial.println("BMP180 init success");
  else
  {
    Serial.println("BMP180 init fail\n\n");
    while(1); // Pause forever.
  }

  if(!mag.begin())
  {
    /* There was a problem detecting the HMC5883 ... check your connections */
    Serial.println("Ooops, no HMC5883 detected ... Check your wiring!");
    while(1);
  }
  
  delay(500); 
 }  
 
 void loop()  
 {  
 Serial.println("HTTP server started");
 server.handleClient();
 }

void handle_OnConnect() {
 
  char status;
  double Te,P;
  
  temperature = dht2.readTemperature( );
  humidity = dht2.readHumidity( );

status = BMP_pres.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:
    delay(status);
 
    status = BMP_pres.getTemperature(Te);
    if (status != 0)
    {
      status = BMP_pres.startPressure(3);
      if (status != 0)
      {
        // Wait for the measurement to complete:
        delay(status);
 
        status = BMP_pres.getPressure(P,Te);
        if (status != 0)
        {
          Serial.print("Pressure: ");
          Serial.print(P);
          Serial.println(" hPa");
         }
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");

  wind_speed = get_wind_speed();
  wind_direction = get_wind_direction();
  server.send(200, "text/html", SendHTML(temperature,humidity,P,ALTITUDE,wind_speed, wind_direction )); 
}

float get_wind_speed(){
  
  float sensorValue = analogRead(A0);
  Serial.print("Analog Value =");
  Serial.println(sensorValue);
 
  float voltage = (sensorValue / 1023) * 5;
  Serial.print("Voltage =");
  Serial.print(voltage);
  Serial.println(" V");
 
  float wind_speed = mapfloat(voltage, 0, 1.2, 0, 30);
  Serial.print("Wind Speed =");
  Serial.print(wind_speed);
  Serial.println("m/s");
  return wind_speed;
  
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

String get_wind_direction() {
  String cardinal;
  /* Get a new sensor event */ 
  sensors_event_t event; 
  mag.getEvent(&event);
 
  /* Display the results (magnetic vector values are in micro-Tesla (uT)) */
  Serial.print("X: "); Serial.print(event.magnetic.x); Serial.print("  ");
  Serial.print("Y: "); Serial.print(event.magnetic.y); Serial.print("  ");
  Serial.print("Z: "); Serial.print(event.magnetic.z); Serial.print("  ");Serial.println("uT");

  // Hold the module so that Z is pointing 'up' and you can measure the heading with x&y
  // Calculate heading when the magnetometer is level, then correct for signs of axis.
  float headingRadians = atan2(event.magnetic.y, event.magnetic.x);
  float headingDegrees = headingRadians * 180 / PI;
  float declinationAngle = 11.41666666666667;

  headingDegrees += declinationAngle;
  
  if (headingDegrees < 0) {
    headingDegrees += 360;
  }

  
  if (headingDegrees > 202 && headingDegrees < 215) {
    cardinal = " S";
  }
  else if (headingDegrees > 189 && headingDegrees < 202) {
    cardinal = " SE";
  }
  else if (headingDegrees > 174 && headingDegrees < 189) {
    cardinal = " E";
  }
  else if (headingDegrees > 277 && headingDegrees < 290) {
    cardinal = " N";
  }
  else if (headingDegrees > 253 && headingDegrees < 277) {
    cardinal = " NW";
  }
  else if (headingDegrees > 240 && headingDegrees < 253) {
    cardinal = " W";
  }
  else if (headingDegrees > 215 && headingDegrees < 240) {
    cardinal = " SW";
  }
 if ((event.magnetic.x > -5 && event.magnetic.x < 5) && (headingDegrees < 277 || headingDegrees > 290)) {
    cardinal = " NE";
 }

  Serial.print("Heading: ");
  Serial.print(headingDegrees);
  Serial.println(cardinal);
  return cardinal;
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float temperature,float humidity,float pressure,float altitude, float wind_speed, String wind_direction){
  String ptr = "<!DOCTYPE html>";
  ptr +="<html>";
  ptr +="<head>";
  ptr +="<title>Vonitsa Meteo</title>";
  ptr +="<meta http-equiv='content-type' content='text/html;charset=utf-8' />";
  ptr +="<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  ptr +="<meta http-equiv='refresh' content='2' >";
  ptr +="<link href='https://fonts.googleapis.com/css?family=Open+Sans:300,400,600' rel='stylesheet'>";
  ptr +="<style>";
  ptr +="html { font-family: 'Open Sans', sans-serif; display: block; margin: 0px auto; text-align: center;color: #444444;}";
  ptr +="body{margin: 0px;} ";
  ptr +="h1 {margin: 50px auto 30px;} ";
  ptr +=".side-by-side{display: table-cell;vertical-align: middle;position: relative;}";
  ptr +=".text{font-weight: 600;font-size: 19px;width: 200px;}";
  ptr +=".reading{font-weight: 300;font-size: 50px;padding-right: 25px;}";
  ptr +=".temperature .reading{color: #F29C1F;}";
  ptr +=".humidity .reading{color: #3B97D3;}";
  ptr +=".pressure .reading{color: #26B99A;}";
  ptr +=".altitude .reading{color: #955BA5;}";
  ptr +=".wind_s .reading{color: #4633FF;}";
  ptr +=".wind_d .reading{color: #33FFF6;}";
  ptr +=".superscript{font-size: 17px;font-weight: 600;position: absolute;top: 10px;}";
  ptr +=".data{padding: 10px;}";
  ptr +=".container{display: table;margin: 0 auto;}";
  ptr +=".icon{width:65px}";
  ptr +="</style>";
  ptr +="</head>";
  ptr +="<body>";
  ptr +="<h1>Μετεωρολογικός Σταθμός Βόνιτσας</h1>";
  ptr +="<div class='container'>";
  
  ptr +="<div class='data temperature'>";
  ptr +="<div class='side-by-side icon'>";
  ptr +="<svg enable-background='new 0 0 19.438 54.003'height=54.003px id=Layer_1 version=1.1 viewBox='0 0 19.438 54.003'width=19.438px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><g><path d='M11.976,8.82v-2h4.084V6.063C16.06,2.715,13.345,0,9.996,0H9.313C5.965,0,3.252,2.715,3.252,6.063v30.982";
  ptr +="C1.261,38.825,0,41.403,0,44.286c0,5.367,4.351,9.718,9.719,9.718c5.368,0,9.719-4.351,9.719-9.718";
  ptr +="c0-2.943-1.312-5.574-3.378-7.355V18.436h-3.914v-2h3.914v-2.808h-4.084v-2h4.084V8.82H11.976z M15.302,44.833";
  ptr +="c0,3.083-2.5,5.583-5.583,5.583s-5.583-2.5-5.583-5.583c0-2.279,1.368-4.236,3.326-5.104V24.257C7.462,23.01,8.472,22,9.719,22";
  ptr +="s2.257,1.01,2.257,2.257V39.73C13.934,40.597,15.302,42.554,15.302,44.833z'fill=#F29C21 /></g></svg>";
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>Θερμοκρασία</div>";
  ptr +="<div class='side-by-side reading'>";
  ptr +=(int)temperature;
  ptr +="<span class='superscript'>&deg;C</span></div>";
  ptr +="</div>";

  ptr +="<div class='data wind_s'>";
  ptr +="<div class='side-by-side icon'>";
  ptr +="<svg enable-background='new 0 0 19.438 54.003'height=54.003px id=Layer_1 version=1.1 viewBox='0 0 19.438 54.003'width=19.438px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg y=0px><g>";
  ptr +="<path d='M564.352484 386.978714H71.006413a22.378321 22.378321 0 1 1 0-44.756642h493.346071c63.074896 0 114.39478-51.319884 114.39478-114.397977S627.42738 113.426118 564.352484 113.426118a22.378321 22.378321 0 1 1 0-44.756642c87.754987 0 159.151422 71.396435 159.151422 159.154619s-71.396435 159.154619-159.151422 159.154619z' fill='#4F46A3' />";
  ptr +="<path d='M815.916781 498.790397h-493.346071a22.378321 22.378321 0 1 1 0-44.756642h493.346071c63.078093 0 114.397977-51.319884 114.397977-114.404371 0-63.078093-51.319884-114.397977-114.397977-114.397977a22.378321 22.378321 0 1 1 0-44.756642c87.758184 0 159.154619 71.396435 159.154619 159.154619 0.003197 87.761381-71.393238 159.161013-159.154619";
  ptr +="159.161013zM704.111492 954.051762a22.378321 22.378321 0 1 1 0-44.756642c63.078093 0 114.397977-51.319884 114.397977-114.401174 0-63.078093-51.319884-114.397977-114.397977-114.397976H210.762224a22.378321 22.378321 0 1 1 0-44.756642h493.346071c87.758184 0 159.154619 71.396435 159.154619 159.154618s-71.393238 159.157816-159.151422 159.157816z' fill='#4F46A3' />";
  ptr +="<path d='M572.226456 313.449945H78.880385a22.378321 22.378321 0 1 1 0-44.756642h493.346071a22.378321 22.378321 0 1 1 0 44.756642z' fill='#5FCEFF' /><path d='M815.191084 588.383603h-493.346071a22.378321 22.378321 0 1 1 0-44.756642h493.346071a22.378321 22.378321 0 1 1 0 44.756642z' fill='#FF4893' /></svg>";
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>Ταχύτητα Ανέμου</div>";
  ptr +="<div class='side-by-side reading'>";
  ptr +=(int)wind_speed;
  ptr +="<span class='superscript'>m/sec</span></div>";
  ptr +="</div>";


  ptr +="<div class='data wind_d'>";
  ptr +="<div class='side-by-side icon'>";
  ptr +="<svg enable-background='new 0 0 19.438 54.003'height=54.003px id=Layer_1 version=1.1 viewBox='0 0 19.438 54.003'width=19.438px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg y=0px><g>";
  ptr +="<path d='M229.056 229.077333l282.986667 150.933334 282.922666-150.933334-150.954666 282.88 150.954666 282.965334-283.008-150.912-282.901333 150.912 150.933333-282.88z' fill='#BCAAA4' /><path d='M229.056 229.077333l282.986667 150.933334L512 512zM794.965333 794.922667l-283.008-150.912L512 512zM794.965333 229.077333l-150.954666 282.88L512 512zM229.056 794.922667l150.933333-282.88L512 512z' fill='#795548' />";
  ptr +="<path d='M512 64l104.533333 343.552L960 512l-343.466667 104.448L512 960l-104.533333-343.552L64 512l343.466667-104.448z' fill='#90CAF9' /><path d='M512 64l104.533333 343.552L512 512zM512 960l-104.533333-343.552L512 512zM960 512l-343.466667 104.448L512 512zM64 512l343.466667-104.448L512 512z' fill='#1976D2' /></svg>";
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>Κατεύθυνση Ανέμου</div>";
  ptr +="<div class='side-by-side reading'>";
  ptr +=(int)wind_direction;
  ptr +="<span class='superscript'></span></div>";
  ptr +="</div>";

 
  ptr +="<div class='data humidity'>";
  ptr +="<div class='side-by-side icon'>";
  ptr +="<svg enable-background='new 0 0 29.235 40.64'height=40.64px id=Layer_1 version=1.1 viewBox='0 0 29.235 40.64'width=29.235px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><path d='M14.618,0C14.618,0,0,17.95,0,26.022C0,34.096,6.544,40.64,14.618,40.64s14.617-6.544,14.617-14.617";
  ptr +="C29.235,17.95,14.618,0,14.618,0z M13.667,37.135c-5.604,0-10.162-4.56-10.162-10.162c0-0.787,0.638-1.426,1.426-1.426";
  ptr +="c0.787,0,1.425,0.639,1.425,1.426c0,4.031,3.28,7.312,7.311,7.312c0.787,0,1.425,0.638,1.425,1.425";
  ptr +="C15.093,36.497,14.455,37.135,13.667,37.135z'fill=#3C97D3 /></svg>";
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>Υγρασία</div>";
  ptr +="<div class='side-by-side reading'>";
  ptr +=(int)humidity;
  ptr +="<span class='superscript'>%</span></div>";
  ptr +="</div>";
  
  ptr +="<div class='data pressure'>";
  ptr +="<div class='side-by-side icon'>";
  ptr +="<svg enable-background='new 0 0 40.542 40.541'height=40.541px id=Layer_1 version=1.1 viewBox='0 0 40.542 40.541'width=40.542px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><g><path d='M34.313,20.271c0-0.552,0.447-1,1-1h5.178c-0.236-4.841-2.163-9.228-5.214-12.593l-3.425,3.424";
  ptr +="c-0.195,0.195-0.451,0.293-0.707,0.293s-0.512-0.098-0.707-0.293c-0.391-0.391-0.391-1.023,0-1.414l3.425-3.424";
  ptr +="c-3.375-3.059-7.776-4.987-12.634-5.215c0.015,0.067,0.041,0.13,0.041,0.202v4.687c0,0.552-0.447,1-1,1s-1-0.448-1-1V0.25";
  ptr +="c0-0.071,0.026-0.134,0.041-0.202C14.39,0.279,9.936,2.256,6.544,5.385l3.576,3.577c0.391,0.391,0.391,1.024,0,1.414";
  ptr +="c-0.195,0.195-0.451,0.293-0.707,0.293s-0.512-0.098-0.707-0.293L5.142,6.812c-2.98,3.348-4.858,7.682-5.092,12.459h4.804";
  ptr +="c0.552,0,1,0.448,1,1s-0.448,1-1,1H0.05c0.525,10.728,9.362,19.271,20.22,19.271c10.857,0,19.696-8.543,20.22-19.271h-5.178";
  ptr +="C34.76,21.271,34.313,20.823,34.313,20.271z M23.084,22.037c-0.559,1.561-2.274,2.372-3.833,1.814";
  ptr +="c-1.561-0.557-2.373-2.272-1.815-3.833c0.372-1.041,1.263-1.737,2.277-1.928L25.2,7.202L22.497,19.05";
  ptr +="C23.196,19.843,23.464,20.973,23.084,22.037z'fill=#26B999 /></g></svg>";
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>Πίεση</div>";
  ptr +="<div class='side-by-side reading'>";
  ptr +=(int)pressure;
  ptr +="<span class='superscript'>hPa</span></div>";
  ptr +="</div>";
  ptr +="<div class='data altitude'>";
  ptr +="<div class='side-by-side icon'>";
  ptr +="<svg enable-background='new 0 0 58.422 40.639'height=40.639px id=Layer_1 version=1.1 viewBox='0 0 58.422 40.639'width=58.422px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><g><path d='M58.203,37.754l0.007-0.004L42.09,9.935l-0.001,0.001c-0.356-0.543-0.969-0.902-1.667-0.902";
  ptr +="c-0.655,0-1.231,0.32-1.595,0.808l-0.011-0.007l-0.039,0.067c-0.021,0.03-0.035,0.063-0.054,0.094L22.78,37.692l0.008,0.004";
  ptr +="c-0.149,0.28-0.242,0.594-0.242,0.934c0,1.102,0.894,1.995,1.994,1.995v0.015h31.888c1.101,0,1.994-0.893,1.994-1.994";
  ptr +="C58.422,38.323,58.339,38.024,58.203,37.754z'fill=#955BA5 /><path d='M19.704,38.674l-0.013-0.004l13.544-23.522L25.13,1.156l-0.002,0.001C24.671,0.459,23.885,0,22.985,0";
  ptr +="c-0.84,0-1.582,0.41-2.051,1.038l-0.016-0.01L20.87,1.114c-0.025,0.039-0.046,0.082-0.068,0.124L0.299,36.851l0.013,0.004";
  ptr +="C0.117,37.215,0,37.62,0,38.059c0,1.412,1.147,2.565,2.565,2.565v0.015h16.989c-0.091-0.256-0.149-0.526-0.149-0.813";
  ptr +="C19.405,39.407,19.518,39.019,19.704,38.674z'fill=#955BA5 /></g></svg>";
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>Υψόμετρο</div>";
  ptr +="<div class='side-by-side reading'>";
  ptr +=(int)altitude;
  ptr +="<span class='superscript'>m</span></div>";
  ptr +="</div>";
  ptr +="</div>";
  ptr +="</body>";
  ptr +="</html>";
  return ptr;
}
