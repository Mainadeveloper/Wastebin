#include <WiFi.h> 
#include <HTTPClient.h>
#include "HX711.h"

const char* ssid = "Wafa";
const char* password = "AnFaWaMu2002!";

const char* THINGSPEAK_API_KEY = "2E1NW7LV3SGFIBPU";
const char* THINGSPEAK_URL = "http://api.thingspeak.com/update";

#define TRIG_PIN 18
#define ECHO_PIN 19

#define HX711_DT 26
#define HX711_SCK 25

#define SOUND_SPEED 0.034   
#define BIN_HEIGHT 35.0     
#define CALIBRATION_FACTOR 6245.0  

HX711 scale;

void connectWiFi() {
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.println("\nConnected to WiFi");
}

float getDistance() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH);
    float distance = (duration * SOUND_SPEED) / 2.0;

    if (distance < 0) distance = 0;
    if (distance > BIN_HEIGHT) distance = BIN_HEIGHT;

    return distance;
}

String getBinStatus(float distance) {
    if (distance >= 0 && distance <= 10) return "Full";
    else if (distance > 10 && distance <= 20) return "Medium";
    else if (distance > 20 && distance <= BIN_HEIGHT) return "Empty";
    else return "Invalid";
}


float getWeight() {
    return scale.get_units(10);  
}

void sendToThingSpeak(float distance, float weight) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String url = String(THINGSPEAK_URL) + "?api_key=" + THINGSPEAK_API_KEY +
                     "&field1=" + String(distance) +
                     "&field2=" + String(weight);

        http.begin(url);
        int httpResponseCode = http.GET();
        if (httpResponseCode > 0) {
            Serial.println("Data sent to ThingSpeak successfully.");
        } else {
            Serial.print("Error sending data. HTTP response code: ");
            Serial.println(httpResponseCode);
        }
        http.end();
    } else {
        Serial.println("WiFi not connected, cannot send data.");
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    scale.begin(HX711_DT, HX711_SCK);
    scale.set_scale(CALIBRATION_FACTOR);  
    scale.tare();  

    connectWiFi();
}

void loop() {
    float distance = getDistance();
    float weight = getWeight();
    String binStatus = getBinStatus(distance);

    Serial.print("Bin Level: ");
    Serial.print(distance, 1);  
    Serial.print(" cm (");
    Serial.print(binStatus);
    Serial.println(")");

    Serial.print("Weight: ");
    Serial.print(weight, 2);    
    Serial.println(" kg");

    sendToThingSpeak(distance, weight);

    delay(3600000); 
}

