OUTPUT32="D:/Users/Alex/Desktop/Banana/packages/system/32/0001/system.dll"
OUTPUT64="D:/Users/Alex/Desktop/sysinit64.sys"

AS32=i386-banana-as
AS64=x86_64-banana-as

CXX32=i386-banana-g++
CC32=i386-banana-gcc
LD32=i386-banana-ld

CXX64=x86_64-banana-g++
CC64=x86_64-banana-gcc
LD64=x86_64-banana-ld

ASFLAGS=
CFLAGS=-x c
CXXFLAGS=-x c++
LDFLAGS=-g -T D:/Users/Alex/Desktop/Banana/drivers/dlink.ld

DFIX=python D:/Users/Alex/Desktop/Banana/drivers/dfix.py

CXXOBJECTS32 = $(patsubst %.cpp, %.32, $(wildcard *.cpp) $(wildcard */*.cpp) $(wildcard */*/*.cpp) $(wildcard */*/*/*.cpp) $(wildcard **/*.cpp))
CXXOBJECTS64 = $(patsubst %.cpp, %.64, $(wildcard *.cpp) $(wildcard */*.cpp) $(wildcard */*/*.cpp) $(wildcard */*/*/*.cpp) $(wildcard **/*.cpp))
COBJECTS32 = $(patsubst %.c, %.32, $(wildcard *.c) $(wildcard */*.c) $(wildcard */*/*.c) $(wildcard */*/*/*.c) $(wildcard **/*.c))
COBJECTS64 = $(patsubst %.c, %.64, $(wildcard *.c) $(wildcard */*.c) $(wildcard */*/*.c) $(wildcard */*/*/*.c) $(wildcard **/*.c))

TARGET32 = BananaDriver32
TARGET64 = BananaDriver64

OBJECTS32 = $(CXXOBJECTS32) $(COBJECTS32)
OBJECTS64 = $(CXXOBJECTS64) $(COBJECTS64)

all: build32
build32: $(TARGET32) 

$(TARGET32): $(OBJECTS32)
	$(LD32) $(LDFLAGS) -shared -q -o $(OUTPUT32) $^ -Map=D:/Users/Alex/Desktop/driver32.map

%.32: %.cpp
	$(DFIX) $^
	$(CXX32) $(CXXFLAGS) -c $^x -o $@ -ID:/Users/Alex/Desktop/Banana/kernel -fno-exceptions -fno-rtti
	
%.32: %.c
	$(DFIX) $^
	$(C32) $(CFLAGS) -c $^x -o $@ -ID:/Users/Alex/Desktop/Banana/kernel


$(TARGET64): $(OBJECTS64)
	$(LD64) $(LDFLAGS) -shared -q -o $(OUTPUT64) $^ -Map=D:/Users/Alex/Desktop/driver64.map

%.64: %.cpp
	$(CXX64) $(CXXFLAGS) -c $^ -o $@ -ID:/Users/Alex/Desktop/Banana/kernel -fno-exceptions -fno-rtti
	
%.64: %.c
	$(C64) $(CFLAGS) -c $^ -o $@ -ID:/Users/Alex/Desktop/Banana/kernel

