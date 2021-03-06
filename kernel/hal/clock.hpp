#ifndef _CLOCK_HPP_
#define _CLOCK_HPP_

#include <stdint.h>
#include <stddef.h>
#include "hal/device.hpp"
#include "fs/vfs.hpp"			// time_t

struct datetime_t
{
	uint8_t day;		//1-31
	uint8_t month;		//0-11
	uint16_t year;		//years since 1970
	uint8_t hour;		//0-23
	uint8_t minute;		//0-59
	uint8_t second;		//0-59
};

time_t datetimeToSeconds(datetime_t dt);
datetime_t secondsToDatetime(time_t tim);

class Clock: public Device
{
private:

protected:

public:
	virtual time_t timeInSecondsUTC() = 0;
	virtual datetime_t timeInDatetimeUTC() = 0;

	virtual bool setTimeInSecondsUTC(time_t t) = 0;
	virtual bool setTimeInDatetimeUTC(datetime_t d) = 0;

	time_t timeInSecondsLocal();
	datetime_t timeInDatetimeLocal();

	bool setTimeInSecondsLocal(time_t t);
	bool setTimeInDatetimeLocal(datetime_t d);

	Clock(const char* name);
	virtual ~Clock();
};

void loadClockSettings();

#endif