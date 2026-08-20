#pragma once

/* STM32 target configuration: error information lives in a static application
 * buffer, never in malloc/free managed memory. */
#define USE_FULL_ERROR_LIST 1
#define USE_USER_ERROR_LIST 0
#define USE_DEVICE_DEPENDENT_ERROR_INFORMATION 1
#define USE_MEMORY_ALLOCATION_FREE 0
#define USE_CUSTOM_DTOSTRE 1
#define SCPI_LINE_ENDING "\r\n"
