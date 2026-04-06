/*
 * Master Test Code for Power Pack Emulator (Arduino Uno R4 Minima)
 * Name: unico_master_v01.ino
 * Purpose: Verify the operation of the RS232 emulator.
 * 
 * Hardware:
 * - Arduino Uno R4 Minima
 * - Serial: USB CLI/Debug (115200 baud)
 * - Serial1: RS232 Communication (Pins 0/1, 4800 baud, 8N1)
 * 
 * Logic:
 * - Sends commands defined in the "slave_design.pdf" protocol.
 * - Parses and displays responses from the emulator.
 * - Includes an automated test suite.
 */

#include <Arduino.h>

// --- Configuration ---
const long RS232_BAUD = 4800;
const long CLI_BAUD = 115200;
const int RESPONSE_TIMEOUT_MS = 500; // Protocol says < 400ms character timing, so 500ms is a safe limit

// --- Global Variables ---
String responseBuffer = "";

void setup() {
  // Debug/CLI Port
  Serial.begin(CLI_BAUD);
  while (!Serial); // Wait for Serial Monitor
  
  // RS232 Communication Port
  Serial1.begin(RS232_BAUD, SERIAL_8N1);
  
  displayMenu();
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    handleMenu(c);
  }
}

// --- Menu & CLI ---
void displayMenu() {
  Serial.println("\n===== UNICO MASTER V0.1 - TESTER =====");
  Serial.println("1. [Ping]      ADDS 0 (Select Device 0)");
  Serial.println("2. [Identity]  *IDN? Query");
  Serial.println("3. [Power ON]  Set Power ON (POWER 1)");
  Serial.println("4. [Power OFF] Set Power OFF (POWER 0)");
  Serial.println("5. [Voltage]   Set Voltage to 15.0V (SV 15.0)");
  Serial.println("6. [Readback]  Query Output Voltage (RV?)");
  Serial.println("7. [Current]   Query Output Current (RI?)");
  Serial.println("8. [Status]    Query Device Status (STUS 0)");
  Serial.println("A. [Automated] Run Full Test Sequence");
  Serial.println("M. [Menu]      Display this menu");
  Serial.println("========================================");
  Serial.print("> ");
}

void handleMenu(char choice) {
  switch (toupper(choice)) {
    case '1': testAddress(0); break;
    case '2': sendCommand("*IDN?"); break;
    case '3': sendCommand("POWER 1"); break;
    case '4': sendCommand("POWER 0"); break;
    case '5': sendCommand("SV 15.0"); break;
    case '6': sendCommand("RV?"); break;
    case '7': sendCommand("RI?"); break;
    case '8': sendCommand("STUS 0"); break;
    case 'A': runFullTest(); break;
    case 'M': displayMenu(); break;
    case '\r':
    case '\n': break; // Ignore
    default:
      Serial.print("Unknown Choice: ");
      Serial.println(choice);
      break;
  }
}

// --- Communication Core ---

// Sends a command and waits for a complete response line (ending in LF)
String sendCommand(String cmd) {
  // Clear buffers
  while (Serial1.available()) Serial1.read();
  responseBuffer = "";
  
  Serial.print("[TX]: ");
  Serial.println(cmd);
  
  // Send with CRLF termination
  Serial1.print(cmd);
  Serial1.print("\r\n");
  
  unsigned long startTime = millis();
  bool complete = false;
  
  while (millis() - startTime < RESPONSE_TIMEOUT_MS) {
    if (Serial1.available()) {
      char c = Serial1.read();
      if (c == '\n') {
        complete = true;
        break;
      }
      if (c != '\r') responseBuffer += c;
    }
  }
  
  if (complete) {
    responseBuffer.trim();
    Serial.print("[RX]: ");
    Serial.println(responseBuffer);
    
    // Validate response type
    if (responseBuffer == "=>") Serial.println("   (STATUS: SUCCESS)");
    else if (responseBuffer == "?>") Serial.println("   (STATUS: CMD ERROR)");
    else if (responseBuffer == "!>") Serial.println("   (STATUS: EXEC ERROR)");
    
    return responseBuffer;
  } else {
    Serial.println("[RX]: (TIMEOUT - NO RESPONSE)");
    return "TIMEOUT";
  }
}

// --- Specific Tests ---

void testAddress(int addr) {
  String cmd = "ADDS " + String(addr);
  String res = sendCommand(cmd);
  if (res == "=>") {
    Serial.println("SUCCESS: Device " + String(addr) + " selected.");
  } else {
    Serial.println("FAILURE: Could not select device.");
  }
}

