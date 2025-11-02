## 🧩 Author

**Md Sajib Pramanic**
Project for FreeRTOS learning on ESP32

---
# 🔦 ESP32 FreeRTOS Binary Semaphore Demo

This project demonstrates how to **safely pass a stack-based argument** to a FreeRTOS task using a **binary semaphore** on an ESP32.

---

## 🧠 What the Code Does

Normally, when creating a FreeRTOS task and passing a **stack variable** (e.g., from `setup()`), there’s a risk that the variable’s memory might be reused before the new task copies it.

To prevent that, a **binary semaphore** is used to synchronize between the task creator (`setup()`) and the new task (`blinkLED`):

1. The main function (`setup`) reads a delay value from Serial.  
2. It creates a binary semaphore (`bin_sem`).  
3. It starts the `blinkLED` task and passes the delay value (stack-based variable) as an argument.  
4. The new task copies that value into a **local variable** and then **gives the semaphore** to signal it’s done.  
5. The main function **waits (takes)** the semaphore before finishing — ensuring safe use of the stack variable.


---

## 💡 What I Understood

* Binary semaphores are useful for **synchronization** between tasks.
* They help ensure **safe memory access** when passing **stack-based arguments** to tasks.
* By waiting for the semaphore, the main task guarantees that the new task has **safely copied the parameter** before the original variable goes out of scope.

---

## 🔧 Hardware & Setup

* **Board:** ESP32 / ESP32-S3
* **LED:** Built-in (`LED_BUILTIN`)
* **Serial Baud Rate:** 115200

Upload the code, open the Serial Monitor, enter a delay value (e.g., `500`), and watch the LED blink at that rate.

---

## 📜 Output Example

```
---FreeRTOS Mutex Solution---
Enter a number for delay (milliseconds)
Sending: 500
Received: 500
Done!
```

The LED then blinks every 500 ms continuously.

---

## 🧭 How to Use

### Step 1️⃣ — Clone the Project

```bash
git clone https://github.com/SAJIB3489/real-time-system-RTOS/
cd Exercise L6
```