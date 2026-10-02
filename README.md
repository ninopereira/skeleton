# Haunted Skeleton

Arduino sketch for an animatronic Halloween skeleton built on a
[Bot'n Roll ONE A+](https://www.botnroll.com) (Arduino Uno compatible).

- Loops spooky background music from an SD card on power-up.
- When a visitor walks up, the music pauses for 5 s and the jaw "says"
  *"Who are you? What are you doing here?"* while both arms move randomly.
- The jaw then rests for at least 5 s before it can be triggered again.

## Hardware

| Part | Connection |
|---|---|
| Bot'n Roll ONE A+ | SPI slave select on pin 2 |
| HC-SR04 sonar | trig pin 6, echo pin 7 |
| Catalex Serial MP3 Player v1.0 | hardware Serial (pins 0/1) |
| Jaw servo | SER1 (pin 3) |
| Arm motors with encoders | left / right motor outputs |
| Buzzer | on-board, pin 9 |

All pins, angles, distances and timings live in [`config.hpp`](config.hpp).

## Setup

1. Install the **BnrOneAPlus** (1.1.2) and **Servo** libraries in the
   Arduino IDE.
2. Copy `haunted.mp3` as the first (ideally only) file on a FAT32 SD card.
3. Start with both arms resting down; that position is encoder zero.
4. Disconnect the MP3 module's TX wire (board pin 0) while uploading.
5. Calibrate `MOUTH_CLOSED_DEG` / `MOUTH_OPEN_DEG` and the arm speeds in
   `config.hpp` for your build.

## Code layout

| File | Purpose |
|---|---|
| `skeleton.ino` | Entry point |
| `haunted_skeleton.*` | Behaviour state machine |
| `mp3_player.*` | Catalex MP3 serial protocol |
| `sonar.*` | HC-SR04 with debounced presence detection |
| `mouth.*` | Scripted jaw animation |
| `arm.*`, `arms.*` | Random arm motion with PID speed control |
| `buzzer.*` | Boot beeps |
