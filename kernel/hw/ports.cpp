#include "hw/ports.hpp"

uint8_t inb (uint16_t port)
{
	uint8_t ret;
	asm volatile ("inb %1, %0"
				  : "=a"(ret)
				  : "Nd"(port));
	return ret;
}

uint16_t inw (uint16_t port)
{
	uint16_t ret;
	asm volatile ("inw %1, %w0"
				  : "=a"(ret)
				  : "Nd"(port));
	return ret;
}

uint32_t inl (uint16_t port)
{
	uint32_t ret;
	asm volatile ("inl %1, %0"
				  : "=a"(ret)
				  : "Nd"(port));
	return ret;
}

void insb (uint16_t port, void *addr, size_t cnt)
{
	asm volatile ("rep insb" : "+D" (addr), "+c" (cnt) : "d" (port) : "memory");
}

void insw (uint16_t port, void *addr, size_t cnt)
{
	asm volatile ("rep insw" : "+D" (addr), "+c" (cnt) : "d" (port) : "memory");
}

void insl (uint16_t port, void *addr, size_t cnt)
{
	asm volatile ("rep insl" : "+D" (addr), "+c" (cnt) : "d" (port) : "memory");
}

void outb (uint16_t port, uint8_t  val)
{
	asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void outw (uint16_t port, uint16_t val)
{
	asm volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

void outl (uint16_t port, uint32_t val)
{
	asm volatile ("outl %0, %1" : : "a"(val), "Nd"(port));
}

void outsb (uint16_t port, const void *addr, size_t cnt)
{
	asm volatile ("rep outsb" : "+S" (addr), "+c" (cnt) : "d" (port));
}

void outsw (uint16_t port, const void *addr, size_t cnt)
{
	asm volatile ("rep outsw" : "+S" (addr), "+c" (cnt) : "d" (port));
}

void outsl (uint16_t port, const void *addr, size_t cnt)
{
	asm volatile ("rep outsl" : "+S" (addr), "+c" (cnt) : "d" (port));
}