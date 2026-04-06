#include "MemorySize.h"
#include "MemoryMonitor.h"
#include <mutex>
#include "Component.h"
std::size_t MEMORY_NUMUSEDBYTES = 0; 




//Anti recursion stuff
static std::mutex allocMutex;
static thread_local bool is_main_thread = false;
static std::atomic<std::thread::id> main_thread_id{ std::this_thread::get_id() };

//For RECORD 
static std::pair<const char*, int> nextPtrData;

bool allocMapDestroyed = false;

void RecordNextUndefinedNew(const char* file, int line) {
    nextPtrData = { file, line };
}

void* operator new(std::size_t numBytes) {
    void* ptr = std::malloc(numBytes);
    if (!ptr) return nullptr;

    

    static thread_local bool recursing = false;

    //Block recursion and also block recording from non-main threads as this isn't really thread safe
    if (!recursing && is_main_thread && nextPtrData.second > 0) {  
        recursing = true;
        std::lock_guard<std::mutex> lock(allocMutex);

        GetAllocMap()[ptr] = { numBytes, nextPtrData.first, nextPtrData.second };
        nextPtrData = {};
        
        MEMORY_NUMUSEDBYTES += numBytes;

        recursing = false;
    }

    return ptr;
}


void* operator new(std::size_t numBytes, const char* FILE, int LINE) {
    void* ptr = std::malloc(numBytes);

    auto& map = GetAllocMap();
    map.insert({ ptr, AllocationInfo{numBytes, FILE, LINE} });



    MEMORY_NUMUSEDBYTES += numBytes;
    return ptr;
}

void operator delete(void* memoryLocation, std::size_t numBytes) noexcept {
    if (!allocMapDestroyed) { // SINGLE call
        if (GetAllocMap().erase(memoryLocation)) { // No count() needed - erase safe on missing keys
            MEMORY_NUMUSEDBYTES -= numBytes;
        }
    }
    std::free(memoryLocation);
}

void operator delete(void* memoryLocation) noexcept {
    //  std::cout << "freeing " << numBytes << " bytes of memory\n";
    if (!allocMapDestroyed) GetAllocMap().erase(memoryLocation);
    std::free(memoryLocation);
}


void operator delete(void* memoryLocation, const char* file, int line) noexcept
{
    //  std::cout << "freeing " << numBytes << " bytes of memory\n";
    if (!allocMapDestroyed) GetAllocMap().erase(memoryLocation);
    std::free(memoryLocation);
}

/// I did a little hack to hide the total number of bytes
/// allocated in the array itself. 
void* operator new[](std::size_t numBytes) {


    ArraySize* array = reinterpret_cast<ArraySize*>(std::malloc(numBytes + sizeof(ArraySize)));
    if (array) {
        array->numBytes = numBytes;

        if (is_main_thread == false && main_thread_id.load() == std::this_thread::get_id()) {
            is_main_thread = true;
        }
        
        //not thread safe so block all threads except main thread from being recorded
        if (is_main_thread && nextPtrData.second > 0) {
            auto& map = GetAllocMap();

            map.insert({ array, AllocationInfo{numBytes, nextPtrData.first, nextPtrData.second} });
            nextPtrData = {};
            MEMORY_NUMUSEDBYTES += array->numBytes;

        }

    }
    return array + 1;
}

void* operator new[](std::size_t numBytes, const char* FILE, int LINE) {


   
    ArraySize* array = reinterpret_cast<ArraySize*>(std::malloc(numBytes + sizeof(ArraySize)));
    if (array) { array->numBytes = numBytes; 

    auto& map = GetAllocMap();
    map.insert({ array, AllocationInfo{numBytes, FILE, LINE} });

    }
    MEMORY_NUMUSEDBYTES += array->numBytes;
    return array + 1;
}

/// This overload doesn't work as advertised in VS22
void operator delete[](void* memoryLocation) noexcept {
    if (memoryLocation == NULL) return;
    ArraySize* array = reinterpret_cast<ArraySize*>(memoryLocation) - 1;
    if (!allocMapDestroyed) {
        if (GetAllocMap().erase(array))
            MEMORY_NUMUSEDBYTES -= array->numBytes;
    }
    free(array);
}

void operator delete[](void* memoryLocation, const char* file, int line) noexcept
{
    if (memoryLocation == NULL) return;
    ArraySize* array = reinterpret_cast<ArraySize*>(memoryLocation) - 1;
    if (!allocMapDestroyed) {
        GetAllocMap().erase(array);
        MEMORY_NUMUSEDBYTES -= array->numBytes;
    }
    std::free(array);
}