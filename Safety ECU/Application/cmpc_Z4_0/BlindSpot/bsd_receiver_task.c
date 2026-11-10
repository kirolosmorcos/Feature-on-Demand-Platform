/**
 * @file bsd_receiver_task.c
 * @brief Blind Spot Detection – ECU TCP receiver task  (cmpc_Z4_0)
 *
 * Socket 1 – TCP SERVER on BSD_ECU_PORT (8888)
 *   Accepts one persistent connection from the CARLA sender and reads
 *   BsdObject structs back-to-back (BSD_OBJ_SIZE = 24 bytes each).
 *   On every received object the shared alert flags (g_bsdLeft /
 *   g_bsdRight) are updated:
 *     distance < BSD_DISTANCE  →  flag for that side = 1
 *     distance ≥ BSD_DISTANCE  →  flag for that side = 0
 *
 * Socket 2 – TCP CLIENT connecting to the Dashboard (DASH_HOST : DASH_PORT)
 *   A separate RTOS task maintains one persistent TCP connection to the
 *   RPi4 dashboard.  Every BSD_DASH_INTERVAL_MS it formats and sends:
 *
 *       "<bsdLeft>-<bsdRight>\n"
 *
 *   e.g.  "0-1\n"  (left clear, right alert)
 *
 *   If the connection drops the task retries after BSD_DASH_RETRY_MS.
 */

#include <string.h>
#include <stdio.h>

#include "core/net.h"
#include "core/socket.h"
#include "bsd_protocol.h"       /* BsdObject, BSD_OBJ_SIZE */
#include "debug.h"
#include "devkit_mpc5748g.h"

/* ── Socket-1 (CARLA receiver) configuration ────────────────────── */
#define BSD_ECU_PORT          8888u
#define BSD_ACCEPT_BACKLOG    1u
#define BSD_CLIENT_TIMEOUT_MS 5000u

/* ── Socket-2 (Dashboard sender) configuration ──────────────────── */
#define BSD_DASH_HOST         "10.0.0.102"     /* RPi4 dashboard IP  */
#define BSD_DASH_PORT         3001u              /* must match DASH_PORT in Python */
#define BSD_DASH_CONNECT_MS   2000u              /* connect() timeout  */
#define BSD_DASH_INTERVAL_MS  500u               /* send rate ~10 Hz   */
#define BSD_DASH_RETRY_MS     1000u              /* reconnect delay    */

/* ── Alert threshold ────────────────────────────────────────────── */
#define BSD_DISTANCE          4                  /* metres             */

/* ── Receiver task parameters ───────────────────────────────────── */
#define BSD_RX_TASK_STACK     500u
#define BSD_RX_TASK_NAME      "BSD_Rx"

/* ── Dashboard sender task parameters ──────────────────────────── */
#define BSD_DASH_TASK_STACK   500u
#define BSD_DASH_TASK_NAME    "BSD_Dash"

/* ── Shared alert flags (written by Rx task, read by Dash task) ─── */
/* Declared volatile so both tasks see up-to-date values.            */
static volatile uint8_t g_bsdLeft  = 0u;
static volatile uint8_t g_bsdRight = 0u;

static volatile bool_t g_bsdFeatureActive=0;


/* ── Forward declarations ───────────────────────────────────────── */
static void    bsdReceiverTask(void *param);
static void    bsdDashboardTask(void *param);
static error_t bsdReadObject(Socket *sock, BsdObject *out);
void           bsdHandleDetection(const BsdObject *obj);

/* ── Byte-order helpers ─────────────────────────────────────────── */
static float swap_float(float f)
{
    uint32_t x;
    memcpy(&x, &f, 4);
    x = __builtin_bswap32(x);
    memcpy(&f, &x, 4);
    return f;
}

static uint32_t read_u32_le(const uint8_t *p)
{
    return ((uint32_t)p[0])        |
           ((uint32_t)p[1] <<  8)  |
           ((uint32_t)p[2] << 16)  |
           ((uint32_t)p[3] << 24);
}

/* ═══════════════════════════════════════════════════════════════════
 *  PUBLIC API
 * ═══════════════════════════════════════════════════════════════════ */
