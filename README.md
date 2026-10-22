# Haunted Skeleton

Arduino sketch for an animatronic Halloween skeleton built on an
Arduino Uno.

- Plays the songs on the SD card in order on power-up, looping back to
  the first song after the last one.
- When a visitor walks up, a voice clip asks *"Who are you? What are you
  doing here?"* with the jaw synced to it, and each arm goes up and back
  down, one slightly after the other. The music then continues with the
  song after the interrupted one.
- The jaw then rests for at least 5 s before it can be triggered again.

## Hardware

| Part | Connection |
|---|---|
| HC-SR04 sonar | trig pin 7, echo pin 6 |
| DFPlayer Mini | **VCC on 3V3, not 5V**; module TX → pin 3; pin 2 → 680 Ω → module RX, 1 kΩ from module RX to GND |
| Jaw servo | pin 9 |
| Arm servos | left: pin 10, right: pin 11 |
| Eye LEDs | pin 8 |

All pins, angles, distances and timings live in [`config.hpp`](config.hpp).

![Wiring diagram](docs/wiring.svg)

## Setup

1. Install the **Servo** and **DFRobotDFPlayerMini** (1.0.6) libraries
   in the Arduino IDE.
2. Format a FAT32 SD card and copy `voice.mp3` onto it first, then the
   songs one at a time in the order they should play (the module numbers
   tracks by copy order). Set `MP3_SONG_COUNT` in `config.hpp` to the
   number of songs.
3. Power the servos well: three servos (two lifting arms) can draw more
   current than the board's 5 V regulator supplies.
4. If the eyes blink 5 times at power-up, the MP3 player or its SD card
   did not answer: check the wiring and the card. `tools/mp3_test` tests
   the player on its own.
5. Calibrate `MOUTH_CLOSED_DEG` / `MOUTH_OPEN_DEG` and the arm angles and
   sweep times in `config.hpp` for your build.

## Known issues

- **Servo twitches when a track starts or ends.** SoftwareSerial pauses
  interrupts while each byte to or from the DFPlayer is sent or received,
  and the servos depend on those interrupts. If the twitch is a problem,
  move the DFPlayer to hardware Serial (pins 0/1) and pass `Serial` to
  `Mp3Player` instead of a SoftwareSerial port.

## Code layout

| File | Purpose |
|---|---|
| `skeleton.ino` | Entry point |
| `haunted_skeleton.*` | Behaviour state machine |
| `mp3_player.*` | DFPlayer Mini wrapper |
| `sonar.*` | HC-SR04 with debounced presence detection |
| `mouth.*` | Scripted jaw animation |
| `arm.*`, `arms.*` | Staggered up-and-down servo arm gesture |
| `eyes.*` | Eye LEDs |
