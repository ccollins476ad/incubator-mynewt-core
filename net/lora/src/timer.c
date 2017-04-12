#include "lora/timer.h"
#include "lora/utilities.h"

void TimerInit( TimerEvent_t *obj, void ( *callback )( void ) ) { }
void TimerIrqHandler(void) { }
void TimerStart( TimerEvent_t *obj ) { }
void TimerStop( TimerEvent_t *obj ) { }
void TimerReset( TimerEvent_t *obj ) { }
void TimerSetValue( TimerEvent_t *obj, uint32_t value ) { }
TimerTime_t TimerGetCurrentTime(void) { return 0; }
TimerTime_t TimerGetElapsedTime(TimerTime_t savedTime) { return 0; }
TimerTime_t TimerGetFutureTime( TimerTime_t eventInFuture ) { return 0; }
void TimerLowPowerHandler(void) { }