bool_t bsdReceiverTaskCreate(void)
{
    OsTaskParameters taskParams;

    // added 9/6/2026
    g_bsdFeatureActive=TRUE;
    /* ── Create CARLA receiver task ─────────────────────────────── */
    taskParams           = OS_TASK_DEFAULT_PARAMS;
    taskParams.stackSize = BSD_RX_TASK_STACK;
    taskParams.priority  = OS_TASK_PRIORITY_NORMAL;

    OsTaskId rxId = osCreateTask(BSD_RX_TASK_NAME, bsdReceiverTask,
                                 NULL, &taskParams);
    if(rxId == OS_INVALID_TASK_ID)
    {
        TRACE_ERROR("[BSD] Failed to create receiver task\r\n");
        return FALSE;
    }
    TRACE_INFO("[BSD] Receiver task created – listening on TCP port %u\r\n",
               BSD_ECU_PORT);

    /* ── Create Dashboard sender task ───────────────────────────── */
    taskParams           = OS_TASK_DEFAULT_PARAMS;
    taskParams.stackSize = BSD_DASH_TASK_STACK;
    taskParams.priority  = OS_TASK_PRIORITY_NORMAL;

    OsTaskId dashId = osCreateTask(BSD_DASH_TASK_NAME, bsdDashboardTask,
                                   NULL, &taskParams);
    if(dashId == OS_INVALID_TASK_ID)
    {
        TRACE_ERROR("[BSD] Failed to create dashboard task\r\n");
        return FALSE;
    }
    TRACE_INFO("[BSD] Dashboard task created – target %s:%u\r\n",
               BSD_DASH_HOST, BSD_DASH_PORT);

    return TRUE;
}

/* ═══════════════════════════════════════════════════════════════════
 *  TASK 1 – CARLA TCP receiver
 * ═══════════════════════════════════════════════════════════════════ */
static void bsdReceiverTask(void *param)
{

    error_t  error;
    Socket  *listenSock;
    Socket  *clientSock;
    IpAddr   clientIp;
    uint16_t clientPort;
    BsdObject obj;

    (void)param;

    /* ── Open listen socket ─────────────────────────────────────── */
    listenSock = socketOpen(SOCKET_TYPE_STREAM, SOCKET_IP_PROTO_TCP);
    if(!listenSock)
    {
        TRACE_ERROR("[BSD-Rx] socketOpen failed\r\n");
        osDeleteTask(OS_SELF_TASK_ID);
        return;
    }

    socketSetTimeout(listenSock, 0);   /* non-blocking accept */

    error = socketBind(listenSock, &IP_ADDR_ANY, BSD_ECU_PORT);
    if(error)
    {
        TRACE_ERROR("[BSD-Rx] socketBind failed (%d)\r\n", error);
        socketClose(listenSock);
        osDeleteTask(OS_SELF_TASK_ID);
        return;
    }

    error = socketListen(listenSock, BSD_ACCEPT_BACKLOG);
    if(error)
    {
        TRACE_ERROR("[BSD-Rx] socketListen failed (%d)\r\n", error);
        socketClose(listenSock);
        osDeleteTask(OS_SELF_TASK_ID);
        return;
    }

    TRACE_INFO("[BSD-Rx] TCP server listening on port %u\r\n", BSD_ECU_PORT);

    /* ── Outer accept loop ──────────────────────────────────────── */
    //added 9/6/2026
    while(g_bsdFeatureActive) // To enable when feature is active, and don't run when is disabled
    {
        clientSock = socketAccept(listenSock, &clientIp, &clientPort);
        if(!clientSock)
        {
            osDelayTask(10);
            continue;
        }

        TRACE_INFO("[BSD-Rx] CARLA connected %s:%u\r\n",
                   ipAddrToString(&clientIp, NULL), clientPort);

        socketSetTimeout(clientSock, BSD_CLIENT_TIMEOUT_MS);

        /* ── Inner receive loop: one BsdObject per iteration ───── */
        while(g_bsdFeatureActive) // same
        {
            error = bsdReadObject(clientSock, &obj);

            if(error == NO_ERROR)
            {
                /* Validate field ranges */
                if(obj.obj_type > BSD_OBJTYPE_PEDESTRIAN ||
                   obj.side     > BSD_SIDE_RIGHT)
                {
                    TRACE_WARNING("[BSD-Rx] invalid fields – discarded\r\n");
                    continue;
                }

                /* Fix byte-order (CARLA sends little-endian floats) */
                uint8_t *p   = (uint8_t *)&obj.actor_id;
                obj.actor_id = read_u32_le(p);
                obj.distance = swap_float(obj.distance);
                obj.rel_vx   = swap_float(obj.rel_vx);
                obj.rel_vy   = swap_float(obj.rel_vy);
                obj.rel_speed= swap_float(obj.rel_speed);

                /*
                TRACE_INFO(
                    "[BSD-Rx] id=%-3lu | %s | %-5s | dist=%4.2f |"
                    " vx=%+.2f | vy=%+.2f | spd=%.2f\r\n",
                    (unsigned long)obj.actor_id,
                    obj.obj_type == BSD_OBJTYPE_PEDESTRIAN ? "PED" : "VEH",
                    obj.side     == BSD_SIDE_LEFT          ? "LEFT" : "RIGHT",
                    (double)obj.distance,
                    (double)obj.rel_vx,
                    (double)obj.rel_vy,
                    (double)obj.rel_speed);
                 */

                bsdHandleDetection(&obj);
            }
            else if(error == ERROR_TIMEOUT)
            {
                /* Normal – CARLA may have no detections this interval */
                continue;
            }
            else if(error == ERROR_END_OF_STREAM)
            {
                TRACE_INFO("[BSD-Rx] CARLA closed connection\r\n");
                break;
            }
            else
            {
                TRACE_INFO("[BSD-Rx] read error (%d)\r\n", error);
                break;
            }
        }

        socketClose(clientSock);
        TRACE_INFO("[BSD-Rx] ready for next CARLA connection\r\n");

    }
       socketClose(listenSock);
       TRACE_INFO("[BSD-Rx] Task safely terminating.\r\n");
       osDeleteTask(OS_SELF_TASK_ID);
}

