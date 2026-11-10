/**
 * @file bsd_protocol.h
 * @brief Blind Spot Detection – ECU datagram protocol definition
 *
 * Must match the Python sender exactly:
 *
 *   BSD_OBJ_FMT  = "<IBBxx4f"   (little-endian)
 *
 *   Offset  Size  Field
 *   ------  ----  ────────────────────────────────────────
 *     0      4    actor_id    uint32   (vehicle/walker ID from CARLA)
 *     4      1    obj_type    0=vehicle  1=pedestrian
 *     5      1    side        0=left     1=right
 *     6      2    _pad        (zeros – alignment)
 *     8      4    distance    float32  metres
 *    12      4    rel_vx      float32  m/s  (+ = approaching from rear)
 *    16      4    rel_vy      float32  m/s  (+ = rightward)
 *    20      4    rel_speed   float32  m/s  magnitude
 *   ------  ----
 *   TOTAL   24   bytes  (fits in one Ethernet frame – zero fragmentation)
 */

#ifndef BSD_PROTOCOL_H
#define BSD_PROTOCOL_H

#include <stdint.h>

#define BSD_OBJTYPE_VEHICLE    0u
#define BSD_OBJTYPE_PEDESTRIAN 1u

#define BSD_SIDE_LEFT          0u
#define BSD_SIDE_RIGHT         1u

#define BSD_OBJ_SIZE           24u

#pragma pack(push, 1)   /* force byte-exact packing */

typedef struct
{
    uint32_t actor_id;   /**< CARLA actor id (lower 32 bits)                */
    uint8_t  obj_type;   /**< BSD_OBJTYPE_VEHICLE or BSD_OBJTYPE_PEDESTRIAN */
    uint8_t  side;       /**< BSD_SIDE_LEFT or BSD_SIDE_RIGHT               */
    uint8_t  _pad[2];    /**< reserved  always 0x00 0x00                   */
    float    distance;   /**< distance from ego, metres                     */
    float    rel_vx;     /**< relative longitudinal velocity, m/s           */
    float    rel_vy;     /**< relative lateral velocity, m/s                */
    float    rel_speed;  /**< relative speed magnitude, m/s                 */
} BsdObject;

#pragma pack(pop)

/* compile-time guard: fails if struct layout drifts */
typedef char _bsd_size_check[(sizeof(BsdObject) == BSD_OBJ_SIZE) ? 1 : -1];

#endif /* BSD_PROTOCOL_H */
