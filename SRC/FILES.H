#ifndef FILES_H
#define FILES_H

/* #include "usr_def.h"
#include "config.h" */

#include <stdio.h>

/* File opening and closing. If stdio is not used, comparable
 * routines need to be implemented. Use the macros below as
 * an example on how to implement such routines. */
/* fread/fwrite, size_t issues... */
/* #ifndef USING_STDIO_H
	extern modem_file_t * modem_fopen_read(const char * name);
	extern modem_file_t * modem_fopen_write(const char * name);
	extern int modem_fclose(modem_file_t * stream);
	extern int modem_fseek(modem_file_t * stream, long int offset);
	extern size_t modem_fread(void * ptr, size_t count, modem_file_t * stream);
	extern size_t modem_fwrite(const void * ptr, size_t count, modem_file_t * stream);
	extern int modem_feof(modem_file_t * stream);
#else */
	#define modem_fopen_read(_x) fopen(_x, "rb")
	#define modem_fopen_write(_x) fopen(_x, "wb")
	#define modem_fclose(_x) fclose(_x)
	#define modem_fseek(_x, _y) fseek(_x, _y, SEEK_SET)
	#define modem_fread(_x, _y, _z) fread(_x, sizeof(uint8_t), _y, _z)
	#define modem_fwrite(_x, _y, _z) fwrite(_x, sizeof(uint8_t), _y, _z)
	#define modem_feof(_x) feof(_x)
/* #endif */

typedef FILE modem_file_t;

/* File attributes functions (take in filename, return info). */
/* typedef union file_perm
{
	
	
}FILE_PERM; */

extern unsigned long get_file_size(char * filename);
extern void get_modification_date(char * filename, char * mod_date);
extern void get_permissions(char * filename);
#endif        /*  #ifndef FILES_H  */

