#include <stdint.h>

#include "main.hpp"

#include "core/main.hpp"
#include "core/physmgr.hpp"
#include "thr/prcssthr.hpp"
#include "hal/intctrl.hpp"
#include "hw/cpu.hpp"
#include "hw/ports.hpp"
#include "hw/acpi.hpp"
#include "fs/vfs.hpp"
#include "vm86/vm8086.hpp"

extern "C" {
	#include "libk/string.h"
}


void start(void* parent)
{
	kprintf("VGA DRIVER STARTED.\n");
	
	Device* rootDevice = (Device*) parent;
	kprintf("GOT ROOT DEVICE.\n");

	VGAVideo* dev = new VGAVideo();
	kprintf("CREATED VGAVideo.\n");

	rootDevice->addChild(dev);
	kprintf("ADDED VGAVideo.\n");

	dev->open(0, 0, nullptr);
	kprintf("OPENED VGAVideo.\n");

	extern Video* screen;
	screen = dev;

	kprintf("VGA DONE.\n");
}


#pragma GCC optimize ("Ofast")
#pragma GCC optimize ("-fno-strict-aliasing")
#pragma GCC optimize ("-fno-align-labels")
#pragma GCC optimize ("-fno-align-jumps")
#pragma GCC optimize ("-fno-align-loops")
#pragma GCC optimize ("-fno-align-functions")

//#define FAST_PLANE_SWITCH(pl) outb(0x3CE, 4);outb(0x3CF, pl & 3);outb(0x3C4, 2);outb(0x3C5, 1 << (pl & 3));
#define FAST_PLANE_SWITCH(pl) setPlane(pl);

VGAVideo::VGAVideo(): Video("VGA Display")
{

}

int VGAVideo::close(int a, int b, void* c)
{
	return 0;
}

int VGAVideo::open(int a, int b, void* c)
{
	kprintf("::open.\n");

	Vm::loadFileAsThread(kernelProcess, "C:/Banana/System/VGASET.COM", 0x0000, 0x90, 0x12, 0x12);
	kprintf("::vm.\n");
	sleep(1);
	kprintf("::sleep.\n");

	volatile uint8_t * volatile vram = (volatile uint8_t * volatile) (VIRT_LOW_MEGS + 0xA0000);

	setPlane(0);
	memset((void*) vram, 0, 640 * 480 / 8);
	setPlane(1);
	memset((void*) vram, 0, 640 * 480 / 8);
	setPlane(2);
	memset((void*) vram, 0, 640 * 480 / 8);
	setPlane(3);
	memset((void*) vram, 0, 640 * 480 / 8);

	width = 640;
	height = 480;

	mono = false;
	kprintf("::done.\n");

	return 0;
}

