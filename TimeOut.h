/*
 * TimeOut.h
 *
 *  Created on: 22 Feb 2016
 *      Author: john
 */

#ifndef TIMEOUT_H_
#define TIMEOUT_H_

#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <pthread.h>
#include <semaphore.h>
#include <ctime>
#include <assert.h>
#include <errno.h>
#include <signal.h>


void* delay(void* args);

typedef struct {
	sem_t *semaphore;
	unsigned int seconds;
} threadArgs;

class TimeOut {
private:
	sem_t sem;
	bool done = false;

	threadArgs t;

public:
	TimeOut(unsigned int seconds);
	virtual ~TimeOut();
	bool isDone();
	void Abort();
};



#endif /* TIMEOUT_H_ */
