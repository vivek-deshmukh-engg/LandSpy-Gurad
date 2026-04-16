#include <WiFi.h>
#include <WebServer.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

const char* ssid = "Vvk";
const char* password = "12345678";

WebServer server(80);
const int RXPin = 4, TXPin = 5;
const uint32_t GPSBaud = 9600;
HardwareSerial gps_module(1);
TinyGPSPlus gps;

int landmine_detect = 0;
float latitude, longitude, gps_speed;
int no_of_satellites;
String satellite_orientation;
String detected_locations = "";

void setup() {
  Serial.begin(115200);
  gps_module.begin(GPSBaud, SERIAL_8N1, RXPin, TXPin);
  pinMode(13, INPUT);

  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient();
  landmine_detect = digitalRead(13);
  
  while (gps_module.available() > 0) {
    gps.encode(gps_module.read());
  }
  if (gps.location.isValid()) {
    latitude = gps.location.lat();
    longitude = gps.location.lng();
    gps_speed = gps.speed.kmph();
    no_of_satellites = gps.satellites.value();
    satellite_orientation = String(gps.course.deg(), 2);
  }
  
  if (landmine_detect) {
    detected_locations += "<p>Landmine at: " + String(latitude, 6) + ", " + String(longitude, 6) + " - <a href='https://www.google.com/maps/search/?api=1&query=" + String(latitude, 6) + "%2C" + String(longitude, 6) + "' target='_blank'>View on Map</a></p>";
  }
}

void handleRoot() {
  String html = "<html><head><title>LandSpy Guard</title>";
  html += "<meta http-equiv='refresh' content='5'>";
  html += "<style>body{font-family:Arial;background-color:#2C3E50;color:#ECF0F1;text-align:center;padding:20px;}h1{color:#E74C3C;} .container{border: 2px solid #E74C3C;padding: 15px;border-radius:10px;display:inline-block;}</style>";
  html += "</head><body>";
  html += "<h1>LandSpy Guard</h1>";
  html += "<div class='container'>";
  html += "<p><b>Speed:</b> " + String(gps_speed, 2) + " km/h</p>";
  html += "<p><b>Satellites:</b> " + String(no_of_satellites) + "</p>";
  html += "<p><b>Orientation:</b> " + satellite_orientation + "</p>";
  html += "<p><b>Landmine Detected:</b> " + String(landmine_detect ? "YES" : "NO") + "</p>";
  html += "</div>";
  
  if (landmine_detect) {
    html += "<div class='container'><p><b>Latitude:</b> " + String(latitude, 6) + "</p>";
    html += "<p><b>Longitude:</b> " + String(longitude, 6) + "</p>";
    html += "<p><b>Google Maps:</b> <a href='https://www.google.com/maps/search/?api=1&query=" + String(latitude, 6) + "%2C" + String(longitude, 6) + "' target='_blank'>View Location</a></p></div>";
  }
  
  html += "<h2>Detection History</h2>";
  html += "<div class='container'>" + detected_locations + "</div>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}
