#ifndef TIMEWRAP_H
#define TIMEWRAP_H

/** User-implemented functions. **/
#include <time.h>

/* #include "usr_def.h"
#include "config.h" */

/* THANK GOD FOR <time.h>! */
/* extern uint16_t check_timer(timer_handle_t t_port);
extern timer_handle_t init_timer(void);
extern start_timer(timer_handle_t t_port); 
extern uint16_t reset_timer(timer_handle_t t_port);
extern uint16_t stop_timer(timer_handle_t t_port); */

/* #ifndef USING_DIFFTIME
	extern uint16_t modem_difftime(time_t time_2, time_t time_1);
#else */
	typedef time_t modem_time_t;
	#define modem_difftime(_x, _y) (unsigned int) difftime(_x, _y)
/* #endif */

/* extern time_since_epoch( */
#endif        /*  #ifndef TIMEWRAP_H  */

