
#ifndef _PHYSMGR_HPP_
#define _PHYSMGR_HPP_

#include <stdint.h>
#include <stddef.h>
#include "core/main.hpp"

namespace PhysMem
{
	extern int usablePages;
	extern int usedPages;
	extern size_t highestMem;

	void physicalMemorySetup(uint32_t highestUsedAddr);
	void freePage(size_t address);
	size_t allocatePage();

	size_t allocateDMA(size_t size);
	void freeDMA(size_t addr, size_t size);

	//ONLY TO BE USED BY PHYSMEM AND VIRTMEM
	void setPageState(size_t pageNum, bool state);
}

#endif