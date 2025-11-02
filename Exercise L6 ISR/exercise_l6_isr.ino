/**
 * ESP32 Timer + Semaphore Demo (Arduino-ESP32 v3.x)
 *
 * Read ADC values at 1 Hz using esp_timer and defer printing a rolling
 * average of the last five readings in a FreeRTOS task.
 *
 * Notes:
 * - Uses esp_timer (newer API) instead of legacy hw_timer_t functions.
 * - Timer callback runs in timer task context (not hard ISR), so we use
 *   xSemaphoreGive (not FromISR).
 * - Rolling average maintained in the printing task.
 *
 * Date: Updated for Arduino-ESP32 v3.x
 * Author: Shawn Hymel
 * UPDATED BY: Md Sajib Pramanic
 * License: 0BSD
 */

#include <esp_timer.h>
//#include <semphr.h> // Might be needed on vanilla FreeRTOS ports

// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Settings
static const uint64_t period_us = 1000000ULL; // 1 Hz

// Pins
static const int adc_pin = A0; // Adjust as needed for your board

// Globals
static volatile uint16_t val = 0;          // Latest ADC reading
static SemaphoreHandle_t bin_sem = nullptr;
static esp_timer_handle_t periodic_timer = nullptr;

//*****************************************************************************
// Timer callback (runs in esp_timer task context, not hard ISR)

static void onTimer(void *arg) {
  // Read ADC and signal the task that a new sample is ready
  val = analogRead(adc_pin);
  // Not ISR context here, so use xSemaphoreGive
  if (bin_sem) {
    xSemaphoreGive(bin_sem);
  }
}

//*****************************************************************************
// Task: Wait for semaphore and print rolling average of last five readings

void printValues(void *parameters) {
  const int N = 5;
  uint16_t window[N] = {0};
  uint32_t sum = 0;
  int idx = 0;
  int count = 0;

  for (;;) {
    // Block until a new sample is available
    xSemaphoreTake(bin_sem, portMAX_DELAY);

    uint16_t sample = val;

    if (count < N) {
      sum += sample;
      window[idx] = sample;
      idx = (idx + 1) % N;
      count++;
    } else {
      sum -= window[idx];
      sum += sample;
      window[idx] = sample;
      idx = (idx + 1) % N;
    }

    int divisor = (count < N) ? count : N;
    uint16_t avg = (divisor > 0) ? (sum / divisor) : 0;

    Serial.println(avg);
  }
}

//*****************************************************************************
// Main (runs as its own task with priority 1 on core 1)

void setup() {
  // Configure Serial
  Serial.begin(115200);
  vTaskDelay(pdMS_TO_TICKS(1000)); // Wait so we don't miss Serial output
  Serial.println();
  Serial.println("--- ESP32 Timer + Semaphore Demo (v3.x) ---");
  Serial.println("Printing rolling average of last 5 ADC readings...");

  // Create semaphore
  bin_sem = xSemaphoreCreateBinary();
  if (bin_sem == nullptr) {
    Serial.println("Could not create semaphore");
    ESP.restart();
  }

  // Start task to print out rolling average (higher priority than setup task)
  xTaskCreatePinnedToCore(
      printValues,
      "Print values",
      2048,
      nullptr,
      2,
      nullptr,
      app_cpu);

  // Create and start periodic esp_timer (1 Hz)
  esp_timer_create_args_t targs = {};
  targs.callback = &onTimer;
  targs.arg = nullptr;
  targs.dispatch_method = ESP_TIMER_TASK; // Task context (safer for Arduino APIs)
  targs.name = "adc_1hz";

  if (esp_timer_create(&targs, &periodic_timer) != ESP_OK || periodic_timer == nullptr) {
    Serial.println("Failed to create esp_timer");
    ESP.restart();
  }

  if (esp_timer_start_periodic(periodic_timer, period_us) != ESP_OK) {
    Serial.println("Failed to start esp_timer");
    ESP.restart();
  }
}

void loop() {
  // Idle
  vTaskDelay(pdMS_TO_TICKS(1000));
}