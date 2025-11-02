/**
Md Sajib Pramanic
  * FreeRTOS Deadlock Demo - Resolved Variants
 */


#include <Arduino.h>
#include <FreeRTOS.h>
#include <semphr.h>

// Select mode: uncomment one
//#define MODE_TIMEOUT
//#define MODE_HIERARCHY
#define MODE_ARBITRATOR


// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Globals
static SemaphoreHandle_t mutex_1;
static SemaphoreHandle_t mutex_2;

#ifdef MODE_ARBITRATOR
static SemaphoreHandle_t arbitrator; // global arbitrator mutex
#endif

//*****************************************************************************
// Tasks

void doTaskA(void *parameters) {

  // For timeout mode
  const TickType_t mutex_timeout = pdMS_TO_TICKS(100);

  while (1) {

#ifdef MODE_TIMEOUT
    // Try to take mutex_1 with timeout
    if (xSemaphoreTake(mutex_1, mutex_timeout) == pdTRUE) {
      Serial.println("Task A took mutex 1");

      // small delay to increase chance of contention
      vTaskDelay(1 / portTICK_PERIOD_MS);

      // try to take mutex_2 with timeout
      if (xSemaphoreTake(mutex_2, mutex_timeout) == pdTRUE) {
        Serial.println("Task A took mutex 2");

        // critical section
        Serial.println("Task A doing some work");
        vTaskDelay(500 / portTICK_PERIOD_MS);

        xSemaphoreGive(mutex_2);
        xSemaphoreGive(mutex_1);
      } else {
        // couldn't get mutex_2 -> give mutex_1 and back off
        Serial.println("Task A timed out waiting for mutex 2, releasing mutex 1 and backing off");
        xSemaphoreGive(mutex_1);
        vTaskDelay(50 / portTICK_PERIOD_MS); // backoff
      }
    } else {
      // couldn't acquire mutex_1
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }

#elif defined(MODE_HIERARCHY)
    // Take mutexes in the same global order: mutex_1 then mutex_2
    xSemaphoreTake(mutex_1, portMAX_DELAY);
    Serial.println("Task A took mutex 1");
    vTaskDelay(1 / portTICK_PERIOD_MS);
    xSemaphoreTake(mutex_2, portMAX_DELAY);
    Serial.println("Task A took mutex 2");

    // critical section
    Serial.println("Task A doing some work");
    vTaskDelay(500 / portTICK_PERIOD_MS);

    xSemaphoreGive(mutex_2);
    xSemaphoreGive(mutex_1);

#elif defined(MODE_ARBITRATOR)
    // Acquire arbitrator, then take both resource mutexes, then release arbitrator
    xSemaphoreTake(arbitrator, portMAX_DELAY);
    Serial.println("Task A took arbitrator");

    xSemaphoreTake(mutex_1, portMAX_DELAY);
    Serial.println("Task A took mutex 1");
    vTaskDelay(1 / portTICK_PERIOD_MS);
    xSemaphoreTake(mutex_2, portMAX_DELAY);
    Serial.println("Task A took mutex 2");

    // Done acquiring — release arbitrator so others can attempt acquisition later
    xSemaphoreGive(arbitrator);
    Serial.println("Task A released arbitrator");

    // critical section
    Serial.println("Task A doing some work");
    vTaskDelay(500 / portTICK_PERIOD_MS);

    xSemaphoreGive(mutex_2);
    xSemaphoreGive(mutex_1);
#endif

    Serial.println("Task A going to sleep");
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void doTaskB(void *parameters) {

  const TickType_t mutex_timeout = pdMS_TO_TICKS(100);

  while (1) {

#ifdef MODE_TIMEOUT
    // Task B attempts opposite order, but will back off on timeout to avoid deadlock.
    if (xSemaphoreTake(mutex_2, mutex_timeout) == pdTRUE) {
      Serial.println("Task B took mutex 2");

      vTaskDelay(1 / portTICK_PERIOD_MS);

      if (xSemaphoreTake(mutex_1, mutex_timeout) == pdTRUE) {
        Serial.println("Task B took mutex 1");

        // critical section
        Serial.println("Task B doing some work");
        vTaskDelay(500 / portTICK_PERIOD_MS);

        xSemaphoreGive(mutex_1);
        xSemaphoreGive(mutex_2);
      } else {
        Serial.println("Task B timed out waiting for mutex 1, releasing mutex 2 and backing off");
        xSemaphoreGive(mutex_2);
        vTaskDelay(50 / portTICK_PERIOD_MS);
      }
    } else {
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }

#elif defined(MODE_HIERARCHY)
    // IMPORTANT: take in same order as Task A (hierarchy) to avoid circular wait
    xSemaphoreTake(mutex_1, portMAX_DELAY);
    Serial.println("Task B took mutex 1");
    vTaskDelay(1 / portTICK_PERIOD_MS);
    xSemaphoreTake(mutex_2, portMAX_DELAY);
    Serial.println("Task B took mutex 2");

    // critical section
    Serial.println("Task B doing some work");
    vTaskDelay(500 / portTICK_PERIOD_MS);

    xSemaphoreGive(mutex_2);
    xSemaphoreGive(mutex_1);

#elif defined(MODE_ARBITRATOR)
    // Use same arbitrator approach
    xSemaphoreTake(arbitrator, portMAX_DELAY);
    Serial.println("Task B took arbitrator");

    xSemaphoreTake(mutex_1, portMAX_DELAY);
    Serial.println("Task B took mutex 1");
    vTaskDelay(1 / portTICK_PERIOD_MS);
    xSemaphoreTake(mutex_2, portMAX_DELAY);
    Serial.println("Task B took mutex 2");

    xSemaphoreGive(arbitrator);
    Serial.println("Task B released arbitrator");

    // critical section
    Serial.println("Task B doing some work");
    vTaskDelay(500 / portTICK_PERIOD_MS);

    xSemaphoreGive(mutex_2);
    xSemaphoreGive(mutex_1);
#endif

    Serial.println("Task B going to sleep");
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

//*****************************************************************************
// Main (runs as its own task with priority 1 on core 1)

void setup() {

  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FreeRTOS Deadlock Demo (resolved variants)---");

  // Create mutexes before starting tasks
  mutex_1 = xSemaphoreCreateMutex();
  mutex_2 = xSemaphoreCreateMutex();
#ifdef MODE_ARBITRATOR
  arbitrator = xSemaphoreCreateMutex();
#endif

  // Start Task A (higher priority)
  xTaskCreatePinnedToCore(doTaskA,
                          "Task A",
                          4096,
                          NULL,
                          2,
                          NULL,
                          app_cpu);

  // Start Task B (lower priority)
  xTaskCreatePinnedToCore(doTaskB,
                          "Task B",
                          4096,
                          NULL,
                          1,
                          NULL,
                          app_cpu);

  // Delete "setup and loop" task
  vTaskDelete(NULL);
}

void loop() {
  // should never reach here
}
