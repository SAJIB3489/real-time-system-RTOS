## Experiments

![WhatsApp Image 2025-09-16 at 16 03 43](https://github.com/user-attachments/assets/28fdb83f-6e92-43fa-9064-02b646011ba1)

<img width="1545" height="443" alt="Screenshot from 2025-09-16 15-49-38" src="https://github.com/user-attachments/assets/e99e576d-4571-4b1c-a463-780feee9c5ce" />


1. After we swap priorities (Serial 1, Compute 2), you see more jitter in the 1 s Serial prints. Compute can preempt Serial, so the timestamps are less regular. The LED stays regular.

2. When we raise the compute loop to 200000, the LED still blinks with the same rhythm. TaskLED has the highest priority and preempts Compute, so its timing holds even if Compute overruns.

3. With period 200 ms and ON 150 ms, the duty cycle is 75%. As ON time approaches the period, the OFF time shrinks to near zero; at ON ≥ period, it overruns and the LED looks almost always on with tiny, irregular off gaps.


## Contributors

1. Md Sajib Pramanic
2. Junaid Khan
3. Mariya Haider
4. Abdul Nizer
