I am making an RTOS for the ARM Cortex-M4.

Features:
- Fixed priority scheduler
- Mutex
- Semaphore
- Memory pool/allocator with MPU support to prevent stack overflows.
- Message queue
- Deferred logging via UART + DMA (with Python script to translate debug logs to human readable format)
