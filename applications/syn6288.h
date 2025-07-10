/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-01-11     chenyouqin       the first version
 */
#ifndef APPLICATIONS_SYN6288_H_
#define APPLICATIONS_SYN6288_H_

typedef unsigned char u8;


void SYN_FrameInfo(u8 Music, u8 *HZdata);
void YS_SYN_Set(u8 *Info_data);


#endif /* APPLICATIONS_SYN6288_H_ */
