#include "hw/timer/apictimer.hpp"
#include "hal/intctrl.hpp"
#include "hw/ports.hpp"
#include "hw/acpi.hpp"
#include "hw/intctrl/apic.hpp"
#include "core/common.hpp"
#include "hw/cpu.hpp"

#define APIC_REGISTER_LVT_TIMER					0x320
#define APIC_REGISTER_TIMER_INITCNT				0x380
#define APIC_REGISTER_TIMER_CURRCNT				0x390
#define APIC_REGISTER_TIMER_DIV					0x3E0
#define APIC_REGISTER_LVT_MASKED				0x10000
#define APIC_REGISTER_LVT_TIMER_MODE_PERIODIC	0x20000

void apicTimerHandler(regs* r, void* context)
{
	timerHandler(1000000000ULL / (uint64_t)(*((int*) context)));
}

APICTimer::APICTimer() : Timer("APIC Timer")
{
	
}

int APICTimer::open(int hz, int _irqNum, void*)
{
	//set the frequency
	write(hz);

	//set the memory range
	memory[noMems].rangeStart = reinterpret_cast<APIC*>(thisCPU()->intCtrl)->getBase() + 0x300;
	memory[noMems++].rangeLength = 0x100;

	//install it as a legacy handler just to keep it on the same number as the PIT was
	irqNum = _irqNum;
	interrupt = addIRQHandler(irqNum, apicTimerHandler, true, (void*) &frequency);

	return 0;
}

void APICTimer::write(int hz)
{
	frequency = hz;

	uint32_t base = reinterpret_cast<APIC*>(thisCPU()->intCtrl)->getBase();

	uint64_t oldticks = nanoSinceBoot;
	*((uint32_t*) ((uint8_t*) (size_t) base + APIC_REGISTER_TIMER_DIV)) = 0x3;
	*((uint32_t*) ((uint8_t*) (size_t) base + APIC_REGISTER_TIMER_INITCNT)) = 0xFFFFFFFF;
	asm volatile ("sti");
	while (nanoSinceBoot < oldticks + 1000 * 1000 * 1000);
	// 1/5 of a second should have passed

	*((uint32_t*) ((uint8_t*) (size_t) base + APIC_REGISTER_LVT_TIMER)) = APIC_REGISTER_LVT_MASKED;	//APIC timer is OFF!

	uint32_t ticksInASecond = (0xFFFFFFFF - *((uint32_t*) ((uint8_t*) (size_t) base + APIC_REGISTER_TIMER_CURRCNT)));
	uint32_t ticksInTimerHertz = ticksInASecond / hz;

	*((uint32_t*) ((uint8_t*) (size_t) base + APIC_REGISTER_LVT_TIMER)) = irqNum | APIC_REGISTER_LVT_TIMER_MODE_PERIODIC;
	*((uint32_t*) ((uint8_t*) (size_t) base + APIC_REGISTER_TIMER_DIV)) = 0x3;
	*((uint32_t*) ((uint8_t*) (size_t) base + APIC_REGISTER_TIMER_INITCNT)) = ticksInTimerHertz;
}

int APICTimer::close(int a, int b, void* c)
{
	return 0;
}