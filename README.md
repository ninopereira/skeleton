# Haunted Skeleton

Arduino sketch for an animatronic Halloween skeleton built on a
[Bot'n Roll ONE A+](https://www.botnroll.com) (Arduino Uno compatible).

- Loops spooky background music from an SD card on power-up.
- When a visitor walks up, a voice clip asks *"Who are you? What are you
  doing here?"* with the jaw synced to it, and each arm goes up and back
  down, one slightly after the other. The music then restarts.
- The jaw then rests for at least 5 s before it can be triggered again.

## Hardware

| Part | Connection |
|---|---|
| Bot'n Roll ONE A+ | SPI slave select on pin 2 |
| HC-SR04 sonar | trig pin 7, echo pin 6 |
| Catalex Serial MP3 Player v1.0 | hardware Serial (pins 0/1) |
| Jaw servo | SER1 (pin 3) |
| Arm servos | left: SER2 (pin 5), right: pin 4 |
| Buzzer | on-board, pin 9 |
| Eye LEDs | pin 8 |

All pins, angles, distances and timings live in [`config.hpp`](config.hpp).

## Setup

1. Install the **BnrOneAPlus** (1.1.2) and **Servo** libraries in the
   Arduino IDE.
2. Copy `haunted.mp3` as the first (ideally only) file on a FAT32 SD card.
3. Power the servos well: three servos (two lifting arms) can draw more
   current than the board's 5 V regulator supplies.
4. Disconnect the MP3 module's TX wire (board pin 0) while uploading.
5. Calibrate `MOUTH_CLOSED_DEG` / `MOUTH_OPEN_DEG` and the arm angles and
   sweep times in `config.hpp` for your build.

## Code layout

| File | Purpose |
|---|---|
| `skeleton.ino` | Entry point |
| `haunted_skeleton.*` | Behaviour state machine |
| `mp3_player.*` | Catalex MP3 serial protocol |
| `sonar.*` | HC-SR04 with debounced presence detection |
| `mouth.*` | Scripted jaw animation |
| `arm.*`, `arms.*` | Staggered up-and-down servo arm gesture |
| `buzzer.*` | Boot beeps |
| `eyes.*` | Eye LEDs |