uint8_t dither16Data[512][2] = {
{0, 0},
{0, 0},
{0, 4},
{0, 4},
{4, 4},
{4, 4},
{4, 4},
{4, 4},
{0, 0},
{0, 6},
{0, 6},
{0, 6},
{4, 6},
{4, 6},
{4, 6},
{4, 6},
{0, 2},
{0, 2},
{2, 4},
{2, 4},
{6, 6},
{6, 6},
{6, 6},
{6, 6},
{0, 2},
{0, 2},
{2, 6},
{2, 6},
{6, 6},
{6, 6},
{6, 6},
{6, 6},
{2, 2},
{2, 6},
{2, 6},
{2, 6},
{2, 6},
{6, 6},
{2, 6},
{12, 14},
{2, 2},
{2, 2},
{2, 6},
{2, 2},
{2, 2},
{6, 6},
{2, 2},
{12, 14},
{2, 2},
{2, 10},
{2, 6},
{2, 6},
{6, 6},
{6, 6},
{7, 14},
{14, 14},
{2, 2},
{2, 2},
{10, 10},
{10, 10},
{10, 14},
{10, 14},
{14, 14},
{14, 14},
{0, 0},
{0, 0},
{0, 4},
{0, 4},
{4, 4},
{4, 4},
{4, 12},
{4, 4},
{0, 0},
{0, 8},
{0, 8},
{0, 12},
{0, 12},
{4, 12},
{4, 12},
{4, 12},
{0, 2},
{0, 8},
{6, 8},
{6, 8},
{6, 8},
{6, 12},
{6, 12},
{12, 12},
{0, 2},
{0, 10},
{0, 10},
{0, 14},
{0, 14},
{4, 14},
{4, 14},
{12, 12},
{2, 2},
{0, 10},
{0, 10},
{0, 14},
{0, 14},
{4, 14},
{4, 14},
{12, 14},
{2, 2},
{2, 10},
{6, 10},
{6, 10},
{6, 10},
{6, 14},
{6, 14},
{12, 14},
{2, 10},
{2, 10},
{2, 10},
{2, 14},
{2, 14},
{2, 14},
{6, 14},
{14, 14},
{2, 2},
{2, 10},
{10, 10},
{10, 10},
{10, 14},
{10, 14},
{14, 14},
{14, 14},
{0, 1},
{0, 1},
{0, 5},
{0, 5},
{4, 5},
{4, 5},
{4, 5},
{4, 5},
{0, 1},
{0, 8},
{1, 6},
{1, 6},
{5, 6},
{5, 6},
{4, 12},
{12, 12},
{0, 3},
{0, 8},
{8, 8},
{8, 8},
{4, 7},
{4, 7},
{6, 12},
{12, 12},
{0, 3},
{0, 10},
{3, 6},
{3, 6},
{6, 7},
{6, 7},
{4, 14},
{12, 12},
{2, 3},
{0, 10},
{3, 6},
{3, 6},
{6, 7},
{6, 7},
{4, 14},
{12, 14},
{2, 3},
{2, 3},
{2, 7},
{2, 7},
{8, 14},
{8, 14},
{6, 14},
{12, 14},
{2, 3},
{2, 10},
{2, 10},
{2, 14},
{2, 14},
{10, 14},
{7, 14},
{14, 14},
{2, 3},
{10, 10},
{10, 10},
{10, 10},
{10, 14},
{10, 14},
{14, 14},
{14, 14},
{0, 1},
{0, 1},
{0, 5},
{0, 5},
{4, 5},
{4, 5},
{4, 5},
{4, 5},
{0, 1},
{0, 9},
{1, 6},
{0, 13},
{0, 13},
{5, 6},
{4, 13},
{12, 12},
{0, 3},
{0, 9},
{8, 8},
{6, 9},
{6, 9},
{4, 7},
{6, 13},
{12, 12},
{0, 3},
{0, 11},
{3, 6},
{ 7,8 },// { 0, 15 },
{ 7,8 },// { 0, 15 },
{6, 7},
{4, 15},
{12, 12},
{2, 3},
{0, 11},
{3, 6},
{ 7,8 },// { 0, 15 },
{ 7,8 },// { 0, 15 },
{6, 7},
{4, 15},
{12, 14},
{2, 3},
{2, 11},
{2, 7},
{6, 11},
{6, 11},
{8, 14},
{6, 15},
{12, 14},
{2, 3},
{2, 11},
{2, 11},
{2, 15},
{2, 15},
{7, 14},
{7, 14},
{14, 14},
{2, 3},
{10, 10},
{10, 10},
{10, 10},
{10, 14},
{10, 14},
{14, 14},
{14, 14},
{1, 1},
{1, 1},
{1, 5},
{1, 5},
{5, 5},
{5, 5},
{5, 5},
{5, 5},
{1, 1},
{0, 9},
{0, 9},
{0, 13},
{0, 13},
{4, 13},
{4, 13},
{12, 13},
{1, 3},
{0, 9},
{1, 7},
{6, 9},
{6, 9},
{5, 7},
{6, 13},
{12, 13},
{1, 3},
{0, 11},
{1, 7},
{ 7,8 },// { 0, 15 },
{ 7,8 },// { 0, 15 },
{5, 7},
{4, 15},
{12, 13},
{3, 3},
{0, 11},
{3, 7},
{ 7,8 },// { 0, 15 },
{ 7,8 },// { 0, 15 },
{7, 7},
{4, 15},
{12, 15},
{3, 3},
{2, 11},
{3, 7},
{6, 11},
{6, 11},
{7, 7},
{6, 15},
{12, 15},
{3, 3},
{2, 11},
{2, 11},
{2, 15},
{2, 15},
{7, 14},
{7, 14},
{14, 15},
{3, 3},
{10, 11},
{10, 11},
{10, 11},
{10, 15},
{10, 15},
{14, 15},
{14, 15},
{1, 1},
{1, 1},
{1, 5},
{1, 5},
{5, 5},
{5, 5},
{5, 5},
{5, 5},
{1, 1},
{1, 9},
{1, 5},
{1, 13},
{1, 13},
{5, 5},
{5, 13},
{12, 13},
{1, 3},
{1, 3},
{1, 7},
{1, 7},
{5, 7},
{5, 7},
{6, 13},
{12, 13},
{1, 3},
{1, 11},
{1, 7},
{1, 7},
{5, 7},
{5, 7},
{5, 15},
{12, 13},
{3, 3},
{1, 11},
{3, 7},
{3, 7},
{7, 7},
{7, 7},
{5, 15},
{12, 15},
{3, 3},
{3, 3},
{3, 7},
{3, 7},
{7, 7},
{7, 7},
{6, 15},
{12, 15},
{3, 3},
{3, 11},
{3, 11},
{3, 15},
{3, 15},
{10, 15},
{7, 15},
{14, 15},
{3, 3},
{10, 11},
{10, 11},
{10, 11},
{10, 15},
{10, 15},
{14, 15},
{14, 15},
{1, 1},
{1, 9},
{1, 5},
{1, 5},
{5, 5},
{5, 5},
{5, 13},
{13, 13},
{1, 9},
{1, 9},
{1, 9},
{1, 13},
{1, 13},
{5, 13},
{5, 13},
{5, 13},
{1, 3},
{1, 9},
{1, 9},
{1, 13},
{1, 13},
{5, 13},
{5, 13},
{13, 13},
{1, 3},
{1, 11},
{1, 11},
{1, 15},
{1, 15},
{5, 15},
{5, 15},
{13, 13},
{3, 3},
{1, 11},
{1, 11},
{1, 15},
{1, 15},
{5, 15},
{5, 15},
{13, 15},
{3, 3},
{3, 11},
{3, 11},
{3, 15},
{3, 15},
{9, 15},
{7, 15},
{13, 15},
{3, 11},
{3, 11},
{3, 11},
{3, 15},
{3, 15},
{7, 15},
{7, 15},
{7, 15},
{11, 11},
{3, 11},
{11, 11},
{11, 11},
{11, 15},
{11, 15},
{7, 15},
{15, 15},
{1, 1},
{1, 1},
{1, 5},
{1, 5},
{5, 5},
{5, 5},
{13, 13},
{13, 13},
{1, 1},
{1, 9},
{9, 9},
{9, 9},
{9, 13},
{9, 13},
{5, 13},
{13, 13},
{1, 3},
{9, 9},
{9, 9},
{9, 9},
{9, 13},
{9, 13},
{13, 13},
{13, 13},
{1, 3},
{9, 9},
{9, 9},
{9, 9},
{9, 13},
{9, 13},
{13, 13},
{13, 13},
{3, 3},
{9, 11},
{9, 11},
{9, 11},
{9, 15},
{9, 15},
{13, 15},
{13, 15},
{3, 3},
{9, 11},
{9, 11},
{9, 11},
{9, 15},
{9, 15},
{13, 15},
{13, 15},
{11, 11},
{3, 11},
{11, 11},
{11, 11},
{11, 15},
{11, 15},
{7, 15},
{15, 15},
{11, 11},
{11, 11},
{11, 11},
{11, 11},
{11, 15},
{11, 15},
{15, 15},
{15, 15},
};

