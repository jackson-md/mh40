/* Copyright (c) 2022 Qualcomm Technologies International, Ltd. */
/*   %%version */
/**
 * \file
 */

#include "trap_api/trap_api_private.h"


#if TRAPSET_BLUESTACK

Task MessageLinkStatusTrackingTask(Task task)
{
    return trap_api_register_message_task(task, IPC_MSG_TYPE_LINK_STATUS);
}

#endif /* TRAPSET_BLUESTACK */


