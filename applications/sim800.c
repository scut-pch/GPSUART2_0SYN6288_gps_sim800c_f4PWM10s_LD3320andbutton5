#include <rtthread.h>
#include <board.h>
#include <rtdevice.h>
#include "sim800.h"

// 发送AT命令的函数
void sim800c_send_at_command(const char *cmd)
{
    rt_device_t serial_device = rt_device_find(SAMPLE_UART_NAME);
    if (serial_device == RT_NULL)
    {
        rt_kprintf("Serial device not found!\n");
        return;
    }

    // 打开串口设备，仅启用发送功能
    rt_device_open(serial_device, RT_DEVICE_FLAG_WRONLY);

    rt_kprintf("Sending AT command: %s\n", cmd);  // 输出发送的AT命令
    rt_device_write(serial_device, 0, cmd, rt_strlen(cmd));
}

// 发送短信的函数
void sim800c_send_sms(const char *character, const char *phone_number, const char *message)
{
    // 设置短信格式为文本模式
    sim800c_send_at_command("AT+CMGF=1\r\n");
    rt_thread_mdelay(1000);  // 等待 SIM800C 响应

    // 设置短信格式为纯字母模式
    char cmd[64];
    rt_sprintf(cmd, "AT+CSCS=\"%s\"\r\n", character);
    sim800c_send_at_command(cmd);
    rt_thread_mdelay(1000);

    // 发送电话号码
    rt_sprintf(cmd, "AT+CMGS=\"%s\"\r\n", phone_number);
    sim800c_send_at_command(cmd);
    rt_thread_mdelay(1000);

    // 发送短信内容
    char str[] = {0x1A};  // Ctrl+Z 结束符
    sim800c_send_at_command(message);
    sim800c_send_at_command(str);
    rt_thread_mdelay(1000);
}

