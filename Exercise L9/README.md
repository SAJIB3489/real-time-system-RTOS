## 🧩 Author

**Md Sajib Pramanic**

Project for FreeRTOS learning on ESP32

# ESP32 Multicore FreeRTOS Demo

I ran and observed a small FreeRTOS demo on an ESP32 (S3). The demo creates two tasks that "hog" the CPU and prints which core each task is running on. I describe how I pinned the tasks to specific cores, how I removed affinity, what I observed on the serial monitor, and why the behavior occurs. I also include suggested follow-up experiments.

---

## What this demo does

- Starts two tasks:
  - Task L (low priority)
  - Task H (high priority)
- Each task prints `Task L, Core N` or `Task H, Core N` using `xPortGetCoreID()`.
- Each task then performs a busy-wait ("hog") loop to simulate CPU work.
- The demo shows how task-to-core pinning and `tskNO_AFFINITY` affect where tasks actually run.

---

## Hardware / Software

- ESP32 (S3 or other dual-core ESP32)
- Arduino / ESP-IDF compatible environment (the sample code is Arduino-style)
- Serial Monitor at 115200 baud

---

## How I ran it

1. I uploaded the provided sketch to the ESP32.
2. I opened the Serial Monitor (115200).
3. I observed the serial output while trying three configurations:
   - Default (example in the repo uses `tskNO_AFFINITY` in the code shown below).
   - Explicit core pinning: pin Task L to core 0 and Task H to core 1.
   - No affinity: both tasks created with `tskNO_AFFINITY`.

---

## Code (important bits)

Here is the minimal relevant code. Use it as-is or copy into your sketch:

```cpp
// Core definitions
static const BaseType_t pro_cpu = 0;
static const BaseType_t app_cpu = 1;

// Settings
static const TickType_t time_hog = 200;  // ms

static void hog_delay(uint32_t ms) {
  for (uint32_t i = 0; i < ms; i++) {
    for (uint32_t j = 0; j < 40000; j++) {
      asm("nop");
    }
  }
}

void doTaskL(void *parameters) {
  char str[40];
  while (1) {
    sprintf(str, "Task L, Core %i\r\n", xPortGetCoreID());
    Serial.print(str);
    hog_delay(time_hog);
  }
}

void doTaskH(void *parameters) {
  char str[40];
  while (1) {
    sprintf(str, "Task H, Core %i\r\n", xPortGetCoreID());
    Serial.print(str);
    hog_delay(time_hog);
  }
}

void setup() {
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FreeRTOS Priority Inheritance Demo---");

  // Example: change the last argument to pin tasks or to tskNO_AFFINITY
  // Pin Task L to core 0, Task H to core 1:
  // xTaskCreatePinnedToCore(doTaskL, "Task L", 2048, NULL, 1, NULL, pro_cpu);
  // xTaskCreatePinnedToCore(doTaskH, "Task H", 2048, NULL, 2, NULL, app_cpu);

  // No affinity (allow scheduler to choose):
  // xTaskCreatePinnedToCore(doTaskL, "Task L", 2048, NULL, 1, NULL, tskNO_AFFINITY);
  // xTaskCreatePinnedToCore(doTaskH, "Task H", 2048, NULL, 2, NULL, tskNO_AFFINITY);

  // Current code used in my run (replace with the two options above to test):
  xTaskCreatePinnedToCore(doTaskL, "Task L", 2048, NULL, 1, NULL, tskNO_AFFINITY);
  xTaskCreatePinnedToCore(doTaskH, "Task H", 2048, NULL, 2, NULL, tskNO_AFFINITY);

  vTaskDelete(NULL);
}

void loop() {
  // should never run
}
```

---

## How to change the core pinning

- To pin Task L to core 0 and Task H to core 1:
  - Change the last argument of `xTaskCreatePinnedToCore()` to `pro_cpu` for Task L and `app_cpu` for Task H (or hardcode `0` and `1`).
