/**
 * @file bsd_receiver_task.h
 * @brief Public API for the BSD ECU UDP receiver task.
 */

#ifndef BSD_RECEIVER_TASK_H
#define BSD_RECEIVER_TASK_H

#include "core/net.h"

/**
 * @brief Create and start the BSD UDP receiver RTOS task.
 * Call after netInit() and network bring-up, before the scheduler starts.
 * @return TRUE on success, FALSE on task creation failure.
 */
bool_t bsdReceiverTaskCreate(void);

#endif /* BSD_RECEIVER_TASK_H */
