# 🧠 ESP32 FreeRTOS Deadlock Demonstration & Resolution

This project demonstrates **deadlock conditions** in FreeRTOS running on an **ESP32**, and shows **three practical methods** to resolve them:

1. **Timeout-based mutex acquisition**  
2. **Mutex hierarchy (consistent locking order)**  
3. **Arbitrator (global mutex control)**  

---

## 🧩 Author

**Md Sajib Pramanic**
Project for FreeRTOS learning on ESP32

## 🚀 Overview

In multitasking systems, **deadlock** occurs when two or more tasks are waiting indefinitely for each other to release resources.

This example uses:
- Two FreeRTOS tasks (`Task A` and `Task B`)
- Two mutexes (`mutex_1`, `mutex_2`)
- Optional **global mutex** (`arbitrator`)

Each task tries to take both mutexes, but in different orders — causing a **deadlock scenario**.

The program demonstrates three resolution strategies by defining a mode at the top of the code.

---

## ⚙️ Hardware & Software Requirements

### 🧩 Hardware
- ESP32 or ESP32-S3 development board  
- USB cable for flashing and serial monitoring  

### 💻 Software
- Arduino IDE (v2.x recommended)
- ESP32 Board Support Package
- FreeRTOS (included in ESP32 SDK)
- Serial Monitor (115200 baud)

---

## 🧪 Deadlock Example

In the **original version**, the following happens:

- **Task A** locks `mutex_1`, then waits for `mutex_2`.
- **Task B** locks `mutex_2`, then waits for `mutex_1`.
- Both tasks wait forever → **deadlock!**

```cpp
// Task A
xSemaphoreTake(mutex_1, portMAX_DELAY);
vTaskDelay(1);
xSemaphoreTake(mutex_2, portMAX_DELAY);

// Task B
xSemaphoreTake(mutex_2, portMAX_DELAY);
vTaskDelay(1);
xSemaphoreTake(mutex_1, portMAX_DELAY);
````

---

## 💡 Deadlock Resolution Methods

### 🕓 1. Timeout-Based Mutex

Add timeouts when taking mutexes and **retry** if not obtained in time.

```cpp
if (xSemaphoreTake(mutex_1, mutex_timeout) == pdTRUE) {
    if (xSemaphoreTake(mutex_2, mutex_timeout) == pdTRUE) {
        // Work
        xSemaphoreGive(mutex_2);
        xSemaphoreGive(mutex_1);
    } else {
        // Back off and try again
        xSemaphoreGive(mutex_1);
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}
```

✅ **Pros:** Simple and flexible
⚠️ **Cons:** Requires retry logic and careful timeout tuning

---

### 🧱 2. Mutex Hierarchy (Consistent Order)

Always lock mutexes in the **same global order** (e.g., `mutex_1` before `mutex_2`).

```cpp
// Both tasks do this:
xSemaphoreTake(mutex_1, portMAX_DELAY);
xSemaphoreTake(mutex_2, portMAX_DELAY);
```

✅ **Pros:** Zero deadlock risk, efficient
⚠️ **Cons:** Must be consistent across all code paths

---

### 🔐 3. Arbitrator (Global Mutex)

Use a **global mutex** to control access to all other mutexes.

```cpp
xSemaphoreTake(arbitrator, portMAX_DELAY);
xSemaphoreTake(mutex_1, portMAX_DELAY);
xSemaphoreTake(mutex_2, portMAX_DELAY);
xSemaphoreGive(arbitrator);
```

✅ **Pros:** Simplest conceptual model, always safe
⚠️ **Cons:** Can slightly reduce concurrency

---

## 🧭 How to Use

### Step 1️⃣ — Clone the Project

```bash
git clone https://github.com/SAJIB3489/real-time-system-RTOS/
cd Exercise L8
```

### Step 2️⃣ — Open in Arduino IDE

* Open `ESP32_Deadlock_Demo.ino`
* Set **Board**: `ESP32 Dev Module`
* Set **Baud Rate**: `115200`

### Step 3️⃣ — Choose a Deadlock Resolution Mode

At the top of the file, uncomment **one** of the following:

```cpp
//#define MODE_TIMEOUT
//#define MODE_HIERARCHY
#define MODE_ARBITRATOR
```

### Step 4️⃣ — Upload and Monitor

Upload to your ESP32 and open the Serial Monitor (`115200 baud`).
You’ll see messages like:

```
Task A took arbitrator
Task A took mutex 1
Task A took mutex 2
Task A released arbitrator
Task A doing some work
Task A going to sleep
Task B took arbitrator
Task B took mutex 1
Task B took mutex 2
Task B released arbitrator
Task B doing some work
Task B going to sleep
```
