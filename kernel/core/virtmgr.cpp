#include "core/common.hpp"
#include "hw/cpu.hpp"
#include "core/virtmgr.hpp"
#include "core/physmgr.hpp"
#include "fs/vfs.hpp"
#include "thr/prcssthr.hpp"
#include "thr/elf.hpp"

VAS* firstVAS = nullptr;
#pragma GCC optimize ("O2")
#pragma GCC optimize ("-fno-strict-aliasing")
#pragma GCC optimize ("-fno-align-labels")
#pragma GCC optimize ("-fno-align-jumps")
#pragma GCC optimize ("-fno-align-loops")
#pragma GCC optimize ("-fno-align-functions")
//#pragma GCC optimize ("-fpeephole2")

namespace Virt
{
	enum class VirtPageState: uint8_t
	{
		//we only have 1 nibble here

		Free = 0,
		Used = 1,
		Start = 2,
		End = 3,
		StartAndEnd = 4,
		Unusable = 0xF,
	};

	uint8_t* bitmap = (uint8_t*) VIRT_VIRT_PAGE_BITMAP;

	void setPageState(size_t pageNum, VirtPageState state)
	{
		pageNum -= VIRT_HEAP_MIN / 4096;

		size_t byte = pageNum / 2;
		size_t nibble = pageNum & 1;

		if (!nibble) {
			uint8_t* ramAddr = bitmap + byte;
			*ramAddr &= 0xF0;
			*ramAddr |= (uint8_t) state;

		} else {
			uint8_t* ramAddr = bitmap + byte;
			*ramAddr &= 0x0F;
			*ramAddr |= ((uint8_t) state) << 4;
		}
	}

	VirtPageState getPageState(size_t pageNum)
	{
		pageNum -= VIRT_HEAP_MIN / 4096;

		size_t byte = pageNum / 2;
		size_t nibble = pageNum & 1;

		if (!nibble) {
			uint8_t* ramAddr = bitmap + byte;
			return (VirtPageState) (*ramAddr & 0xF);

		} else {
			uint8_t* ramAddr = bitmap + byte;
			return (VirtPageState) ((*ramAddr >> 4) & 0xF);
		}

		return VirtPageState::Unusable;
	}

	size_t pageAllocPtr = VIRT_HEAP_MIN / 4096;

	size_t allocateKernelVirtualPages(int pages)
	{
		bool failures = false;

		size_t inARow = 0;
		size_t firstInRow = 0;

		while (1) {
			VirtPageState state = getPageState(pageAllocPtr);

			if (state == VirtPageState::Free) {
				if (inARow == 0) {
					firstInRow = pageAllocPtr;
				}
				++inARow;

				if (inARow == pages) {
					pageAllocPtr = firstInRow;

					if (pages == 1) {
						setPageState(pageAllocPtr++, VirtPageState::StartAndEnd);
					} else {
						for (size_t i = 0; i < pages; ++i) {
							if (i == 0)						 setPageState(pageAllocPtr++, VirtPageState::Start);
							else if (i == pages - 1)		 setPageState(pageAllocPtr++, VirtPageState::End);
							else							 setPageState(pageAllocPtr++, VirtPageState::Used);
						}
					}


					size_t x = firstInRow * 4096;

					return x;
				}

			} else {
				inARow = 0;
			}
			pageAllocPtr++;

			if (pageAllocPtr > VIRT_HEAP_MAX / 4096) {
				pageAllocPtr = VIRT_HEAP_MIN / 4096;
				if (failures) {
					panic("KERNEL VIRTUAL MEMORY EXHAUSTED");
				}
				failures = true;
			}
		}

		return 0;
	}

	void freeKernelVirtualPages(size_t address)
	{
		uint64_t page = address / 4096;
		bool first = true;

		while (1) {
			VirtPageState state = getPageState(page);

			if (state == VirtPageState::Free) {
				panic("FREEING PAGES NOT ALLOCATED");
			}

			size_t* entry = getAKernelVAS()->getPageTableEntry(page * 4096);
			if (*entry & PAGE_ALLOCATED) {
				Phys::freePage(*entry & ~0xFFF);
			}

			if (state == VirtPageState::StartAndEnd) {
				if (first) {
					setPageState(page, VirtPageState::Free);
				} else {
					panic("FREEING PAGES START AND END PROBLEM");
				}
				return;

			} else if (state == VirtPageState::Start) {
				if (first) {
					setPageState(page, VirtPageState::Free);
				} else {
					panic("FREEING PAGES START PROBLEM");
				}

			} else if (state == VirtPageState::End) {
				if (!first) {
					setPageState(page, VirtPageState::Free);
				} else {
					panic("FREEING PAGES END PROBLEM");
				}
				return;

			} else if (state == VirtPageState::Used) {
				setPageState(page, VirtPageState::Free);
			}

			++page;
			first = false;
		}
	}

