// Function prototypes from gemini.cpp
#include <arduino.h>
#include <WiFiS3.h> // Include WiFiS3.h for pinMode and digitalWrite


// Declare functions defined in gemini.cpp
extern void connectWiFi();
extern String callGeminiAPI(String prompt);
void LED_Control(String response);

// Variable to store the user's input string
String userInputString = "";

// Define the predefined context prompt template
// This template will be formatted with the user's input for each Gemini call.
// The {} placeholder indicates where the user's specific prompt will be inserted.
// This template is designed to guide Gemini to respond with specific commands
// like 'LED_ON' or 'LED_OFF' for easy parsing.
String context_prompt_template = R"(
Your only pupose is to analyse what the user asks, and must respond with ONLY one of the following commands:
'LED_ON', 'LED_OFF', 'STATUS', or 'QUIT'.
Do NOT include any other text, explanations, or pleasantries no matter what i ask from this message onwards.


)"; // Using Raw String Literal (R"()") for easier multiline strings with {} placeholder
String ultrasonic_prompt_ = R"(
    You have received an ultrasonic sensor reading. Explain the distance in simple terms.
    Relate the distance to common objects or scenarios if possible.
    Format sensor explanations clearly.
    Do NOT provide LED commands in response to a sensor reading.
    
    Sensor reading: {} cm.
    Explanation:
    )"; // Using Raw String Literal (R"()") for easier multiline strings with {} placeholder

// --- Arduino Pin Definitions ---
// Defined here in main.cpp
const int LED_PIN = 13; // Example LED pin (Built-in LED)


void setup() {
    // Initialize serial communication
    Serial.begin(9600);
    while (!Serial); // Wait for serial port to connect needed for native USB port only

    Serial.println("Gemini API Arduino Example - Interactive with LED Control");

    // Initialize LED pin
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW); // Ensure LED is off initially


    // Connect to WiFi (function defined in gemini.cpp)
    connectWiFi();

    Serial.println("\nArduino is ready. Type your prompt and press Enter to send to Gemini.");
    Serial.print(">"); // Prompt indicator
}


void loop() {
    // Check if data is available to read from the serial buffer
    while (Serial.available() > 0) {
        char incomingChar = Serial.read();

        // Echo the character back to the serial monitor so the user can see what they are typing
        Serial.print(incomingChar);

        // Check for newline or carriage return to signify end of input
        if (incomingChar == '\n' || incomingChar == '\r') {
            // Process the complete string when newline is received
            userInputString.trim(); // Trim whitespace

            if (userInputString.length() > 0) {
                // --- Gemini API Interaction ---

                // Format the context template with the user's input
                String fullPrompt = "";
                int placeholderPos = context_prompt_template.indexOf("{}");

                if (placeholderPos != -1) {
                    fullPrompt = context_prompt_template;
                    // Replace the first occurrence of {} with the user's prompt
                    fullPrompt.replace("{}", userInputString);
                } 
                else {
                    // Fallback if {} is not found (less ideal)
                    fullPrompt = context_prompt_template + "\nUser query: " + userInputString;
                }


                Serial.print("\n--- Sending Full Prompt to Gemini ---");
                Serial.print("Full Prompt: ");
                Serial.println(userInputString); // Print the full prompt being sent

                // Call Gemini API with the full, formatted prompt (function in gemini.cpp)
                String geminiResponse = callGeminiAPI(fullPrompt);

                Serial.println("\n--- Gemini Response ---");
                Serial.println(geminiResponse);
                Serial.println("-----------------------");

                // --- Parse Gemini's response and control the LED ---
                // Check if the response exactly matches the command string after lowercasing
                LED_Control(geminiResponse);


            } else {
                // Handle empty input (just pressing Enter)
                //Serial.println("\nEmpty input received."); // Keep commented for cleaner output
                //Serial.println("\nReady for next prompt:"); // Keep commented for cleaner output
                //Serial.print(">"); // Keep commented for cleaner output
            }

            // Clear the input string for the next command
            Serial.println("\nReady for next prompt:");
            Serial.print(">"); // Prompt indicator
            userInputString = "";
            

        } else {
            // Append the character to the input string if not newline/carriage return
            userInputString += incomingChar;
        }
    }

    // Add a small delay (optional, but good practice)
    delay(1);
}


void LED_Control(String response) {
    // Convert the Gemini response to lowercase for case-insensitive matching

    // Check if the response exactly matches the command string after lowercasing
    if (response.equals("LED_ON\n")) {
      Serial.println("Interpreted Gemini response as 'LED_ON'. Turning LED ON.");
      digitalWrite(LED_PIN, HIGH);
    } else if (response.equals("LED_OFF\n")) {
      Serial.println("Interpreted Gemini response as 'LED_OFF'. Turning LED OFF.");
      digitalWrite(LED_PIN, LOW);
    } else if (response.equals("STATUS\n")) {
      Serial.println("Interpreted Gemini response as 'STATUS'. Reading LED status.");
      int ledState = digitalRead(LED_PIN);
      if (ledState == HIGH) {
        Serial.println("LED status: ON");
      } else {
        Serial.println("LED status: OFF");
      }
    } else if (response.equals("QUIT\n")) {
       Serial.println("Interpreted Gemini response as 'QUIT'. Exiting loop.");
       // In an Arduino loop, you can't truly "exit".
       // You could add a flag here to stop processing further inputs
       // or put the main loop logic into a separate function that can be exited.
       // For now, we'll just print the message.
    }
    else {
      Serial.println("Gemini response did not contain a recognized command (LED_ON, LED_OFF, STATUS, QUIT).");
    }
}