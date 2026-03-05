#pragma once
#include "MemoryMonitor.h"

#ifdef _DEBUG
    #define DEBUG_NEW new(__FILE__, __LINE__)
    #define new DEBUG_NEW
    
    #define RECORD RecordNextUndefinedNew(__FILE__, __LINE__);

#endif