inline int pixelLookup(int source, int addr)
{
	//80
	//10
	//2
	//return (source >> 22) + ((source >> 14) & 3) + ((source >> 6) & 3);
	return dither16Data[((source & 0xE00000) >> 21) | ((source & 0xE000) >> 10) | ((source & 0xE0) << 1)][addr & 1];
}

void VGAVideo::putrect(int __x, int __y, int maxx, int maxy, uint32_t colour)
{
	uint8_t* vram = (uint8_t*) (VIRT_LOW_MEGS + 0xA0000);
	int col1 = pixelLookup(colour, 0);
	int col2 = pixelLookup(colour, 1);

	//actually passed in as width and height
	maxx += __x;
	maxy += __y;

	for (int x = __x; x < maxx; ++x) {
		int blocks = (maxx - x) >> 3;

		if (blocks && !(x & 7)) {
			int baseaddr = (__y * width + x) >> 3;
			int addr;
			for (int i = 0; i < 4; ++i) {
				FAST_PLANE_SWITCH(i);

				int val1 = (((col1 >> i) & 1) ? 0x55 : 0) | (((col2 >> i) & 1) ? 0xAA : 0);
				int val2 = (((col2 >> i) & 1) ? 0x55 : 0) | (((col1 >> i) & 1) ? 0xAA : 0);

				for (int b = 0; b < blocks; ++b) {
					addr = baseaddr + b;
					for (int y = __y; y < maxy; ++y) {
						vram[addr] = (x + y) & 1 ? val1 : val2;
						addr += width >> 3;
					}
				}
			}

			x += (blocks << 3) - 1;

		} else {
			int baseaddr = (__y * width + x) >> 3;
			int bit = 7 - (x & 7);
			int ww = ~(1 << bit);

			int px1 = (x + __y) & 1 ? col2 : col1;
			int px2 = (x + __y) & 1 ? col1 : col2;

			for (int i = 0; i < 4; ++i) {
				FAST_PLANE_SWITCH(i);
				int addr = baseaddr;

				if (col1 == col2) {
					int shift = (((col1 >> i) & 1) << bit);
					for (int y = __y; y < maxy; ++y) {
						vram[addr] = (vram[addr] & ww) | shift;
						addr += width >> 3;
					}

				} else {
					int shift = (((px1 >> i) & 1) << bit);
					for (int y = __y; y < maxy; y += 2) {
						vram[addr] = (vram[addr] & ww) | shift;
						addr += width >> 2;
					}

					addr = baseaddr + (width >> 3);
					shift = (((px2 >> i) & 1) << bit);
					for (int y = __y + 1; y < maxy; y += 2) {
						vram[addr] = (vram[addr] & ww) | shift;
						addr += width >> 2;
					}
				}
			}
		}

	}
}

void VGAVideo::putpixel(int x, int y, uint32_t colour)
{
	uint8_t* vram = (uint8_t*) (VIRT_LOW_MEGS + 0xA0000);

	int addr = (y * width + x) >> 3;
	int bit = 7 - (x & 7);

	int px = pixelLookup(colour, y + x);

	int w = ~(1 << bit);
	for (int i = 0; i < 4; ++i) {
		FAST_PLANE_SWITCH(i);
		vram[addr] = (vram[addr] & w) | ((px & 1) << bit);
		px >>= 1;
	}
}