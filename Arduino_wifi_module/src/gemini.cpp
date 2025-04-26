#include <WiFiS3.h>
#include <WiFiSSLClient.h>
#include <ArduinoJson.h>
#include "secrets.h" // Include your secrets
#include <HTTPClient.h>
#include <arduino.h>

// Network details
const char ssid[] = SSID;
const char password[] = WIFI_PASSWORD;
int status = WL_IDLE_STATUS;

// Gemini API details
const char* geminiHost = "generativelanguage.googleapis.com";
const String geminiApiKey = API_KEY;
// Adjust the model and path as needed for the specific Gemini API version/model
const String geminiPath = "/v1beta/models/gemini-2.0-flash:generateContent?key=" + geminiApiKey;
const int httpsPort = 443;

WiFiSSLClient client; // Use SSL client for HTTPS

// Function to connect to WiFi
void connectWiFi() {
    while (status != WL_CONNECTED) {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(ssid);
        status = WiFi.begin(ssid, password);
        // Wait 10 seconds for connection:
        delay(10000);
    }
    Serial.println("\nConnected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

// Wrapper function to call the Gemini API
String callGeminiAPI(String prompt) {
    String responseText = "";
    JsonDocument requestDoc; // Use JsonDocument for request (adjust size as needed)
    String requestBody;

    // Construct the JSON request body
    // This structure might vary based on the specific Gemini API endpoint/model
    JsonObject contents = requestDoc.createNestedObject("contents");
    JsonObject parts = contents.createNestedArray("parts").createNestedObject();
    parts["text"] = prompt;

    serializeJson(requestDoc, requestBody);

    Serial.println("Connecting to Gemini API...");
    if (client.connect(geminiHost, httpsPort)) {
        Serial.println("Connected to server.");
        // Make the HTTP POST request
        client.println("POST " + geminiPath + " HTTP/1.1");
        client.print("Host: ");
        client.println(geminiHost);
        client.println("Content-Type: application/json");
        client.print("Content-Length: ");
        client.println(requestBody.length());
        client.println("Connection: close");
        client.println(); // End of headers
        client.println(requestBody); // Send the JSON body

        Serial.println("Request sent. Waiting for response...");

        // Basic response reading (timeouts and error checking can be improved)
        unsigned long timeout = millis();
        while (client.available() == 0) {
            if (millis() - timeout > 15000) { // 15 second timeout
                Serial.println(">>> Client Timeout !");
                client.stop();
                return "Error: Timeout";
            }
        }
      // Read the response headers (optional, can be skipped)
        while (client.available()) {
            String line = client.readStringUntil('\n');
            if (line == "\r") { // Look for the empty line separating headers and body
                // Serial.println("Headers received");
                break;
            }
        }

        // Read the response body
        String responseBody = "";
        while(client.available()){
            responseBody += (char)client.read();
        }
        client.stop(); // Disconnect
        Serial.println("Response received.");

        // Find the first occurrence of '{'
        int jsonStart = responseBody.indexOf('{');
        if (jsonStart > 0) {
            responseBody = responseBody.substring(jsonStart);
            //Serial.println("Trimmed response body to start with '{'");
        }

        //Serial.println("Raw Response Body:");
        //Serial.println(responseBody);

        // Parse the JSON response
        JsonDocument responseDoc; // Adjust size based on expected response complexity
        DeserializationError error = deserializeJson(responseDoc, responseBody);

        if (error) {
            Serial.print("deserializeJson() failed: ");
            Serial.println(error.c_str());
            Serial.println("Response Body causing error:");
            Serial.println(responseBody); // Print the problematic response
            return "Error: Failed to parse JSON response";
        }

        // Extract the generated text
        // Adjust the path based on the actual Gemini API response structure
        // Example path: responseDoc["candidates"][0]["content"]["parts"][0]["text"]
        if (responseDoc.containsKey("candidates") &&
            responseDoc["candidates"][0].containsKey("content") &&
            responseDoc["candidates"][0]["content"].containsKey("parts") &&
            responseDoc["candidates"][0]["content"]["parts"][0].containsKey("text")) {
            responseText = responseDoc["candidates"][0]["content"]["parts"][0]["text"].as<String>();
        } else {
             Serial.println("Could not find text in response structure.");
             Serial.println("Response JSON:");
             serializeJsonPretty(responseDoc, Serial); // Print parsed JSON for debugging
             Serial.println();
             responseText = "Error: Could not parse expected text from response.";
        }

    } else {
        Serial.println("Connection failed");
        responseText = "Error: Connection failed";
    }
    return responseText;
}