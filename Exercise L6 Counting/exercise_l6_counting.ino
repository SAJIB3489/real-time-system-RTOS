/**
 * FreeRTOS Counting Semaphore Demo (ESP32)
 *
 * Part 1: Synchronize five task creations using a counting semaphore.
 * Part 2: Ask the user to input a sentence, split into words, and create N tasks,
 *         where each task receives one word (msg.body = that word).
 *
 * Updated by: Md Sajib Pramanic
 * Author:Shawn Hymel
 * License: 0BSD
 */

// You'll likely need this on vanilla FreeRTOS
// #include <semphr.h>

// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Settings
static const int num_tasks = 5; // Part 1: Number of tasks to create

// Example struct for passing a string as a parameter
typedef struct Message {
  char body[20];  // word or message (truncated to 19 chars + NUL)
  uint8_t len;    // length of body
} Message;

// Globals
static SemaphoreHandle_t sem_params = NULL; // Part 1: Counts when parameters are read
static SemaphoreHandle_t sem_words  = NULL; // Part 2: Counts when each word-task copies params

//*****************************************************************************
// Tasks

// Part 1: Tasks reading a common stack-based message
void myTaskCommon(void *parameters) {
  // Copy the message struct from the parameter to a local variable
  Message msg = *(Message *)parameters;

  // Increment the semaphore to indicate that the parameter has been read
  xSemaphoreGive(sem_params);

  // Print out message contents
  Serial.print("Received: ");
  Serial.print(msg.body);
  Serial.print(" | len: ");
  Serial.println(msg.len);

  // Wait for a while and delete self
  vTaskDelay(pdMS_TO_TICKS(1000));
  vTaskDelete(NULL);
}

// Part 2: Each task gets its own heap-allocated Message* (unique word)
// This task copies the message, frees the heap param, then signals.
void wordTask(void *parameters) {
  Message *param = (Message *)parameters;

  // Copy to local, then free the heap memory immediately
  Message msg = *param;
  delete param;

  // Signal that this task has safely copied its parameter
  if (sem_words) {
    xSemaphoreGive(sem_words);
  }

  // Print out message contents
  Serial.print("Word: ");
  Serial.print(msg.body);
  Serial.print(" | len: ");
  Serial.println(msg.len);

  // Simulate work, then self-delete
  vTaskDelay(pdMS_TO_TICKS(1000));
  vTaskDelete(NULL);
}

//*****************************************************************************
// Helpers

// Block until a line is received or timeout (ms). Returns true if got a line.
bool readLine(String &out, uint32_t timeout_ms = 60000) {
  Serial.setTimeout(timeout_ms);
  out = Serial.readStringUntil('\n');
  out.trim();
  return out.length() > 0;
}

// Count words in a C string buffer using tokenization
int countWords(char *buf) {
  int count = 0;
  const char *delims = " \t\r\n";
  char *tok = strtok(buf, delims);
  while (tok != NULL) {
    count++;
    tok = strtok(NULL, delims);
  }
  return count;
}

//*****************************************************************************
// Main (runs as its own task with priority 1 on core 1)

void setup() {
  // Configure Serial
  Serial.begin(115200);
  vTaskDelay(pdMS_TO_TICKS(1000)); // Wait a moment so we don't miss Serial output
  Serial.println();
  Serial.println("--- FreeRTOS Counting Semaphore Demo ---");

  // =========================
  // Part 1: Five tasks sync
  // =========================
  {
    Serial.println("Part 1: Synchronizing five tasks using a counting semaphore...");

    // Create counting semaphore (max = num_tasks, initial = 0)
    sem_params = xSemaphoreCreateCounting(num_tasks, 0);
    if (sem_params == NULL) {
      Serial.println("Could not create counting semaphore (Part 1)");
      ESP.restart();
    }

    // Create a message to use as an argument common to all tasks
    Message msg;
    const char text[] = "All your base";
    strncpy(msg.body, text, sizeof(msg.body) - 1);
    msg.body[sizeof(msg.body) - 1] = '\0';
    msg.len = strlen(msg.body);

    // Start tasks
    for (int i = 0; i < num_tasks; i++) {
      char task_name[16];
      snprintf(task_name, sizeof(task_name), "Task %d", i);
      xTaskCreatePinnedToCore(
          myTaskCommon,
          task_name,
          2048,
          (void *)&msg, // common stack var; tasks copy immediately
          1,
          NULL,
          app_cpu);
    }

    // Wait for all tasks to read shared memory
    for (int i = 0; i < num_tasks; i++) {
      xSemaphoreTake(sem_params, portMAX_DELAY);
    }

    // Notify that all tasks have been created and parameters read
    Serial.println("Part 1: All 5 tasks started and copied parameters.");
  }

  // =========================
  // Part 2: Sentence to tasks
  // =========================
  {
    Serial.println();
    Serial.println("Part 2: Enter a sentence. I will create N tasks (N = number of words).");
    Serial.println("Each task receives one word in msg.body (max 19 chars per word).");
    Serial.print("> ");

    // Wait for user input
    String line;
    while (!readLine(line)) {
      Serial.print("> ");
      vTaskDelay(pdMS_TO_TICKS(50));
    }

    // Make two mutable copies for two-pass tokenization
    const size_t BUF_SZ = 512;
    static char buf1[BUF_SZ];
    static char buf2[BUF_SZ];

    line.toCharArray(buf1, BUF_SZ);
    line.toCharArray(buf2, BUF_SZ);

    // First pass: count words
    int word_count = countWords(buf1);
    if (word_count <= 0) {
      Serial.println("No words found. Done.");
      return;
    }

    // Create counting semaphore for word tasks (max = word_count, initial = 0)
    sem_words = xSemaphoreCreateCounting(word_count, 0);
    if (sem_words == NULL) {
      Serial.println("Could not create counting semaphore (Part 2)");
      ESP.restart();
    }

    // Second pass: create one task per word
    const char *delims = " \t\r\n";
    char *tok = strtok(buf2, delims);
    int idx = 0;

    while (tok != NULL) {
      // Prepare heap-allocated message so each task gets its own copy
      Message *m = new Message;
      if (!m) {
        Serial.println("Allocation failed for Message");
        break;
      }
      // Copy token into msg.body (truncate to 19 chars)
      strncpy(m->body, tok, sizeof(m->body) - 1);
      m->body[sizeof(m->body) - 1] = '\0';
      m->len = strlen(m->body);

      // Create a unique task name
      char tname[24];
      snprintf(tname, sizeof(tname), "WordTask %d", idx);

      // Spawn the task for this word
      BaseType_t ok = xTaskCreatePinnedToCore(
          wordTask,
          tname,
          2048,
          (void *)m,   // pass heap pointer; task will delete it
          1,
          NULL,
          app_cpu);

      if (ok != pdPASS) {
        Serial.println("Failed to create word task");
        delete m; // cleanup on failure
        break;
      }

      idx++;
      tok = strtok(NULL, delims);
    }

    // Wait until all created tasks have copied their parameters
    for (int i = 0; i < idx; i++) {
      xSemaphoreTake(sem_words, portMAX_DELAY);
    }

    Serial.print("Part 2: Started ");
    Serial.print(idx);
    Serial.println(" word tasks. They will print their word and exit.");
  }
}

void loop() {
  // Do nothing but allow yielding to lower-priority tasks
  vTaskDelay(pdMS_TO_TICKS(1000));
}