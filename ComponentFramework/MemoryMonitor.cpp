#include "MemorySize.h"
#include "MemoryMonitor.h"

std::size_t MEMORY_NUMUSEDBYTES = 0; // Defines and initializes the single instance

void* operator new(std::size_t numBytes) {
    //   std::cout << "allocating " << numBytes << " bytes of memory\n";
    MEMORY_NUMUSEDBYTES += numBytes;
    return std::malloc(numBytes);
}

void* operator new(std::size_t numBytes, const char* FILE, int LINE) {
    void* ptr = std::malloc(numBytes);
    GetAllocMap().emplace(ptr, AllocationInfo{ numBytes, FILE, LINE });

    MEMORY_NUMUSEDBYTES += numBytes;
    return ptr;
}


void operator delete(void* memoryLocation, std::size_t numBytes) noexcept {
    //  std::cout << "freeing " << numBytes << " bytes of memory\n";
    if (!GetAllocMap().empty() && GetAllocMap().count(memoryLocation)) {
        GetAllocMap().erase(memoryLocation);
    }

    MEMORY_NUMUSEDBYTES -= numBytes;
    if (MEMORY_NUMUSEDBYTES == 0) {

        std::cout << " e";
    }
    std::free(memoryLocation);
}


void operator delete(void* memoryLocation) noexcept {
    //  std::cout << "freeing " << numBytes << " bytes of memory\n";
    if (!GetAllocMap().empty() && GetAllocMap().count(memoryLocation)) {
        GetAllocMap().erase(memoryLocation);
    }

  
    std::free(memoryLocation);
}


void operator delete(void* memoryLocation, const char* file, int line) noexcept
{
    //  std::cout << "freeing " << numBytes << " bytes of memory\n";
    if (!GetAllocMap().empty() && GetAllocMap().count(memoryLocation)) {
        GetAllocMap().erase(memoryLocation);
    }

    
    std::free(memoryLocation);
}

/// I did a little hack to hide the total number of bytes
/// allocated in the array itself. 
void* operator new[](std::size_t numBytes) {


    // std::cout << "allocating an array of " << numBytes << " bytes of memory\n";
    ArraySize* array = reinterpret_cast<ArraySize*>(std::malloc(numBytes + sizeof(ArraySize)));
    if (array) array->numBytes = numBytes;
    MEMORY_NUMUSEDBYTES += array->numBytes;
    return array + 1;
}

void* operator new[](std::size_t numBytes, const char* FILE, int LINE) {


   
    // std::cout << "allocating an array of " << numBytes << " bytes of memory\n";
    ArraySize* array = reinterpret_cast<ArraySize*>(std::malloc(numBytes + sizeof(ArraySize)));
    if (array) { array->numBytes = numBytes; 
     GetAllocMap().emplace(array, AllocationInfo{ numBytes, FILE, LINE });

    }
    MEMORY_NUMUSEDBYTES += array->numBytes;
    return array + 1;
}

/// This overload doesn't work as advertised in VS22
void operator delete[](void* memoryLocation) noexcept {
    if (memoryLocation == NULL) return;
    ArraySize* array = reinterpret_cast<ArraySize*>(memoryLocation) - 1;

    if (!GetAllocMap().empty() && GetAllocMap().count(array)) {
        GetAllocMap().erase(array);
    }

    MEMORY_NUMUSEDBYTES -= array->numBytes;

    free(array);
}

void operator delete[](void* memoryLocation, const char* file, int line) noexcept
{
    if (memoryLocation == NULL) return;

    ArraySize* array = reinterpret_cast<ArraySize*>(memoryLocation) - 1;

    if (!GetAllocMap().empty() && GetAllocMap().count(array)) {
        GetAllocMap().erase(array);
    }
    MEMORY_NUMUSEDBYTES -= array->numBytes;

    std::free(array);
}