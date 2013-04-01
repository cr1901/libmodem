#ifndef CONFIG_H
#define CONFIG_H

/* Compile-type defines (Comment out or undefine (preferably both) 
 * any of which do not apply). */

/* Define if using C standard I/O streams for files (such streams may 
 * not exist in all contexts). Define MODEM_FILE (in USR_DEF.H, 
 * abstraction layer section) to be a data type
 * or structure which gives comparable functionality to a FILE
 * stream if NOT using C I/O. Otherwise MODEM_FILE can be safely left
 * undefined. */
#define USING_STDIO_H

/* Define if the system uses little endian byte order. */
#define LITTLE_ENDIAN

/* Define if the target system has no floating point support. */
//#define NO_FLOAT

/* Define if the difftime function exists on the target system.
 * Comparable functionality must be implemented if the function does
 * not exist on the target system's compiler. */
#define USING_DIFFTIME

/* Define if debugging messages should be sent to stderr.
 * Keep in mind that DOS does not permit redirection of stderr (send stdout to
 * NUL as an alternative). 
 * Debugging messages require <stdio.h> and streams to be well-defined and
 * feasible (space wise) for your target. */
 
/* Be prepared for the maximum possible transfer speed to decrease
 * greatly with this option enabled!
 * Example: 19,200 baud on IBM 4.77 MHz is very likely to fail with XMODEM-1K 
 * due to the buffer size of the serial port library not holding the entire packet.
 * There are debug messages being print out after receiving the initial
 * character but before checking whether the character is a valid/expected
 * value. At 19,200 baud and above, the entire packet may arrive between printing
 * out characters to the console and the ISR receiving characters from the serial port. */
//#define USE_DEBUG_MESSAGES

/* Define if messages detailing current transfer should be print.
This is a temporary feature until logic for the code to yield 
(via cooperative-multitasking or faking it) is supported. */
#define DISPLAY_MESSAGES

#endif
