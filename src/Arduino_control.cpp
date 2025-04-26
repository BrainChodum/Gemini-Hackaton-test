#include <Arduino.h>

// Define an LED pin (example)
const int LED_PIN = 13; // Built-in LED on most Arduinos

void setup() {
  // Initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  // Wait for serial port to connect. Needed for native USB port only
  while (!Serial) {
    ;
  }

  // Initialize the LED pin as an output
  pinMode(LED_PIN, OUTPUT);

  Serial.println("Arduino is ready. Send commands:");
  Serial.println(" 'LED_ON' to turn LED on");
  Serial.println(" 'LED_OFF' to turn LED off");
  Serial.println(" 'STATUS' to get LED status");
}

void loop() {
  // Check if data is available to read from the serial port
  if (Serial.available() > 0) {
    // Read the incoming string until a newline character
    String command = Serial.readStringUntil('\n');
    command.trim(); // Remove leading/trailing whitespace

    Serial.print("Received command: ");
    Serial.println(command);

    // Process the received command
    if (command == "LED_ON") {
      digitalWrite(LED_PIN, HIGH); // Turn LED on
      Serial.println("LED turned ON");
    } else if (command == "LED_OFF") {
      digitalWrite(LED_PIN, LOW); // Turn LED off
      Serial.println("LED turned OFF");
    } else if (command == "STATUS") {
      int ledState = digitalRead(LED_PIN);
      if (ledState == HIGH) {
        Serial.println("LED status: ON");
      } else {
        Serial.println("LED status: OFF");
      }
    } else {
      // Handle unknown commands
      Serial.println("Unknown command");
    }
  }

  // Add a small delay to prevent flooding the serial port
  delay(10);
}