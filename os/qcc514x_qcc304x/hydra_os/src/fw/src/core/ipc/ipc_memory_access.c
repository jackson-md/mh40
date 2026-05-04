/* Copyright (c) 2018 Qualcomm Technologies International, Ltd. */
/*   %%version */
/**
 * \file
 */
#include "ipc/ipc_private.h"


#include <message.h>

void ipc_memory_access_fault_handler(IPC_SIGNAL_ID id, const void *msg)
{
    switch(id)
    {
    case IPC_SIGNAL_ID_MEMORY_ACCESS_FAULT_INFO:
    {
        const IPC_MEMORY_ACCESS_FAULT_INFO *fault_msg = msg;
        
        /* The following strings are matched in Pylib log decoder. */
        L0_DBG_MSG2("IPC: CPU1 memory violation: Access to address 0x%x AccessType %d",
            fault_msg->address,fault_msg->type);
        L0_DBG_MSG1("IPC: at P1 ProgramCounter 0x%08x", fault_msg->pc); 
        
        panic_diatribe(PANIC_CPU1_RESTRICTED_ACCESS, fault_msg->address);
    }

    default:
        L1_DBG_MSG1("ipc_memory_access_fault_handler: unexpected signal ID 0x%x received", id);
        assert(FALSE);
        break;
    }
}
