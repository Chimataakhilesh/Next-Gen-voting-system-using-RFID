# Next‑Gen Voting System using RFID (LPC2129 + EM-18)
<img width="249" height="119" alt="title" src="https://github.com/user-attachments/assets/3617dd08-264a-4d6c-975a-097856e21d9d" />


A simple embedded voting system that authenticates voters using an EM-18 RFID reader and an ARM7 LPC2129 microcontroller. Voting status is displayed on a 16×2 LCD. The system prevents duplicate voting and lets an admin view results.

---

## Key Features (at-a-glance)
- Uses LPC2129 (ARM7) microcontroller
- RFID reader: EM-18 (UART/TTL)
- 16×2 LCD for user prompts and results
- Prevents duplicate votes (tracked in RAM)
- Three-party voting support (DMK, ADMK, TVK)
- Show final results via a dedicated button/interrupt

---

## Components / Hardware
- LPC2129 (ARM7) development board
- EM-18 RFID reader (TTL UART output)
- 16×2 character LCD (HD44780-compatible)
- Push buttons (for selecting parties and showing results)
- Pull-up / pull-down resistors as required for buttons
- Power supply (as per your board & modules)
- Wires, breadboard / PCB

---

## Pin & Signal Mapping (as used in main.c)
- UART1 (EM-18) — configured at 9600 baud:
  - LPC2129 P0.8  = TXD1 (microcontroller TX)
  - LPC2129 P0.9  = RXD1 (microcontroller RX)  <-- connect EM-18 TX here
- External Interrupts (buttons):
  - EINT0 -> P0.1  (RESULTS button)
  - EINT1 -> P0.3  (Party 1 button) — selects DMK
  - EINT2 -> P0.7  (Party 2 button) — selects ADMK
  - EINT3 -> P0.20 (Party 3 button) — selects TVK
- LCD:
  - Connect per the lcd_header library used in this project (check `lcd_header.h` for exact pins). The project uses helper functions `LCD_INIT`, `LCD_COMMAND`, `LCD_DATA`, etc.

Notes:
- EM-18 Vcc -> 5V (or per module), GND -> common ground with LPC2129.
- EM-18 TX -> LPC2129 RXD1 (P0.9). Do NOT connect EM-18 RX (unless you need to send data to the reader).
- Buttons should be wired with debouncing (hardware or software) and proper pull-ups/pull-downs.

---

## How the system works (brief)
1. System starts in VERIFY_MODE and prompts "SCAN FOR VERIFY".
3. User scans an RFID tag (EM-18 sends ASCII tag over UART).
4. The code compares the tag against the `Voter_ID[]` array:
   - If match and not voted -> transitions to VOTE_MODE.
   - <img width="306" height="155" alt="verified" src="https://github.com/user-attachments/assets/e9076c61-e7e3-4d9b-b2ff-d0a999408b78" />

   - If match and already voted -> displays "ALREADY VOTED".
   - <img width="302" height="157" alt="already_voted" src="https://github.com/user-attachments/assets/61ec6274-8c3a-4452-b494-e1cc18c7563d" />

   - If no match -> displays "INVALID ID".
   - <img width="461" height="232" alt="Capture" src="https://github.com/user-attachments/assets/b7427e1f-bd7e-4268-a4cd-1166456ea483" />

5. In VOTE_MODE the LCD shows party options. User presses the corresponding button (EINT1/EINT2/EINT3) to select a party, then must re-scan the same tag to confirm the vote.
   <img width="306" height="154" alt="party_list" src="https://github.com/user-attachments/assets/b6438b51-9918-4f9d-9143-d24ad1b93ccb" />

7. On confirmation, vote is recorded (DMK/ADMK/TVK counters increment) and voter's entry in `voted[]` is set to block repeats.
   <img width="303" height="157" alt="vote_success" src="https://github.com/user-attachments/assets/6f908bf4-5b19-4703-a001-01b8ccd5694b" />

9. Pressing the RESULTS button (EINT0) displays winner or "DRAW".
    <img width="304" height="157" alt="results" src="https://github.com/user-attachments/assets/91f1d628-6441-45f9-86a0-680d91dd7e83" />


---

## Code specifics & important constants
- `Voter_ID[5][14]` — current list of authorized voter tag strings (edit to add/remove voters).
- `voted[5]` — parallel array that flags whether the voter has already voted.
- `read_rfid(char *id)` — reads 13 bytes from UART (then null terminates). EM-18 typically sends ASCII tag digits; verify the tag length your reader sends.
- UART is configured at 9600 baud, 8N1.
- External interrupts handle button presses; IRQ mapping is set in `EINT_config()`.

---

## Quick Start — Build & Flash
1. Install Keil uVision (or another ARM7-compatible toolchain) and set up an LPC2129 project.
2. Add the project source files (including `main.c` and `lcd_header.h`).
3. Compile/Build in Keil (target: LPC2129).
4. Flash the produced binary to the LPC2129 using:
   - Flash Magic, or
   - Keil with a suitable debug adapter (ULINK, JTAG, etc.), or
   - Your preferred programmer that supports LPC2xxx devices.
5. Power the board, ensure EM-18 and LCD are connected, then test with an RFID tag.

---

## How to add / change voters
- Open `main.c` and edit the `Voter_ID` array. Each entry is a null-terminated string representing a tag ID.
- Make sure `voted[]` array length matches `Voter_ID[]`.
- Recompile and flash.

Example:
```c
char Voter_ID[5][14] = {"TAG1", "TAG2", "TAG3", "TAG4", "TAG5"};
int voted[5] = {0,0,0,0,0};
```

---

## Common adjustments / tips
- Tag length mismatch: EM-18 modules differ in output framing. `read_rfid()` currently expects 13 characters — adjust the read loop or trim received data to match your EM-18 output (or configure the reader if possible).
- If tags arrive with leading/trailing bytes (CR/LF), trim them before comparison.
- Storing voters & votes in non-volatile memory: the current implementation uses RAM only, so a reset/power-cycle clears votes. For persistence, store voter status and counters in EEPROM/Flash.
- Debounce your hardware buttons or implement software debouncing to avoid multiple triggers.
- LCD pin mapping is controlled by the `lcd_header.h` library — adapt wiring to that header.

---

## Troubleshooting
- Nothing displayed on LCD: ensure LCD is powered, contrast adjusted, and `lcd_header.h` pin mapping matches wiring.
- Reader not detected: verify EM-18 TX -> LPC RX (common ground), and UART speed (9600).
- Wrong tag matching: print or log raw received bytes (via UART to PC or blinking LED) to inspect framing and length; then update `check_voter()` or `read_rfid()` accordingly.
- Duplicate votes still allowed: confirm `voted[current_voter_index] = 1;` executes on successful confirm and that `current_voter_index` is properly set by `check_voter()`.

---

## Safety & best practices
- Ensure common ground between EM-18 and LPC2129 board.
- Power modules within their rated voltages (EM-18 commonly 5V).
- If using 5V signals to a 3.3V MCU input, ensure voltage compatibility or use level shifting.
---

## Credits
- Project by Chimataakhilesh
- Uses EM-18 RFID module and LPC2129 microcontroller

##schematic capture
<img width="1224" height="865" alt="schematic_view" src="https://github.com/user-attachments/assets/6b2560e0-8bfe-4f3b-95ec-4497293c8075" />

