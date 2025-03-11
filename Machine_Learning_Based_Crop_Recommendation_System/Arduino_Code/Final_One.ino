#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

#define PH_OFFSET -1.00 //if there is an offset
#define SensorPin A0        // the pH meter Analog output is connected with the Arduino’s Analog
unsigned long int avgValue;  // Store the average value of the sensor feedback
float b;
int buf[10], temp;

const char *ssid = "Redmi Note 12 Pro 5G";
const char *password = "om@475786007";


WebServer server(80);
DHT dht(4, DHT11); // For ESP8266, DHT sensor data pin is D2

void handleRoot() {
  char msg[1500];

  snprintf(msg, 1500,
           "<html>\
  <head>\
    <meta http-equiv='refresh' content='4'/>\
    <meta name='viewport' content='width=device-width, initial-scale=1'>\
    <link rel='stylesheet' href='https://use.fontawesome.com/releases/v5.7.2/css/all.css' integrity='sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr' crossorigin='anonymous'>\
    <title>ESP8266 DHT pH Server</title>\
    <style>\
    html { font-family: Arial; display: inline-block; margin: 0px auto; text-align: center;}\
    h2 { font-size: 3.0rem; }\
    p { font-size: 3.0rem; }\
    .units { font-size: 1.2rem; }\
    .dht-labels{ font-size: 1.5rem; vertical-align:middle; padding-bottom: 15px;}\
    </style>\
  </head>\
  <body>\
      <h2>ESP32 DHT pH Server!</h2>\
      <p>\
        <i class='fas fa-thermometer-half' style='color:#ca3517;'></i>\
        <span class='dht-labels'>Temperature</span>\
        <span>%.2f</span>\
        <sup class='units'>&deg;C</sup>\
      </p>\
      <p>\
        <i class='fas fa-tint' style='color:#00add6;'></i>\
        <span class='dht-labels'>Humidity</span>\
        <span>%.2f</span>\
        <sup class='units'>&percnt;</sup>\
      </p>\
      <p>\
        <i class='fas fa-flask' style='color:#0080ff;'></i>\
        <span class='dht-labels'>pH Value</span>\
        <span>%.2f</span>\
      </p>\
  </body>\
</html>",
           readDHTTemperature(), readDHTHumidity(), readPHValue()
          );
  server.send(200, "text/html", msg);
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);

  server.begin();
  Serial.println("HTTP server started");
  pinMode(13, OUTPUT);  // assuming LED output for pH indication
}

void loop() {
  server.handleClient();
  delay(20); // allow the CPU to switch to other tasks
}

float readDHTTemperature() {
  // Sensor readings may also take up to 2 seconds
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  if (isnan(t)) {    
    Serial.println("Failed to read from DHT sensor!");
    return -1;
  }
  else {
    Serial.println(t);
    return t;
  }
}

float readDHTHumidity() {
  // Sensor readings may also take up to 2 seconds
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return -1;
  }
  else {
    Serial.println(h);
    return h;
  }
}

float readPHValue() {
  for(int i = 0; i < 10; i++) {       // Get 10 sample value from the sensor for smoothing the value
    buf[i] = analogRead(SensorPin);
    delay(10);
  }
  for(int i = 0; i < 9; i++) {        // Sort the analog values from small to large
    for(int j = i + 1; j < 10; j++) {
      if(buf[i] > buf[j]) {
        temp = buf[i];
        buf[i] = buf[j];
        buf[j] = temp;
      }
    }
  }
  avgValue = 0;
  for(int i = 2; i < 8; i++) {                      // Take the average value of 6 center samples
    avgValue += buf[i];
  }
  float phValue = (float)avgValue * 5.0/4095/6; // Convert the analog value into millivolts
  phValue = 3.5 * phValue;                          // Convert the millivolts into pH value

  phValue = phValue + PH_OFFSET;

  Serial.print("pH: ");  
  Serial.println(phValue, 2);
  return phValue;
}
