#ifndef _MAIN_HPP_
#define _MAIN_HPP_

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

extern "C" void kernel_main();

#define KERNEL_DEBUG 

#ifdef THIRTYTWOBIT
#define PLATFORM_ID 86
#else
#define PLATFORM_ID 64
#endif

#define OS_VERSION_MAJ 0
#define OS_VERSION_MIN 1
#define OS_VERSION_PATCH1 0
#define OS_VERSION_PATCH2 0
#define OS_VERSION_STRING "Banana 0.1.3"
#define OS_COPYRIGHT "Copyright Alex Boxall 2016-2021. All rights reserved."

constexpr size_t PHYS_RAM_TABLE_SIZE			= 0x513;
constexpr size_t PHYS_HIGHEST_UNUSED_ADDRESS	= 0x524;
constexpr size_t PHYS_RAM_TABLE					= 0x600;
constexpr size_t PHYS_DMA_MEMORY_1				= 0x10000;
constexpr size_t PHYS_PHYS_PAGE_BITMAP			= 0x20000;
constexpr size_t PHYS_VIRT_PAGE_BITMAP			= 0x40000;
constexpr size_t PHYS_DMA_MEMORY_2				= 0x60000;
constexpr size_t PHYS_ALLOCED_VIRT_PAGES		= 0x100000;
constexpr size_t PHYS_KRNL_PAGE_DIRECTORY		= 0x1000;


constexpr size_t VIRT_KERNEL_BASE				= 0xC0000000U;
constexpr size_t VIRT_LOW_MEGS					= 0x2000000 + VIRT_KERNEL_BASE;
constexpr size_t VIRT_CPU_SPECIFIC				= 0x2400000 + VIRT_KERNEL_BASE;
constexpr size_t VIRT_RECURSIVE_SPOT_1			= 0x2800000 + VIRT_KERNEL_BASE;
constexpr size_t VIRT_RECURSIVE_SPOT_2			= 0x2C00000 + VIRT_KERNEL_BASE;

constexpr size_t VIRT_DMA_MEMORY_1				= VIRT_LOW_MEGS + PHYS_DMA_MEMORY_1;
constexpr size_t SIZE_DMA_MEMORY_1				= 0x10000;
constexpr size_t VIRT_DMA_MEMORY_2				= VIRT_LOW_MEGS + PHYS_DMA_MEMORY_2;
constexpr size_t SIZE_DMA_MEMORY_2				= 0x20000;

constexpr size_t VIRT_KRNL_PAGE_DIRECTORY		= VIRT_LOW_MEGS + PHYS_KRNL_PAGE_DIRECTORY;

constexpr size_t VIRT_PHYS_PAGE_BITMAP			= VIRT_LOW_MEGS + PHYS_PHYS_PAGE_BITMAP;
constexpr size_t SIZE_PHYS_PAGE_BITMAP			= 128 * 1024;

constexpr size_t VIRT_VIRT_PAGE_BITMAP			= VIRT_LOW_MEGS + PHYS_VIRT_PAGE_BITMAP;
constexpr size_t SIZE_VIRT_PAGE_BITMAP			= 32 * 1024;

constexpr size_t VIRT_RAM_TABLE					= VIRT_LOW_MEGS + PHYS_RAM_TABLE;
constexpr size_t VIRT_RAM_TABLE_SIZE			= VIRT_LOW_MEGS + PHYS_RAM_TABLE_SIZE;

constexpr size_t VIRT_HIGHEST_UNUSED_ADDRESS	= VIRT_LOW_MEGS + PHYS_HIGHEST_UNUSED_ADDRESS;

constexpr size_t SIZE_APP_STACK_USER			= 0x180000;
constexpr size_t SIZE_APP_STACK_KRNL			= 0x80000;
constexpr size_t SIZE_APP_STACK_TOTAL			= SIZE_APP_STACK_USER + SIZE_APP_STACK_KRNL;

constexpr size_t VIRT_APP_DATA					= 0x10000000;		//TODO will be changed to 0x2000000 after we rebuild / tweak the linker 
constexpr size_t VIRT_APP_STACK_USER_TOP		= 0x2000000;
constexpr size_t VIRT_APP_STACK_KRNL_TOP		= VIRT_APP_STACK_USER_TOP - SIZE_APP_STACK_USER;

constexpr size_t VIRT_HEAP_MIN					= VIRT_KERNEL_BASE + 0x08000000;
constexpr size_t VIRT_HEAP_MAX					= 0xEFFFFFFFU;
constexpr size_t VIRT_SBRK_MIN					= 0xF0000000U;
constexpr size_t VIRT_SBRK_MAX					= 0xFFC00000U;

constexpr size_t VIRT_ALLOCED_VIRT_PAGES		= VIRT_LOW_MEGS + 0x100000;

constexpr size_t VIRT_ACPI_DRIVER				= 0xC2484000U;

extern uint32_t sysBootSettings;

#include "core/terminal.hpp"


#endif