/*
 * TimeOut.cpp
 *
 *  Created on: 22 Feb 2016
 *      Author: john
 */

#include "TimeOut.h"


TimeOut::TimeOut(unsigned int seconds)
{
	sem_init(&sem, 0, 1);
	t = {&sem, seconds};

	pthread_t delayThread;

	pthread_create(&delayThread, NULL, delay, &t);
}

TimeOut::~TimeOut() {
	// TODO Auto-generated destructor stub
}

bool TimeOut::isDone()
{

	if (!sem_trywait(&sem))
	{
		//semaphore is available
		//timer has ended
		return true;
	}
	return false;
}

void TimeOut::Abort() {
}

void* delay(void* args)
{
	threadArgs t = *(threadArgs*) args;

	sem_wait(t.semaphore);
	sleep(t.seconds);
	sem_post(t.semaphore);

	pthread_exit(0);
}
