
#ifndef _PIC_HPP_
#define _PIC_HPP_

#include <stdint.h>
#include "hal/intctrl.hpp"

class PIC : public InterruptController
{
private:

protected:
	void ioWait();

public:
	PIC();

	uint16_t getIRQReg(int ocw3);
	void eoi (uint8_t irqNum) override;
	void remap();
	void disable() override;

	int open(int, int, void*);
	int close(int, int, void*);
};

#endif