#include <assert.h>

#define NRF_LOG_MODULE_NAME     fault
#define NRF_LOG_LEVEL           4
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#include "Fault.h"

//----------------------------------------------------------------------------
// FaultHandler
//----------------------------------------------------------------------------
void FaultHandler(const char* file, unsigned short line)
{
    NRF_LOG_ERROR("Assert failed: %s:%d\n", file, line);
    assert(0);
}
