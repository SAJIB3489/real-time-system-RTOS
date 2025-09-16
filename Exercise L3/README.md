## Experiments

    1. After we swap priorities (Serial 1, Compute 2), you see more jitter in the 1 s Serial prints. Compute can preempt Serial, so the timestamps are less regular. The LED stays regular.

    2. When we raise the compute loop to 200000, the LED still blinks with the same rhythm. TaskLED has the highest priority and preempts Compute, so its timing holds even if Compute overruns.

    3. With period 200 ms and ON 150 ms, the duty cycle is 75%. As ON time approaches the period, the OFF time shrinks to near zero; at ON ≥ period, it overruns and the LED looks almost always on with tiny, irregular off gaps.
