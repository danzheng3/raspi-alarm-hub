Our smart alarm clock is a comprehensive, C++ based smart alarm system designed for the Raspberry Pi. It integrates a custom touchscreen UI, hardware-accelerated audio processing, peripheral device control (lights, pillboxes), and a unique wireless docking speaker system.

The project uses a modular architecture powered by an internal Event Bus to coordinate between the user interface, hardware sensors, and cloud services.
________________________________________________________________________
Key Features
1. Hybrid Audio & Docking System
- The core feature of this hub is its ability to seamlessly switch between a docked (wired) state and a wireless (Bluetooth) state.
- Dock Detection: Uses a physical switch (GPIO 27) to detect when the portable speaker unit is docked to the base station.
- Auto-Switching: When docked, audio routes via the 3.5mm Jack (alsa_output).
- Undocked: The system automatically connects to a paired Bluetooth speaker (Target ID: 44:1D:64:AE:52:E2) and routes audio wirelessly.
- BLE Alerts: Sends GATT messages over Bluetooth Low Energy to the speaker to trigger specific alert tones or lighting on the speaker itself during alarms.
- 7-Band Hardware Equalizer: Controls physical digital potentiometers (MCP44X1) to adjust audio frequency bands (62Hz - 16kHz).

2. Advanced Alarm Management
- Multi-Modal Alerts: Triggers multiple actions simultaneously:
- Audio: Plays MP3/WAV files.
- Strobe: Activates a high-intensity strobe light (GPIO 25).
- LED Indicators: Light indicators for specific days of the week (GPIO 22, 23, 24).
- Pillbox Release: Actuates a servo motor (GPIO 18 PWM) to open a medicine compartment.
- Persistence: Alarms and settings are saved to config.json and persist across reboots.

3. Touchscreen UI (SDL2)
- A custom graphical user interface built with SDL2 (Simple DirectMedia Layer).
- Main Dashboard: Displays time, live weather, and status icons.
- Music Player: Browse SD card library, play/pause, and adjust EQ.
- Settings: Wi-Fi scanning/connection and Bluetooth status.
- Gestures: Supports touch inputs and long-press actions for time setting.

4. Power & Environment
- Auto-Brightness: Reads ambient light via an LDR sensor (MCP3021 ADC) and adjusts the screen backlight automatically.
- Sleep Mode: Dims the screen after inactivity; wakes on touch or alarm.
- Weather: Fetches live local weather data (Temp, Condition, Wind) via the Open-Meteo API.
- Timekeeping: Syncs system time via NTP (WiFi) and maintains backup time via a hardware RTC (MCP7940N) when offline.

____________________________________________________________________
Software Architecture

The project is organized into Managers that communicate via a Publish/Subscribe EventBus.
1. main.cpp: Entry point. Initializes hardware and starts the main loop.
2. EventBus: Decouples logic. (for example: alarmManager publishes AlarmTriggeredEvent, and audioManager subscribes, playing the alarm audio)

Managers:
- connectivityManager: Handles Wi-Fi (nmcli) and Bluetooth (bluez/gatttool).
- audioManager: Wraps mpg123 and PulseAudio for playback.
- powerManager: Handles backlight PWM and idle timers.
- weatherManager: Handles cURL requests and JSON parsing.

___________________________________________________________________
Installation & Setup

The system runs on Raspberry Pi OS (Linux). Ensure the dependencies in setup.sh are installed. Use ./rerun.sh in order to get a clean build and run.