- To remove affinity:
  - Replace the last argument with `tskNO_AFFINITY` for both tasks.

Example variants:
- Pinning
  ```cpp
  xTaskCreatePinnedToCore(doTaskL, "Task L", 2048, NULL, 1, NULL, pro_cpu); // core 0
  xTaskCreatePinnedToCore(doTaskH, "Task H", 2048, NULL, 2, NULL, app_cpu); // core 1
  ```
- No affinity
  ```cpp
  xTaskCreatePinnedToCore(doTaskL, "Task L", 2048, NULL, 1, NULL, tskNO_AFFINITY);
  xTaskCreatePinnedToCore(doTaskH, "Task H", 2048, NULL, 2, NULL, tskNO_AFFINITY);
  ```

---

## Observed serial outputs

I captured the serial output for two configurations. Below are the raw snippets I observed.

1) When I pinned tasks (Task L -> core 0, Task H -> core 1)
```
ESP-ROM:esp32s3-20210327
Build:Mar 27 2021
rst:0x1 (POWERON),boot:0x8 (SPI_FAST_FLASH_BOOT)
SPIWP:0xee
mode:DIO, clock div:1
load:0x3fce3808,len:0x370
load:0x403c9700,len:0x900
load:0x403cc700,len:0x2364
entry 0x403c98ac

---FreeRTOS Priority Inheritance Demo---
Task L, Core 0
Task H, Core 1
Task H, Core 1
Task L, Core 0
Task H, Core 1
Task L, Core 0
Task H, Core 1
Task L, Core 0
Task H, Core 1
Task L, Core 0
Task H, Core 1
```
![image](/Exercise%20L9/Core_0_1.png)


2) When I used `tskNO_AFFINITY` for both tasks
```
ESP-ROM:esp32s3-20210327
Build:Mar 27 2021
rst:0x1 (POWERON),boot:0x8 (SPI_FAST_FLASH_BOOT)
SPIWP:0xee
mode:DIO, clock div:1
load:0x3fce3808,len:0x370
load:0x403c9700,len:0x900
load:0x403cc700,len:0x2364
entry 0x403c98ac

---FreeRTOS Priority Inheritance Demo---
Task L, Core 1
Task H, Core 1
Task H, Core 1
Task L, Core 0
Task H, Core 1
Task L, Core 0
Task H, Core 1
Task L, Core 0
Task H, Core 1
Task L, Core 0
Task H, Core 1
Task L, Core 0
Task H, Core 1
```

![image](/Exercise%20L9/Core_tskNO_AFFINITY.png)

---

## What these outputs tell me (analysis)

- `xPortGetCoreID()` reports the core where the task is currently executing.
- When I pinned tasks to separate cores:
  - Each task consistently reports the pinned core (Task L: Core 0, Task H: Core 1).
  - Because the tasks are on separate cores they can truly run concurrently and appear interleaved in the serial log.
- When I left both tasks with `tskNO_AFFINITY`:
  - The scheduler was free to run either task on either core.
  - Most prints came from Core 1 early on (likely because the “setup”/creator task runs on core 1 by default and the tasks inherit or start on that core), so both tasks initially ran on Core 1.
  - Over time a task migrated to Core 0 (you can see intermittent "Task L, Core 0" lines), so tasks can move between cores when affinity is not specified.
- I did not see hard resets as a result of setting or clearing affinity—only different core IDs in the printout.
- You may see repeated prints like `Task H, Core 1` twice in a row because the high-priority task can preempt the low-priority task and may run more than once before the lower-priority task gets CPU time (depending on scheduling and hog delays).

Why the migration happens:
- With `tskNO_AFFINITY` the scheduler may place the task on any available core and it can be migrated by the scheduler for load balancing or because a new core became available.
- The initial core choice often depends on the core where the task was created. In Arduino-style setup, the default core for the main task is often core 1.

---


## 🧭 How to Use this Code

### Clone the Project

```bash
git clone https://github.com/SAJIB3489/real-time-system-RTOS/
cd Exercise L9
```