/**
 * @brief Read exactly one BsdObject from the TCP stream.
 *
 * TCP is a byte stream – partial reads are possible, so we use
 * SOCKET_FLAG_WAIT_ALL which loops internally until all BSD_OBJ_SIZE
 * bytes have arrived.
 */
static error_t bsdReadObject(Socket *sock, BsdObject *out)
{
    error_t error;
    size_t  received;
    uint8_t buf[BSD_OBJ_SIZE];

    error = socketReceive(sock, buf, BSD_OBJ_SIZE, &received,
                          SOCKET_FLAG_WAIT_ALL);
    if(error != NO_ERROR)
        return error;

    if(received != BSD_OBJ_SIZE)
        return ERROR_INVALID_LENGTH;

    memcpy(out, buf, BSD_OBJ_SIZE);
    return NO_ERROR;
}

/* ═══════════════════════════════════════════════════════════════════
 *  TASK 2 – Dashboard TCP sender
 *
 *  Connects to the RPi4 dashboard as a TCP client and periodically
 *  sends the string:   "<g_bsdLeft>-<g_bsdRight>\n"
 *  e.g.  "0-1\n"
 *
 *  On any send / connect failure the socket is closed and the task
 *  waits BSD_DASH_RETRY_MS before attempting a fresh connection.
 * ═══════════════════════════════════════════════════════════════════ */
