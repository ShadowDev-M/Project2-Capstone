#pragma once



#ifndef MEMORYMONITOR_H
#define MEMORYMONITOR_H
#include <iostream>
#include <memory>
#include "MemorySize.h"
#include <iostream>

void RecordNextUndefinedNew(const char* file = __FILE__, int line = __LINE__);

void* operator new(std::size_t numBytes);

void* operator new(std::size_t numBytes, const char* FILE, int LINE);

void operator delete(void* memoryLocation, std::size_t numBytes) noexcept;

void operator delete(void* memoryLocation) noexcept;

void operator delete(void* memoryLocation, const char* file, int line) noexcept;

/// This is a trick to get access to numBytes 
/// w/o wild typecasts. 
struct ArraySize {
    std::size_t numBytes;
};

extern bool allocMapDestroyed;

void* operator new[](std::size_t numBytes);

void* operator new[](std::size_t numBytes, const char* FILE, int LINE);

void operator delete[](void* memoryLocation) noexcept;

void operator delete[](void* memoryLocation, const char* file, int line) noexcept;

#endif


