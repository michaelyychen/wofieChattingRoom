#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>

void sfwrite(pthread_mutex_t *lock, FILE* stream, char *fmt, ...){

	pthread_mutex_lock(lock);
	va_list ap;
	va_start(ap,fmt);
	vfprintf(stream,fmt,ap);
	va_end(ap);
	pthread_mutex_unlock(lock);
	
}