	void virtualMemorySetup()
	{
		int start = VIRT_HEAP_MIN / 4096;
		int end = VIRT_HEAP_MAX / 4096;

		for (int i = start; i < end; ++i) {
			setPageState(i, VirtPageState::Unusable);
		}

		kprintf("KHEAP START = 0x%X\n", start * 4096);
		kprintf("KHEAP END   = 0x%X\n", end * 4096);

		for (int i = start; i < end; ++i) {
			setPageState(i, VirtPageState::Free);
		}

		if (1) {
			for (int i = 0xD0000; i < 0xE0000; ++i) {
				setPageState(i, VirtPageState::Unusable);
			}
		}
	}

	VAS* getAKernelVAS()
	{
		return firstVAS;
	}

	void setupPageSwapping(int megs)
	{
		File* f = new File("C:/Banana/SWAPFILE.SYS", kernelProcess);
		f->unlink();
		FileStatus st = f->open(FILE_OPEN_WRITE_NORMAL);
		if (st != FileStatus::Success) {
			kprintf("st = %d\n", st);
			panic("NO PAGE SWAPPING AVAILABLE");
		}

		int br = 0;
		int pages = megs * 256;
		uint8_t* buff = (uint8_t*) malloc(4096 * 16);
		memset(buff, 0, 4096 * 16);
		pages /= 16;
		while (pages--) {
			st = f->write(4096 * 16, buff, &br);
			if (st != FileStatus::Success) {
				kprintf("Status != success = %d\n", (int) st);
			}
			if (br != 4096 * 16) {
				kprintf("Br = %d\n", br);
				panic("UH OH");
			}
		}

		f->close();
		rfree(buff);
		delete f;
	}
}

size_t VAS::allocatePages(int count, int flags)
{
	bool invlpg = thisCPU()->features.hasINVLPG;

	if (supervisorVAS) {
		size_t virt = Virt::allocateKernelVirtualPages(count);
		if (virt >= VIRT_KERNEL_BASE && thisCPU()->features.hasGlobalPages) {
			flags |= PAGE_GLOBAL;
		}
		for (int i = 0; i < count; ++i) {
			size_t phys = Phys::allocatePage();
			mapPage(phys, virt + i * 4096, flags | PAGE_ALLOCATED);

			if (invlpg) {
				asm volatile ("invlpg (%0)" : : "b"((void*) (virt + i * 4096)) : "memory");
			}
		}

		if (!invlpg) {
			CPU::writeCR3(CPU::readCR3());
		} else {
			//invalidate the recursive structure
			size_t invaddrLow = (0xFFC00000 + ((virt / 0x400) & ~0xFFF));
			size_t invaddrHigh = (0xFFC00000 + (((virt + count * 4096) / 0x400) & ~0xFFF));

			while (invaddrLow <= invaddrHigh) {
				asm volatile ("invlpg (%0)" : : "b"((void*) invaddrLow) : "memory");
				invaddrLow += 4096;
			}
		}

		return virt;

	} else {
		if (!sbrk) {
			panic("NEED TO LOAD TASK BEFORE ALLOCATING PAGES");
		}

		size_t virt = sbrk;
		sbrk += count * 4096;

		for (int i = 0; i < count; ++i) {
			size_t phys = Phys::allocatePage();
			mapPage(phys, virt + i * 4096, flags | PAGE_ALLOCATED);
			if (invlpg) {
				asm volatile ("invlpg (%0)" : : "b"((void*) (virt + i * 4096)) : "memory");
			}
		}

		if (!invlpg) {
			CPU::writeCR3(CPU::readCR3());
		} else {
			//invalidate the recursive structure
			size_t invaddrLow = (0xFFC00000 + ((virt / 0x400) & ~0xFFF));
			size_t invaddrHigh = (0xFFC00000 + (((virt + count * 4096) / 0x400) & ~0xFFF));

			while (invaddrLow <= invaddrHigh) {
				asm volatile ("invlpg (%0)" : : "b"((void*) invaddrLow) : "memory");
				invaddrLow += 4096;
			}
		}

		return virt;
	}

	return 0;
}