static void bsdDashboardTask(void *param)
{
    error_t  error;
    Socket  *dashSock;
    IpAddr   dashIp;
    char     txBuf[8];   /* "0-0\n" … "1-1\n" – never exceeds 5 bytes */
    size_t   txLen;
    size_t   sent;

    (void)param;

    /* Resolve the dashboard IP string once */
    error = ipStringToAddr(BSD_DASH_HOST, &dashIp);
    if(error)
    {
        TRACE_ERROR("[BSD-Dash] ipStringToAddr(\"%s\") failed (%d)\r\n",
                    BSD_DASH_HOST, error);
        osDeleteTask(OS_SELF_TASK_ID);
        return;
    }

    while(g_bsdFeatureActive)
    {
        /* ── Open a fresh TCP socket ────────────────────────────── */
        dashSock = socketOpen(SOCKET_TYPE_STREAM, SOCKET_IP_PROTO_TCP);
        if(!dashSock)
        {
            TRACE_ERROR("[BSD-Dash] socketOpen failed – retry in %u ms\r\n",
                        BSD_DASH_RETRY_MS);
            osDelayTask(BSD_DASH_RETRY_MS);
            continue;
        }

        socketSetTimeout(dashSock, BSD_DASH_CONNECT_MS);

        error = socketConnect(dashSock, &dashIp, BSD_DASH_PORT);
        if(error)
        {
            TRACE_WARNING("[BSD-Dash] connect to %s:%u failed (%d) –"
                          " retry in %u ms\r\n",
                          BSD_DASH_HOST, BSD_DASH_PORT, error,
                          BSD_DASH_RETRY_MS);
            socketClose(dashSock);
            osDelayTask(BSD_DASH_RETRY_MS);
            continue;
        }

        /* Switch to per-send timeout after connect */
        socketSetTimeout(dashSock, BSD_DASH_INTERVAL_MS * 2u);

        TRACE_INFO("[BSD-Dash] connected to dashboard %s:%u\r\n",
                   BSD_DASH_HOST, BSD_DASH_PORT);

        /* ── Periodic send loop ─────────────────────────────────── */
        while(g_bsdFeatureActive)
        {
            /* Snapshot shared flags (volatile read – no mutex needed
             * for single-byte atomics on this platform)              */
            uint8_t left  = g_bsdLeft;
            uint8_t right = g_bsdRight;

            /* Build  "bsdLeft-bsdRight\n"  e.g. "0-1\n"             */
            txLen = (size_t)snprintf(txBuf, sizeof(txBuf),
                                     "%u-%u\n", left, right);

            error = socketSend(dashSock, txBuf, txLen, &sent, 0);
            if(error != NO_ERROR)
            {
                TRACE_WARNING("[BSD-Dash] send error (%d) – reconnecting\r\n",
                              error);
                break;   /* fall through to socketClose + reconnect   */
            }

            TRACE_INFO("[BSD-Dash] sent \"%u-%u\\n\"\r\n", left, right);

            osDelayTask(BSD_DASH_INTERVAL_MS);
        }

        socketClose(dashSock);
        TRACE_INFO("[BSD-Dash] socket closed – reconnecting in %u ms\r\n",
                   BSD_DASH_RETRY_MS);
        osDelayTask(BSD_DASH_RETRY_MS);
    }
    /* ── CLEANUP AND SELF-DELETION ──────────────────────────────── */
        TRACE_INFO("[BSD-Dash] Task safely terminating.\r\n");
        osDeleteTask(OS_SELF_TASK_ID);
}

/* ═══════════════════════════════════════════════════════════════════
 *  Application handler
 *
 *  Updates the shared alert flags based on distance threshold:
 *    distance <  BSD_DISTANCE  →  flag for that side = 1  (alert)
 *    distance >= BSD_DISTANCE  →  flag for that side = 0  (clear)
 * ═══════════════════════════════════════════════════════════════════ */
void bsdHandleDetection(const BsdObject *obj)
{
    uint8_t alert = ((double)obj->distance < (double)BSD_DISTANCE) ? 1u : 0u;

    if(obj->side == BSD_SIDE_LEFT)
    {
        g_bsdLeft = alert;
        TRACE_INFO("[BSD] LEFT  alert=%u  dist=%.2f\r\n",
                   alert, (double)obj->distance);
    }
    else
    {
        g_bsdRight = alert;
        TRACE_INFO("[BSD] RIGHT alert=%u  dist=%.2f\r\n",
                   alert, (double)obj->distance);
    }
}
/* ═══════════════════════════════════════════════════════════════════
 * Deactivate Application
 * ═══════════════════════════════════════════════════════════════════ */
void bsdReceiverTaskDelete(void)
{
    TRACE_INFO("[BSD] Deactivating feature, signaling tasks to exit...\r\n");
    g_bsdFeatureActive = FALSE;
}
