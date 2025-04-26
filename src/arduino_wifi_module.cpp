#include <Arduino.h>
#include <WiFiS3.h> // Library for Arduino Uno R4 WiFi
#include <HTTPClient.h>
#include <WiFiClientSecure.h> // Required for HTTPS

// --- WiFi Credentials ---
const char* ssid = "YOUR_WIFI_SSID";      // Your WiFi network SSID
const char* password = "YOUR_WIFI_PASSWORD";  // Your WiFi password

// --- Gemini API Configuration ---
// WARNING: Storing API keys directly in code is INSECURE.
// This is for demonstration only. Use a secure method in production.
const char* geminiApiKey = "YOUR_GEMINI_API_KEY"; // Your Gemini API Key
const char* geminiHost = "generativelanguage.googleapis.com";
const int geminiPort = 443; // HTTPS port
const char* geminiEndpoint = "/v1beta/models/gemini-2.0-flash:generateContent?key="; // Using gemini-2.0-flash as requested

// --- Arduino Pin Definitions ---
const int LED_PIN = 13; // Example LED pin (Built-in LED)

// --- WiFi Client ---
WiFiClientSecure client;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // Wait for serial port to connect. Needed for native USB port only
  }

  Serial.println();
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  // Check for the presence of the WiFi module
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // Don't continue if the module is not present
    while (true);
  }

  // Attempt to connect to WiFi network
  int status = WL_IDLE_STATUS;
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, password);

    // wait 10 seconds for connection:
    delay(10000);
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Configure SSL/TLS (optional but recommended for security)
  // For production, you might need to set a root certificate.
  // For testing, you might use client.setInsecure(); but this is NOT recommended for production.
  // client.setInsecure(); // Use with caution for testing

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // Ensure LED is off initially

  Serial.println("Arduino is ready. Send prompts starting with 'gemini <prompt>'");
}

void loop() {
  // Check for serial input
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command.startsWith("gemini ")) {
      String userPrompt = command.substring(7);
      Serial.print("Sending prompt to Gemini: ");
      Serial.println(userPrompt);

      // Send prompt to Gemini and get response
      String geminiResponse = sendPromptToGemini(userPrompt);

      Serial.print("Gemini responded: ");
      Serial.println(geminiResponse);

      // --- Parse Gemini's response and execute command ---
      // This parsing logic needs to match the expected output format from Gemini.
      // If you use few-shot prompting in the prompt sent to Gemini,
      // you would expect specific command strings like "LED_ON".
      if (geminiResponse.equals("LED_ON")) {
        Serial.println("Interpreted Gemini response as 'LED_ON'. Turning LED ON.");
        digitalWrite(LED_PIN, HIGH);
      } else if (geminiResponse.equals("LED_OFF")) {
        Serial.println("Interpreted Gemini response as 'LED_OFF'. Turning LED OFF.");
        digitalWrite(LED_PIN, LOW);
      } else if (geminiResponse.equals("STATUS")) {
        Serial.println("Interpreted Gemini response as 'STATUS'. Reading LED status.");
        int ledState = digitalRead(LED_PIN);
        if (ledState == HIGH) {
          Serial.println("LED status: ON");
        } else {
          Serial.println("LED status: OFF");
        }
      } else {
        Serial.println("Gemini response did not contain a recognized command for Arduino.");
      }
      // --- End Parsing and Execution ---

    } else {
      Serial.println("Unknown command. Use 'gemini <prompt>' to send to Gemini.");
    }
  }

  // Add a small delay
  delay(10);
}

// Function to send prompt to Gemini and get response
String sendPromptToGemini(String prompt) {
  String response = "";

  // Ensure client is connected before making HTTP request
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected. Cannot send request.");
    return "";
  }

  HTTPClient http;
  // Use the secure client for HTTPS
  http.begin(client, geminiHost, geminiPort, String(geminiEndpoint) + geminiApiKey);
  http.addHeader("Content-Type", "application/json");

  // Construct the JSON payload
  // You would ideally include few-shot examples here to guide Gemini's output
  String jsonPayload = "{\"contents\": [{\"parts\": [{\"text\": \"" + prompt + "\"}]}]}";

  Serial.print("Sending POST request to: ");
  Serial.println(String(geminiHost) + geminiEndpoint + geminiApiKey);
  Serial.print("Payload: ");
  Serial.println(jsonPayload);

  // Send the POST request
  int httpResponseCode = http.POST(jsonPayload);

  if (httpResponseCode > 0) {
    Serial.printf("[HTTP] POST... code: %d\n", httpResponseCode);

    if (httpResponseCode == HTTP_CODE_OK) {
      // Get the response payload
      String payload = http.getString();
      Serial.println("Received payload:");
      Serial.println(payload);

      // --- Basic JSON Parsing (Simplified) ---
      // This is a very basic way to extract text.
      // For robust parsing, use a JSON library like ArduinoJson.
      // We are looking for the "text" field within the "parts" array
      int textStartIndex = payload.indexOf("\"text\": \"");
      if (textStartIndex != -1) {
        textStartIndex += 9; // Move past "\"text\": \""
        int textEndIndex = payload.indexOf("\"", textStartIndex); // Find the next quote
        if (textEndIndex != -1) {
          response = payload.substring(textStartIndex, textEndIndex);
          // Basic unescaping of common escape sequences (e.g., \\n becomes \n)
           response.replace("\\n", "\n");
           response.replace("\\\"", "\"");
           response.replace("\\\\", "\\"); // Unescape backslashes
        }
      }
      // --- End Basic JSON Parsing ---

    }
  } else {
    Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpResponseCode).c_str());
  }

  http.end(); // Free resources

  return response;
}