size_t VAS::virtualToPhysical(size_t virt)
{
	return (*getPageTableEntry(virt)) & ~0xFFF;
}

void VAS::freeAllocatedPages(size_t virt) {
	if (supervisorVAS) {
		Virt::freeKernelVirtualPages(virt);

	} else {
		kprintf("@@@ VAS::freeAllocatedPages TODO!");
	}
}

VAS::VAS()
{
	firstVAS = this;

	supervisorVAS = true;
	specialFirstVAS = true;
	pageDirectoryBase = (size_t*) VIRT_KRNL_PAGE_DIRECTORY;
}

VAS::~VAS()
{
	int fp = 0;

	lockScheduler();

	currentTaskTCB->processRelatedTo->vas->mapOtherVASIn(true, this);

	for (int i = 0; i < 256 * 3; ++i) {
		size_t oldEntry = pageDirectoryBase[i];

		if (oldEntry & PAGE_PRESENT) {
			for (int j = 0; j < 1024; ++j) {
				size_t vaddr = ((size_t) i) * 0x400000 + ((size_t) j) * 0x1000;
				size_t* oldPageEntryPtr = currentTaskTCB->processRelatedTo->vas->getForeignPageTableEntry(true, vaddr);
				size_t oldPageEntry = *oldPageEntryPtr;

				if ((oldPageEntry & PAGE_ALLOCATED) && (oldPageEntry & PAGE_PRESENT)) {
					Phys::freePage(oldPageEntry & ~0xFFF);
					++fp;
				}
			}

			if (oldEntry & PAGE_ALLOCATED) {
				Phys::freePage(oldEntry & ~0xFFF);
				++fp;
			}
		}
	}

	Virt::freeKernelVirtualPages((size_t) pageDirectoryBase);
	Phys::freePage(pageDirectoryBasePhysical);
	++fp;

	kprintf("Freed %d KB from VAS deletion.\n", fp * 4);
	
	unlockScheduler();
}

void VAS::setCPUSpecific(size_t physAddr)
{
	//map the CPU specific data in (kernel mode can write to readable pages)
	mapPage(physAddr, VIRT_CPU_SPECIFIC, PAGE_PRESENT | PAGE_READONLY | PAGE_USER);
}

VAS::VAS(VAS* old)
{
	panic("VAS::VAS(VAS* old) not implemented");
}

VAS::VAS(bool kernel) {
	supervisorVAS = kernel;

	pageDirectoryBasePhysical = Phys::allocatePage();

	//DO NOT mark as allocated, as we shouldn't be able to swap out a page table
	pageDirectoryBase = (size_t*) Virt::getAKernelVAS()->mapRange(pageDirectoryBasePhysical, Virt::allocateKernelVirtualPages(1), 1, PAGE_PRESENT | PAGE_WRITABLE | PAGE_SUPERVISOR);

	//make everything non-present to start with
	for (int i = 0; i < 1024; ++i) {
		pageDirectoryBase[i] = PAGE_NOT_PRESENT | PAGE_WRITABLE | PAGE_SUPERVISOR;
	}

	//map in the kernel
	for (int i = 768; i < 1024; ++i) {
		pageDirectoryBase[i] = PAGE_PRESENT | PAGE_SUPERVISOR | PAGE_WRITABLE | (0x100000 + (i - 768) * 4096) | (thisCPU()->features.hasGlobalPages ? PAGE_GLOBAL : 0);

		if (1 && (i - 768) >= 64 && (i - 768) < 64 * 3) {
			pageDirectoryBase[i] = PAGE_NOT_PRESENT | PAGE_WRITABLE | PAGE_SUPERVISOR;
		}
	}

	pageDirectoryBase[0xC20 / 4] = 0x4003 | (thisCPU()->features.hasGlobalPages ? PAGE_GLOBAL : 0);

	//the first VAS on each CPU gets called with a different constructor
	setCPUSpecific((size_t) thisCPU()->cpuSpecificPhysAddr);

	//set up recursive mapping (wizardry!)
	pageDirectoryBase[1023] = (size_t) pageDirectoryBasePhysical | PAGE_PRESENT | PAGE_WRITABLE | PAGE_SUPERVISOR | (thisCPU()->features.hasGlobalPages ? PAGE_GLOBAL : 0);

	if (!strcmp(thisCPU()->getName(), "Intel Pentium")) {
		disableIRQs();
		mapPage((*getPageTableEntry(thisCPU()->idt.getPointerToInvalidOpcodeEntryForF00F())) & ~0xFFF, thisCPU()->idt.getPointerToInvalidOpcodeEntryForF00F() & ~0xFFF, PAGE_PRESENT | PAGE_SUPERVISOR | PAGE_CACHE_DISABLE);
		enableIRQs();
	}
}

