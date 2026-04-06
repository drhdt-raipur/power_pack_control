# UNICO Master V0.1 - RS232 Tester & Controller

## Overview
The **UNICO Master V0.1** is a robust diagnostic and control tool developed for the **Arduino Uno R4 Minima**. It serves as a master controller (Tester) to validate the RS232 communication protocol and operational integrity of the Power Pack Emulator (PPE) or compatible 3kW Power Modules.

This tool is essential for verifying hardware integration, protocol compliance, and performing automated stress tests on power electronics modules using the RS232 interface.

---

## Key Features
- **Protocol Verification:** Ensures all commands (ADDS, POWER, SV, etc.) are correctly parsed and responded to.
- **Hardware Integration:** Validates the physical RS232 layer (4800 Baud, 8N1).
- **Interactive CLI Menu:** User-friendly interface via Serial Monitor at 115200 Baud.
- **Automated Full Test:** A 10-step sequence to verify edge cases, range limits, and error handling.
- **Safety Monitoring:** Real-time query of Voltage, Current, Temperature, and Device Status.

---

## Hardware Setup & Wiring

### Supported Hardware
- **Main Controller:** Arduino Uno R4 Minima.
- **Communication Interface:** Hardware Serial1 (Pins 0 and 1).
- **Level Shifting:** If connecting to a real Power Module with ±12V RS232 levels, an **RS232-to-TTL level shifter (e.g., MAX3232)** is **mandatory** to avoid damaging the Arduino (which operates at 5V TTL).

### Wiring Specifications
| Arduino Pin | Signal (RS232) | Connection Target (Module/Emulator) |
| :--- | :--- | :--- |
| **Pin 0 (RX1)** | RX | TX of Emulator/Power Module |
| **Pin 1 (TX1)** | TX | RX of Emulator/Power Module |
| **GND** | Ground | Ground of Emulator/Power Module |
| **USB-C** | Debug/CLI | PC Terminal (115200 Baud) |

### Connection Diagram
A detailed connection schematic is available:  
`docs/connection_schematic_RS232_test.png`

---

## Communication Settings & Values
The system follows a strict command-response architecture:

| Parameter | Value |
| :--- | :--- |
| **Baud Rate (RS232)** | 4800 bps |
| **Baud Rate (CLI)** | 115200 bps |
| **Data Format** | 8N1 (8 Data Bits, No Parity, 1 Stop Bit) |
| **Termination** | `\r\n` (CRLF) for all commands and responses |
| **Timeout Limit** | 500 ms (Response must be received within this window) |

### Protocol Response Flags
- `=>` : **SUCCESS** (Command accepted and executed)
- `?>` : **CMD ERROR** (Command not recognized or malformed)
- `!>` : **EXEC ERROR** (Command recognized but value out of range or execution failed)

---

## How to Use

### 1. Software Installation
1. Open the Arduino IDE.
2. Load the file `unico_master_v01.ino` from the `unico_master_v01` folder.
3. Select **Arduino Uno R4 Minima** as the board.
4. Upload the code to your Arduino.

### 2. Operation
1. Connect the hardware as per the wiring table.
2. Open the **Serial Monitor** in the Arduino IDE.
3. Set the baud rate to **115200**.
4. You will be presented with the following interactive menu:

```text
===== UNICO MASTER V0.1 - TESTER =====
1. [Ping]      ADDS 0 (Select Device 0)
2. [Identity]  *IDN? Query
3. [Power ON]  Set Power ON (POWER 1)
4. [Power OFF] Set Power OFF (POWER 0)
5. [Voltage]   Set Voltage to 15.0V (SV 15.0)
6. [Readback]  Query Output Voltage (RV?)
7. [Current]   Query Output Current (RI?)
8. [Status]    Query Device Status (STUS 0)
A. [Automated] Run Full Test Sequence
M. [Menu]      Display this menu
========================================
```

### 3. Automated Testing
By pressing **'A'**, the controller will run a 10-step automated sequence that validates:
- Device Addressing.
- Identity Parsing (expects "EMULATOR" keyword during testing).
- Power State toggling.
- Voltage/Current setpoint and readback accuracy.
- Error handling for invalid commands.

---

## Testing with Emulator
For development and validation, this Master Controller code was tested using an **RS232 Emulator** that mimics the behavior of the original 3kW Power Module. This ensures that the logic is sound before connecting to high-power hardware.

---

## Visual Demo
A video demonstration of the operation within the Arduino IDE is available in the repository:  
`docs/arduino_ide_demo.mkv`

---

## Documentation Reference
- **Source Code:** `unico_master_v01.ino`
- **Technical Detail (Markdown):** `unico_master_v01.md`
- **Comprehensive Master Document:** `MASTER_DOCUMENT.md`
