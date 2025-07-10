/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-01-15     Lenovo       the first version
 */
#ifndef APPLICATIONS_SIM800_H_
#define APPLICATIONS_SIM800_H_

#define SAMPLE_UART_NAME "uart2" /* 串口设备名称 */

void sim800c_init(void);
void sim800c_send_at_command(const char *cmd);
void sim800c_send_sms(const char *character,const char *phone_number, const char *message);

#endif /* APPLICATIONS_SIM800_H_ */