size_t VAS::mapRange(size_t physicalAddr, size_t virtualAddr, int pages, int flags)
{
	bool invlpg = thisCPU()->features.hasINVLPG;

	for (int i = 0; i < pages; ++i) {
		mapPage(physicalAddr + i * 4096, virtualAddr + i * 4096, flags);

		if (invlpg) {
			asm volatile ("invlpg (%0)" : : "b"((void*) (virtualAddr + i * 4096)) : "memory");
		}
	}

	if (!invlpg) {
		CPU::writeCR3(CPU::readCR3());
	} else {
		//invalidate the recursive structure
		size_t invaddrLow = (0xFFC00000 + ((virtualAddr / 0x400) & ~0xFFF));
		size_t invaddrHigh = (0xFFC00000 + (((virtualAddr + pages * 4096) / 0x400) & ~0xFFF));

		while (invaddrLow <= invaddrHigh) {
			asm volatile ("invlpg (%0)" : : "b"((void*) invaddrLow) : "memory");
			invaddrLow += 4096;
		}
	}

	return virtualAddr;
}

size_t* VAS::getForeignPageTableEntry(bool secondSlot, size_t virt)
{
	size_t pageTableNumber = virt / 0x400000;

	/*if (!(pageDirectoryBase[pageTableNumber] & PAGE_PRESENT)) {
		panic("VAS::getForeignPageTableEntry NOT PRESENT");
	}*/

	size_t pageNumber = (virt % 0x400000) / 0x1000;
	size_t* pageTable = (size_t*) ((secondSlot ? VIRT_RECURSIVE_SPOT_2 : VIRT_RECURSIVE_SPOT_1) + pageTableNumber * 0x1000);

	return pageTable + pageNumber;
}

size_t* VAS::getPageTableEntry(size_t virt)
{
	size_t pageTableNumber = virt / 0x400000;

	/*if (!(pageDirectoryBase[pageTableNumber] & PAGE_PRESENT)) {
		panic("VAS::getPageTableEntry NOT PRESENT");
	}*/

	size_t pageNumber = (virt % 0x400000) / 0x1000;
	size_t* pageTable = (size_t*) (0xFFC00000 + pageTableNumber * 0x1000);

	return pageTable + pageNumber;
}

void VAS::reflagRange(size_t virtualAddr, int pages, int andFlags, int orFlags)
{
	for (int i = 0; i < pages; ++i) {
		size_t* entry = getPageTableEntry(virtualAddr + i * 4096);
		*entry &= andFlags;
		*entry |= orFlags;
	}
}

void VAS::setToWriteCombining(size_t virtualAddr, int pages)
{
	if (thisCPU()->features.hasPAT) {
		reflagRange(virtualAddr, pages, -1, PAGE_PAT);
	}
}

void VAS::mapOtherVASIn(bool secondSlot, VAS* other)
{
	//recursively map the thingy
	pageDirectoryBase[(secondSlot ? VIRT_RECURSIVE_SPOT_2 : VIRT_RECURSIVE_SPOT_1) / 0x400000] = other->pageDirectoryBasePhysical | PAGE_PRESENT | PAGE_WRITABLE | PAGE_SUPERVISOR;

	//flush TLB
	CPU::writeCR3(CPU::readCR3());
}

