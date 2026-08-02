/* Copyright 2023 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * Patch file for easy removal when the following issue is resolved:
 * LL Function for checking UCPD RxErr Flag is missing #42
 * https://github.com/STMicroelectronics/STM32CubeG0/issues/42
 */

#include "stm32g0xx_ll_ucpd.h"

#ifndef LL_UCPD_PATCH_H
#define LL_UCPD_PATCH_H

/**
 * @brief  Check if Rx error interrupt
 * @rmtoll SR          RXERR         LL_UCPD_IsActiveFlag_RxErr
 * @param  UCPDx UCPD Instance
 * @retval None
 */
__STATIC_INLINE uint32_t LL_UCPD_IsActiveFlag_RxErr(UCPD_TypeDef const *const UCPDx)
{
	return ((READ_BIT(UCPDx->SR, UCPD_SR_RXERR) == UCPD_SR_RXERR) ? 1UL : 0UL);
}

#endif
