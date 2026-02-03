#pragma once

#ifndef MEMORYMONITOR_H
#define MEMORYMONITOR_H
#include <iostream>
#include <memory>
#include "MemorySize.h"



void* operator new(std::size_t numBytes) {
 //   std::cout << "allocating " << numBytes << " bytes of memory\n";
    MEMORY_NUMUSEDBYTES += numBytes;
    return std::malloc(numBytes);
}

void operator delete(void* memoryLocation, std::size_t numBytes) {
  //  std::cout << "freeing " << numBytes << " bytes of memory\n";
    MEMORY_NUMUSEDBYTES -= numBytes;
    if (MEMORY_NUMUSEDBYTES == 0) {

        std::cout << " e";
    }
    std::free(memoryLocation);
}


/// This is a trick to get access to numBytes 
/// w/o wild typecasts. 
struct ArraySize {
    std::size_t numBytes;
};

/// I did a little hack to hide the total number of bytes
/// allocated in the array itself. 
void* operator new[](std::size_t numBytes) {


   // std::cout << "allocating an array of " << numBytes << " bytes of memory\n";
    ArraySize* array = reinterpret_cast<ArraySize*>(std::malloc(numBytes + sizeof(ArraySize)));
    if (array) array->numBytes = numBytes;
    MEMORY_NUMUSEDBYTES += array->numBytes;
    return array + 1;
}

/// This overload doesn't work as advertised in VS22
void operator delete[](void* memoryLocation) {
    if (memoryLocation == NULL) return;
    ArraySize* array = reinterpret_cast<ArraySize*>(memoryLocation) - 1;

    MEMORY_NUMUSEDBYTES -= array->numBytes;

 //   std::cout << "freeing array " << array->numBytes << " bytes of memory\n";
    free(array);
}

#endif