void VAS::mapForeignPage(bool secondSlot, VAS* other, size_t physicalAddr, size_t virtualAddr, int flags)
{
	if ((virtualAddr | physicalAddr) & 0xFFF) {
		panic("UNALIGNED PAGE MAPPING REQUESTED 2");
	}

	size_t pageTableNumber = virtualAddr / 0x400000;

	if (!(other->pageDirectoryBase[pageTableNumber] & PAGE_PRESENT)) {
		//create the page table first
		size_t addr = Phys::allocatePage();

		//clear it
		void* virtaddr = (void*) Virt::getAKernelVAS()->mapRange(addr, Virt::allocateKernelVirtualPages(1), 1, PAGE_PRESENT | PAGE_WRITABLE | PAGE_SUPERVISOR);
		memset(virtaddr, 0, 4096);
		Virt::freeKernelVirtualPages((size_t) virtaddr);

		other->pageDirectoryBase[pageTableNumber] = addr | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER | PAGE_ALLOCATED;
	}

	//map it in
	size_t pageNumber = (virtualAddr % 0x400000) / 0x1000;
	size_t* pageTable = (size_t*) ((secondSlot ? VIRT_RECURSIVE_SPOT_2 : VIRT_RECURSIVE_SPOT_1) + pageTableNumber * 0x1000);
	pageTable[pageNumber] = physicalAddr | flags;
}

void VAS::mapPage(size_t physicalAddr, size_t virtualAddr, int flags) {
	if (virtualAddr < VIRT_KERNEL_BASE) {
		size_t cr3;
		asm volatile ("mov %%cr3, %0" : "=r"(cr3));
		if (cr3 != (size_t) pageDirectoryBasePhysical) {
			kprintf("\n\nFATAL 'WARNING':\n    CANNOT MAP NON-KERNEL IN NON-CURRENT VAS.\n    THIS COULD BE A *FATAL ERROR*.\n");
			//panic("CANNOT MAP NON-KERNEL IN NON-CURRENT VAS");
		}
	}
	
	if ((virtualAddr | physicalAddr) & 0xFFF) {
		panic("UNALIGNED PAGE MAPPING REQUESTED");
	}

	size_t pageTableNumber = virtualAddr / 0x400000;

	if (!(pageDirectoryBase[pageTableNumber] & PAGE_PRESENT)) {		
		//create the page table first
		size_t addr = Phys::allocatePage();

		//clear the page
		void* virtaddr = (void*) Virt::getAKernelVAS()->mapRange(addr, Virt::allocateKernelVirtualPages(1), 1, PAGE_PRESENT | PAGE_WRITABLE | PAGE_SUPERVISOR);
		memset(virtaddr, 0, 4096);
		Virt::freeKernelVirtualPages((size_t) virtaddr);

		pageDirectoryBase[pageTableNumber] = addr | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER | PAGE_ALLOCATED;
	}

	//map it in
	size_t pageNumber = (virtualAddr % 0x400000) / 0x1000;
	size_t* pageTable = (size_t*) (0xFFC00000 + pageTableNumber * 0x1000);

	pageTable[pageNumber] = physicalAddr | flags;
}

extern uint8_t inb(uint16_t);

extern "C" void mapVASFirstTime()
{
	if (currentTaskTCB->firstTimeEIP == 1) {
		kprintf("STARTING A FORKED TASK.\n");
		return;
	}

	int threadNo = currentTaskTCB->rtid;
	VAS* vas = currentTaskTCB->processRelatedTo->vas;

	//12KB kernel (interrupt handler) stack
	for (int i = 0; i < 3; ++i) {
		vas->mapRange(Phys::allocatePage(), VIRT_APP_STACK_KRNL_TOP - 4096 * (1 + i) - threadNo * SIZE_APP_STACK_TOTAL, 1, PAGE_PRESENT | PAGE_ALLOCATED | PAGE_WRITABLE | PAGE_SUPERVISOR);
	}

	//OLD: 8KB user (or kernel mode task) stack
	//NEW: 128KB user stack
	for (int i = 0; i < 32; ++i) {
		vas->mapRange(Phys::allocatePage(), VIRT_APP_STACK_USER_TOP - 4096 * (1 + i) - threadNo * SIZE_APP_STACK_TOTAL, 1, PAGE_PRESENT | PAGE_ALLOCATED | PAGE_WRITABLE | (vas->supervisorVAS ? PAGE_SUPERVISOR : PAGE_USER));
	}

	CPU::writeCR3(CPU::readCR3());
}