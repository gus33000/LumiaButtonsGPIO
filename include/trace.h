#include <wdm.h>

#define Trace(Level, Flags, Msg, ...) \
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ST: " Msg "\n", __VA_ARGS__);

#define TRACE_FLAG_INIT        1  
#define TRACE_FLAG_REGISTRY    2  
#define TRACE_FLAG_HID         3  
#define TRACE_FLAG_PNP         4  
#define TRACE_FLAG_POWER       5  
#define TRACE_FLAG_SPB         6  
#define TRACE_FLAG_CONFIG      7  
#define TRACE_FLAG_REPORTING   8  
#define TRACE_FLAG_INTERRUPT   9  
#define TRACE_FLAG_SAMPLES     10  
#define TRACE_FLAG_OTHER       11  
#define TRACE_FLAG_IDLE		   12

#define TRACE_LEVEL_ERROR       1
#define TRACE_LEVEL_VERBOSE     2
#define TRACE_LEVEL_INFORMATION 4
#define TRACE_LEVEL_WARNING     3