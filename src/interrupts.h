// gestione degli interrupt

void handleInterrupts();

static void handlePLTInterrupt();

static void handleIntervalTimerInterrupt();

static void handleDeviceInterrupt(unsigned int interruptLine);

static unsigned int getDeviceNo(unsigned int interruptLine);

static inline void acknowledgeInterrupt(memaddr devBaseAddr);