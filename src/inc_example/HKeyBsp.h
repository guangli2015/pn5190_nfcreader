/***********************************************************************
 * @file HKeyBsp.h
     HKEYBSP
 * @brief   header file
 * @history
 * Date       Version Author    description
 * ========== ======= ========= =======================================
 * 2024-04-05 V1.0    Joe<joe@u-tec.com>   Create
 *
 * @Copyright (C)  2024  .U-Tec Group Inc. all right reserved
***********************************************************************/
#ifndef __HKEYBSP_H__
#define __HKEYBSP_H__
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
/* Defines */

/* @{ */

/* @} */

/* Data structure's */

/* @{ */
int hkey_bsp_send_ecp(uint8_t *ecp_data, int ecp_len);
int hkey_bsp_exchange(const uint8_t *send, uint16_t send_len, uint8_t *recBuffer, uint16_t *receiveLen);

/* @} */

#ifdef __cplusplus
}
#endif

#endif // __HKEYBSP_H__
