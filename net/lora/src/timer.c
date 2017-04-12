#include "lora/timer.h"
#include "lora/utilities.h"

void TimerIrqHandler(void) { }
void TimerStart(struct hal_timer *obj) { }
void TimerStop(struct hal_timer *obj) { }
void TimerReset(struct hal_timer *obj) { }
void TimerSetValue(struct hal_timer *obj, uint32_t value) { }
uint32_t TimerGetCurrentTime(void) { return 0; }
uint32_t TimerGetElapsedTime(uint32_t savedTime) { return 0; }
uint32_t TimerGetFutureTime(uint32_t eventInFuture) { return 0; }
void TimerLowPowerHandler(void) { }
