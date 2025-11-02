## 🧩 Author

**Md Sajib Pramanic**

Project for FreeRTOS learning on ESP32

---
![image](/Exercise%20L6%20Counting/Screenshot%20from%202025-11-03%2000-31-21.png)

# FreeRTOS Counting Semaphore Demo (ESP32)

This example demonstrates FreeRTOS counting semaphores in two parts:

- Part 1: Create and synchronize five tasks using a counting semaphore so the main task can wait until all child tasks have safely copied a shared parameter.
- Part 2: Prompt the user for a sentence, split it into words, and create N tasks (one per word). Each task receives a unique `Message` where `msg.body` is the individual word.

## Features

- Counting semaphore used as a barrier to confirm parameter handoff from creator to tasks.
- Safe parameter passing:
  - Part 1: All tasks copy a common stack-based `Message` immediately and signal via semaphore.
  - Part 2: Each word-task receives a heap-allocated `Message*`, copies it locally, frees the pointer, and signals via semaphore.
- Tasks self-delete after printing their message.

## Hardware/Software

- ESP32 development board (Arduino core)
- Serial monitor at 115200 baud
- FreeRTOS is included with the ESP32 Arduino core


## How It Works

- Message struct:
  - `msg.body` (char[20]) holds the message/word (truncated to 19 characters plus null terminator).
  - `msg.len` is the string length.

- Part 1 (fixed 5 tasks):
  - Creates a counting semaphore with initial count 0 and max 5.
  - Spawns 5 tasks that all copy the same stack-local `Message`.
  - Each task signals the semaphore after copying parameters.
  - The creator waits to take the semaphore 5 times before proceeding.

- Part 2 (N word tasks):
  - Reads one line from Serial as a sentence.
  - Tokenizes into words and spawns one task per word.
  - Each task gets a unique heap-allocated `Message*` populated with that word.
  - The task copies the message to a local variable, frees the pointer, signals the counting semaphore, prints, and exits.


## 🧭 How to Use

### Clone the Project

```bash
git clone https://github.com/SAJIB3489/real-time-system-RTOS/
cd Exercise L6 Counting
```

## Usage

1. Open the sketch and upload to your ESP32.
2. Open Serial Monitor at 115200 baud.
3. Observe Part 1 output as five tasks start and report the same message.
4. When prompted in Part 2, type a sentence and press Enter. You’ll see one line printed per word.

## Example Serial Output

```text
--- FreeRTOS Counting Semaphore Demo ---

Part 1: Synchronizing five tasks using a counting semaphore...

Received: All your base | len: 13

Received: All your base | len: 13Received: All your base | len: 13

Received: All your base | len: 13



Received: All your base | len: 13

Part 1: All 5 tasks started and copied parameters.



Part 2: Enter a sentence. I will create N tasks (N = number of words).

Each task receives one word in msg.body (max 19 chars per word).

> Word: My | len: 2

Word: name | len: 4

Word: is | len: 2

Word: sajib | len: 5

Part 2: Started 4 word tasks. They will print their word and exit.
```

