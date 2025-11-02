## 🧩 Author

**Md Sajib Pramanic**

Project for FreeRTOS learning on ESP32

---
![image](/Exercise%20L6%20ISR/Screenshot%20from%202025-11-02%2023-58-33.png)

# ESP32 Timer + Semaphore Rolling Average (Arduino-ESP32 v3.x)

Reads an analog input at 1 Hz using `esp_timer`, signals a FreeRTOS task with a binary semaphore, and prints the rolling average of the last five readings.

## Features
- Uses `esp_timer` (Arduino-ESP32 3.x API) for periodic callbacks
- Timer callback reads ADC; task prints a 5-sample rolling average
- FreeRTOS binary semaphore for safe task signaling

## Requirements
- Arduino IDE 2.x (or PlatformIO)
- ESP32 Arduino Core 3.x or newer
- An ESP32 development board
- Analog signal connected to the configured ADC pin (default: `A0`)

## 🧭 How to Use

### Clone the Project

```bash
git clone https://github.com/SAJIB3489/real-time-system-RTOS/
cd Exercise L6 ISR
```

## Getting Started
1. Open `exercise_l6_isr.ino` in Arduino IDE.
2. Select your ESP32 board and serial port.
3. Upload and open Serial Monitor at 115200 baud.
4. Observe the rolling average printed once per second.

## Configuration
- ADC pin: change `adc_pin` (default `A0`) to your board’s analog-capable pin.
- Sample rate: adjust `period_us` (default `1,000,000` for 1 Hz).
- Task pinning: uses `app_cpu` (core 1 on dual-core ESP32; core 0 on UNICORE).
