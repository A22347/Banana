#ifndef _MouseBUS_HPP_
#define _MouseBUS_HPP_

#include <stdint.h>
#include <stddef.h>
#include "hal/device.hpp"

class Mouse: public Device
{
private:

protected:

public:
	Mouse(const char* name);
	virtual ~Mouse();
};

#endif