#include "password.h"

const char* ssid = "SpectrumSetup-E0";
const char* serverName = "http://192.168.1.145:6666/";

void wifi_setup(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  Serial.print("\nConnecting");
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(100);
  }
  Serial.println(" Connected!");
}
void sendPostData(String jsonPayload, String endpoint){
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin(serverName + endpoint);
    http.addHeader("Content-Type", "application/json");


    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Server response: " + response);
    } else {
      Serial.print("Error in POST: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}

void sendFitnessData(uint32_t totalSteps, uint8_t swole_level) {
      // Create JSON payload
    String jsonPayload = "{\"steps\": " + String(totalSteps) + 
                         ", \"swole\": " + String(swole_level) + "}";
    sendPostData(jsonPayload, "FitnessData");
}