void runFullTest() {
  String rv;
  float vRead;
  Serial.println("\n--- STARTING COMPREHENSIVE AUTOMATED TEST SEQUENCE ---");
  
  Serial.println("\nStep 1: Addressing Device 0");
  if (sendCommand("ADDS 0") != "=>") goto failed;
  
  Serial.println("\nStep 2: Identification & Info Queries");
  if (sendCommand("*IDN?").indexOf("EMULATOR") == -1) {
    Serial.println("FAILURE: *IDN? mismatch.");
    goto failed;
  }
  if (sendCommand("DEVI?").indexOf("DEVICE") == -1) {
    Serial.println("FAILURE: DEVI? mismatch.");
    goto failed;
  }
  if (sendCommand("RATE?").indexOf("V") == -1) {
    Serial.println("FAILURE: RATE? mismatch.");
    goto failed;
  }
  if (sendCommand("INFO 0").indexOf("PWR") == -1) {
    Serial.println("FAILURE: INFO 0 mismatch.");
    goto failed;
  }
  
  Serial.println("\nStep 3: Power Control (Local & Global)");
  sendCommand("POWER 1");
  delay(50);
  if (sendCommand("POWER 2") != "1") {
    Serial.println("FAILURE: POWER 1 failed.");
    goto failed;
  }
  
  sendCommand("POWER 0");
  delay(50);
  if (sendCommand("POWER 2") != "0") {
    Serial.println("FAILURE: POWER 0 failed.");
    goto failed;
  }

  sendCommand("GRPWR 1");
  delay(50);
  if (sendCommand("POWER 2") != "1") {
    Serial.println("FAILURE: GRPWR 1 failed.");
    goto failed;
  }

  sendCommand("GLOB 0"); // Global Power OFF
  delay(50);
  if (sendCommand("POWER 2") != "0") {
    Serial.println("FAILURE: GLOB 0 failed.");
    goto failed;
  }

  Serial.println("\nStep 4: Voltage & Current Setting/Query");
  sendCommand("SV 15.50");
  if (sendCommand("SV?") != "15.50") {
    Serial.println("FAILURE: SV/SV? mismatch.");
    goto failed;
  }

  sendCommand("SI 5.25");
  if (sendCommand("SI?") != "5.25") {
    Serial.println("FAILURE: SI/SI? mismatch.");
    goto failed;
  }

  Serial.println("\nStep 5: Global Setting (GSV/GSI)");
  sendCommand("GSV 20.00");
  if (sendCommand("SV?") != "20.00") {
    Serial.println("FAILURE: GSV failed.");
    goto failed;
  }

  sendCommand("GSI 10.00");
  if (sendCommand("SI?") != "10.00") {
    Serial.println("FAILURE: GSI failed.");
    goto failed;
  }

  Serial.println("\nStep 6: Readback Queries (RV/RI/RT)");
  sendCommand("POWER 1");
  delay(100);
  rv = sendCommand("RV?");
  vRead = rv.toFloat();
  if (vRead < 19.0 || vRead > 21.0) {
    Serial.println("FAILURE: RV? out of range.");
    goto failed;
  }
  if (sendCommand("RI?").toFloat() < 9.0) {
    Serial.println("FAILURE: RI? out of range.");
    goto failed;
  }
  if (sendCommand("RT?").toFloat() < 10.0) {
    Serial.println("FAILURE: RT? out of range.");
    goto failed;
  }

  Serial.println("\nStep 7: Remote & Status");
  sendCommand("REMS 1");
  if (sendCommand("REMS 2") != "1") {
    Serial.println("FAILURE: REMS 1 failed.");
    goto failed;
  }
  sendCommand("REMS 0");
  if (sendCommand("REMS 2") != "0") {
    Serial.println("FAILURE: REMS 0 failed.");
    goto failed;
  }
  if (sendCommand("STUS 0").length() == 0) {
    Serial.println("FAILURE: STUS query failed.");
    goto failed;
  }

  Serial.println("\nStep 8: Error Handling");
  if (sendCommand("INVALID_CMD") != "?>") {
    Serial.println("FAILURE: No error code for invalid command.");
    goto failed;
  }

  Serial.println("\n--- ALL COMPREHENSIVE TESTS PASSED! ---");
  return;

failed:
  Serial.println("\n--- COMPREHENSIVE TEST FAILED! ---");
}
