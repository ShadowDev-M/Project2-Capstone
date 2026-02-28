#pragma once
#include <cstddef> // for std::size_t
#include <unordered_map>
#include <iostream>

extern size_t MEMORY_NUMUSEDBYTES;



struct AllocationInfo {
    std::size_t size;
    const char* file;
    int line;
};

inline std::unordered_map<void*, AllocationInfo>& GetAllocMap() {
    static std::unordered_map<void*, AllocationInfo> allocMap;
    return allocMap;
}


inline void ReportLeaks() {
    auto& map = GetAllocMap();

    if (map.empty()) {
        std::cout << "No leaks detected. Total used bytes: "
            << MEMORY_NUMUSEDBYTES << "\n";
        return;
    }

    std::cout << "Memory leaks detected:\n";
    for (const auto& [ptr, info] : map) {
        std::cout << " Leaked block at " << ptr
            << " size " << info.size
            << " bytes, allocated at " << info.file
            << ":" << info.line << "\n";
    }
    std::cout << "Total bytes still in use: " << MEMORY_NUMUSEDBYTES << "\n";
}

