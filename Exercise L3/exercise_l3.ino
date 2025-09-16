#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

// ===== Onboard RGB LED (ESP32-S3-DevKitC-1 v1.1) =====
#define LED_PIN    38
#define NUM_LEDS   1
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Optional: task handles for future diagnostics
TaskHandle_t hLED = nullptr, hSER = nullptr, hCPU = nullptr;

// ---------- Task: LED (highest priority) ----------
void TaskLED(void *pv) {
  TickType_t last = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(400);   // 400 ms period

  uint32_t colors[] = {
    Adafruit_NeoPixel::Color(255, 0, 0),  // Red
    Adafruit_NeoPixel::Color(0, 255, 0),  // Green
    Adafruit_NeoPixel::Color(0, 0, 255),  // Blue
    0                                      // Off
  };
  int i = 0;

  for (;;) {
    // LED ON for 150 ms
    strip.setPixelColor(0, colors[i]);
    strip.show();
    vTaskDelay(pdMS_TO_TICKS(200));

    // LED OFF until the next period boundary (fixed-rate schedule)
    strip.setPixelColor(0, 0);
    strip.show();
    vTaskDelayUntil(&last, period);

    i = (i + 1) % 4; // R -> G -> B -> Off
  }
}

// ---------- Task: Serial (medium priority) ----------
void TaskSerial(void *pv) {
  TickType_t last = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(1000);  // 1 s

  for (;;) {
    size_t freeHeap = xPortGetFreeHeapSize();
    Serial.printf("[t=%lu ms] freeHeap=%u bytes\n",
                  (unsigned long)millis(),
                  (unsigned int)freeHeap);
    vTaskDelayUntil(&last, period);
  }
}

// ---------- Task: Compute (lowest priority) ----------
void TaskCompute(void *pv) {
  TickType_t last = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(200);   // 200 ms
  volatile uint32_t sink = 0;

  for (;;) {
    // Light CPU load: simple arithmetic loop
    for (int k = 0; k < 200000; ++k) {
      sink += (k ^ ((k << 1) + (k >> 1)));
    }
    vTaskDelayUntil(&last, period);
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\nESP32-S3 FreeRTOS demo (LED + Serial + Compute)");

  // Init onboard RGB LED
  strip.begin();
  strip.setBrightness(120);  // 0..255
  strip.clear();
  strip.show();

  // Create tasks (bigger number = higher priority)
  xTaskCreate(TaskLED,    "TaskLED",    2048, NULL, 3, &hLED);
  xTaskCreate(TaskSerial, "TaskSerial", 3072, NULL, 1, &hSER);
  xTaskCreate(TaskCompute,"TaskCompute",2048, NULL, 2, &hCPU);
}

void loop() {
  // Not used: FreeRTOS scheduler runs our tasks
}