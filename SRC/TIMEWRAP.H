#ifndef TIMEWRAP_H
#define TIMEWRAP_H

/** User-implemented functions. **/
#include <time.h>
#include <stdint.h>

//#include "usr_def.h"
//#include "config.h"

/* #ifndef USING_DIFFTIME
	extern uint16_t modem_difftime(time_t time_2, time_t time_1);
#else */
	typedef time_t modem_time_t;
	#define modem_difftime(_x, _y) (uint16_t) difftime(_x, _y)
//#endif

//extern time_since_epoch(
#endif        //  #ifndef TIMEWRAP_H

