/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-02-03     RT-Thread    first version
 */

#include <rtthread.h>
#include <board.h>
#include <rtdevice.h>
#define DBG_TAG "main"
#define DBG_LVL         DBG_LOG
#include <rtdbg.h>
#include "syn6288.h"
#include <string.h>  // 包含标准库的字符串函数
#include"sim800.h"
#include <drv_common.h>
#include "pwm.h"

#define PIN_PIN1    GET_PIN(A, 3) //不需要真正启用，因为它是串口引脚
#define PIN_KEY0    GET_PIN(A, 10) //按键控制PWM波输出
#define PIN_KEY1    GET_PIN(A, 5) //语音控制PWM波输出
#define PIN_KEY2    GET_PIN(A, 6) //语音控制gps_sim800c定位发送
#define PIN_KEY3    GET_PIN(A, 7) //语音控制报警，即SYN6288输出语音
#define PIN_KEY4    GET_PIN(G, 1) //按键控制gps_sim800c定位发送
#define PIN_KEY5    GET_PIN(A, 9) //按键控制报警，即SYN6288输出语音

#define SAMPLE_UART_NAME "uart2" /* 串口设备名称 */
/* 串口设备句柄 */
static rt_device_t serial;
/* 消息队列控制块 */
static struct rt_messagequeue rx_mq;
/* 串口接收消息结构*/
struct rx_msg
{
    rt_device_t dev;
    rt_size_t size;
};

/* 定义 PWM 设备句柄 */
struct rt_device_pwm *pwm_dev;

/* 接收数据回调函数 */
static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    struct rx_msg msg;
    rt_err_t result;
    msg.dev = dev;
    msg.size = size;

    result = rt_mq_send(&rx_mq, &msg, sizeof(msg));
    if ( result == -RT_EFULL)
    {
        /* 消息队列满 */
        rt_kprintf("message queue full！\n");
    }
    return result;
}

///* SYN6288 的线程函数 */
//static void thread_syn6288_entry(void *parameter)
//{
//    char str[] = {0xB3, 0xC2, 0xEC, 0xDA, 0xB4, 0xF3, 0xC3, 0xC0, 0xC5, 0xAE}; // GBK 编码"陈熠大美女"
//
//    while (1)
//    {
//        SYN_FrameInfo(0, str);
//
//        for (int i = 0; i < 3; i++)
//        {
//            rt_thread_mdelay(1500);
//        }
//    }
//}

///* 语音控制 SYN6288 功能（类似音乐的播放和赞停）*//*语音:A7输入*/
//static rt_bool_t is_running = RT_FALSE;  // 播放控制标志
//static rt_mutex_t state_mutex = RT_NULL;  // 保护标志的互斥量
//
//// 播放线程函数
//static void play_function(void *parameter)
//{
//    char str[] = {0xB4, 0xF2, 0xD1, 0xFD, 0xD1, 0xFD, 0xC1, 0xE9, 0x20}; //0xB0, 0xEF, 0xCE, 0xD2, 0xB1, 0xA8, 0xBE, 0xAF, 0x20 GBK 编码"帮我报警"
//
//    while (1)
//    {
//        rt_mutex_take(state_mutex, RT_WAITING_FOREVER);  // 加锁检查状态
//        rt_bool_t local_running = is_running;  // 读取标志状态
//        rt_mutex_release(state_mutex);  // 解锁
//
//        if (local_running)
//        {
//            SYN_FrameInfo(0, str);  // 调用播放函数
//            rt_thread_mdelay(1800);  // 播放间隔
//        }
//        else
//        {
//            rt_thread_mdelay(12);  // 未播放时短延时循环
//        }
//    }
//}
//
//// 按键线程函数
//static void key_thread_entry(void *parameter)
//{
//    rt_pin_mode(PIN_KEY3, PIN_MODE_INPUT_PULLDOWN);  // 设置按键输入引脚
//    rt_bool_t key_last_state = RT_FALSE;  // 上一次按键状态
//
//    while (1)
//    {
//        rt_bool_t key_state = rt_pin_read(PIN_KEY3);  // 读取按键状态
//
//        // 检测按键从低电平到高电平的变化
//        if (key_state == PIN_HIGH && key_last_state == PIN_LOW)
//        {
//            rt_thread_mdelay(30);  // 消抖
//            if (rt_pin_read(PIN_KEY3) == PIN_HIGH)  // 再次确认按键
//            {
//                // 切换播放标志
//                rt_mutex_take(state_mutex, RT_WAITING_FOREVER);  // 加锁
//                is_running = !is_running;  // 切换状态
//                rt_mutex_release(state_mutex);  // 解锁
//            }
//        }
//
//        key_last_state = key_state;  // 更新按键状态
//        rt_thread_mdelay(20);  // 主循环延时
//    }
//}

/* 按键控制 SYN6288 功能（类似音乐的播放和赞停）*//*按键:A9输入*/
static rt_bool_t is_running1 = RT_FALSE;  // 播放控制标志
static rt_mutex_t state_mutex1 = RT_NULL;  // 保护标志的互斥量

// 播放线程函数1
static void play_function1(void *parameter)
{
    char str[] = {0xB4, 0xF2, 0xD1, 0xFD, 0xD1, 0xFD, 0xC1, 0xE9, 0x20}; //0xB3, 0xC2, 0xEC, 0xDA, 0xB4, 0xF3, 0xC3, 0xC0, 0xC5, 0xAE GBK 编码"陈熠大美女"

    while (1)
    {
        rt_mutex_take(state_mutex1, RT_WAITING_FOREVER);  // 加锁检查状态
        rt_bool_t local_running = is_running1;  // 读取标志状态
        rt_mutex_release(state_mutex1);  // 解锁

        if (local_running)
        {
            SYN_FrameInfo(0, str);  // 调用播放函数
            rt_thread_mdelay(1800);  // 播放间隔
        }
        else
        {
            rt_thread_mdelay(12);  // 未播放时短延时循环
        }
    }
}

// 按键线程函数1
static void key_thread_entry1(void *parameter)
{
    rt_pin_mode(PIN_KEY5, PIN_MODE_INPUT_PULLDOWN);  // 设置按键输入引脚
    rt_bool_t key_last_state = RT_FALSE;  // 上一次按键状态

    while (1)
    {
        rt_bool_t key_state = rt_pin_read(PIN_KEY5);  // 读取按键状态

        // 检测按键从低电平到高电平的变化
        if (key_state == PIN_HIGH && key_last_state == PIN_LOW)
        {
            rt_thread_mdelay(30);  // 消抖
            if (rt_pin_read(PIN_KEY5) == PIN_HIGH)  // 再次确认按键
            {
                // 切换播放标志
                rt_mutex_take(state_mutex1, RT_WAITING_FOREVER);  // 加锁
                is_running1 = !is_running1;  // 切换状态
                rt_mutex_release(state_mutex1);  // 解锁
            }
        }

        key_last_state = key_state;  // 更新按键状态
        rt_thread_mdelay(20);  // 主循环延时
    }
}



///*gps的serial 线程函数*/
//static void serial_thread_entry(void *parameter)
//{
//    struct rx_msg msg;
//       rt_err_t result;
//       rt_uint32_t rx_length;
//       static char rx_buffer[RT_SERIAL_RB_BUFSZ + 1];
//       int timeout = 5000;  // 可以设定秒的等待时间
//       int max_lines = 11;  // 最大接收11行数据
//       int line_count = 0;   // 当前接收到的行数
//       // 在主函数中调用发送短信的函数
//       const char *character = "GSM";  // 替换为目标电话号码
//       const char *phone_number = "18126845556";  // 替换为目标电话号码
//       char message[60] = "Help!!! My location is ";
//
//    while (line_count < max_lines)  // 接收最多11行数据
//    {
//        // 等待5秒钟
//        rt_thread_mdelay(timeout);  // 延时5秒
//
//        rt_memset(&msg, 0, sizeof(msg));
//        /* 从消息队列中读取消息*/
//        result = rt_mq_recv(&rx_mq, &msg, sizeof(msg), 0);  // 立即返回，不等待
//
//        if (result == RT_EOK)  // 如果接收到消息
//        {
//            /* 从串口读取数据*/
//            rx_length = rt_device_read(msg.dev, 0, rx_buffer, msg.size);
//            rx_buffer[rx_length] = '\0';  // 确保字符串以 null 结尾
//
//            if (rx_length > 0)
//            {
//                // 遍历多行输入数据
//                char *line = rx_buffer;
//                while (line != NULL && *line != '\0')
//                {
//                    // 查找以 $GNRMC 开头的行
//                    if (rt_strncmp(line, "$GNRMC", 6) == 0)
//                    {
//                        // 提取经纬度部分
//                        char *start = line;
//                        int comma_count = 0;
//                        char filtered_data[50] = {0};
//                        int filtered_index = 0;
//
//                        // 遍历字符串，寻找目标经纬度部分
//                        while (*start != '\0')
//                        {
//                            if (*start == ',')
//                            {
//                                comma_count++;
//
//                                // 纬度
//                                if (comma_count == 3)
//                                {
//                                    start++; // 跳过逗号
//                                    while (*start != ',' && *start != '\0')
//                                    {
//                                        filtered_data[filtered_index++] = *start;
//                                        start++;
//                                    }
//                                    filtered_data[filtered_index++] = ','; // 添加分隔符
//                                    continue;
//                                }
//
//                                // 纬度方向
//                                if (comma_count == 4)
//                                {
//                                    start++; // 跳过逗号
//                                    filtered_data[filtered_index++] = *start;
//                                    filtered_data[filtered_index++] = ','; // 添加分隔符
//                                    continue;
//                                }
//
//                                // 经度
//                                if (comma_count == 5)
//                                {
//                                    start++; // 跳过逗号
//                                    while (*start != ',' && *start != '\0')
//                                    {
//                                        filtered_data[filtered_index++] = *start;
//                                        start++;
//                                    }
//                                    filtered_data[filtered_index++] = ','; // 添加分隔符
//                                    continue;
//                                }
//
//                                // 经度方向
//                                if (comma_count == 6)
//                                {
//                                    start++; // 跳过逗号
//                                    filtered_data[filtered_index++] = *start;
//                                    filtered_data[filtered_index] = '\0'; // 结束字符串
//                                    break;
//                                }
//                            }
//                            start++;
//                        }
//
//                        // 如果成功提取到目标内容，输出到串口
//                                               if (filtered_data[0] != '\0')
//                                               {
//                                                   // 使用 rt_strncat 追加
//                                                   strncat(message, filtered_data, sizeof(message) - strlen(message) - 1);
//                                                   rt_kprintf("Result: %s\n", message);
//
//                                                  // 调用发送短信的函数
//                                                  sim800c_send_sms(character,phone_number, message);
//                                                  rt_kprintf("This is a test command: Hello, RT-Thread!\n");
//
//                                                   //rt_kprintf("Filtered data: %s\n", filtered_data);
//                                                   //rt_device_write(serial, 0, filtered_data, rt_strlen(filtered_data));
//                        }
//                    }
//
//                    // 查找下一行数据
//                    line = strchr(line, '\n');
//                    if (line != NULL) line++;
//                    line_count++;  // 每接收到一行数据，行数加一
//                    if (line_count >= max_lines) break;  // 如果已经接收了11行数据，跳出循环
//                }
//            }
//        }
//        else
//        {
//            // 如果没有接收到数据，可以选择打印日志
//            rt_kprintf("No GPS data received after waiting for 5 seconds\n");
//        }
//    }
//
//    // 处理完成后退出
//    rt_kprintf("Finished processing 11 lines of data.\n");
//}


/* 定义 PWM 线程函数 *//*按键:A10输入*/
static void pwm_thread_entry(void *parameter)
{
    rt_uint32_t freq = 108000;  // PWM 频率
    rt_uint32_t duty = 47.25;   // PWM 占空比

    /* 查找 PWM 设备 */
    pwm_dev = (struct rt_device_pwm *)rt_device_find(PWM_DEV_NAME);
    if (pwm_dev == RT_NULL)
    {
        rt_kprintf("Cannot find PWM device: %s\n", PWM_DEV_NAME);
        return;
    }

    /* 设置按键和输出引脚 */
    //rt_pin_mode(PIN_PIN1, PIN_MODE_OUTPUT);          // 设置输出引脚
    rt_pin_mode(PIN_KEY0, PIN_MODE_INPUT_PULLDOWN);  // 设置按键输入引脚

    while (1)
    {
        /* 检查按键是否按下 */
        if (rt_pin_read(PIN_KEY0) == PIN_HIGH)// || rt_pin_read(PIN_KEY1) == PIN_HIGH
        {
            rt_thread_mdelay(30);  // 消抖
            if (rt_pin_read(PIN_KEY0) == PIN_HIGH )//|| rt_pin_read(PIN_KEY1) == PIN_HIGH
            {
                /* 按键按下，设置 PWM 参数 */
                set_pwm_param(freq, duty);
                rt_thread_mdelay(10000);  // 延时 10 秒
                inset_pwm_param(freq, duty);  // 关闭 PWM
            }
        }
        else
        {
            //rt_pin_write(PIN_PIN1, PIN_LOW);  // 熄灭 LED
        }

        rt_thread_mdelay(10);  // 主循环延时
    }
}

///* 定义 PWM 线程函数 *//*语音:A5输入*/
//static void pwm_thread_entry1(void *parameter)
//{
//    rt_uint32_t freq = 108000;  // PWM 频率
//    rt_uint32_t duty = 47.25;   // PWM 占空比
//
//    /* 查找 PWM 设备 */
//    pwm_dev = (struct rt_device_pwm *)rt_device_find(PWM_DEV_NAME);
//    if (pwm_dev == RT_NULL)
//    {
//        rt_kprintf("Cannot find PWM device: %s\n", PWM_DEV_NAME);
//        return;
//    }
//
//    /* 设置按键和输出引脚 */
//    //rt_pin_mode(PIN_PIN1, PIN_MODE_OUTPUT);          // 设置输出引脚
//    rt_pin_mode(PIN_KEY1, PIN_MODE_INPUT_PULLDOWN);  // 设置按键输入引脚
//
//    while (1)
//    {
//        /* 检查A5是否为高电平 ，LD3320为P1.7输出高电平*/
//        if (rt_pin_read(PIN_KEY1) == PIN_HIGH)
//        {
//            rt_thread_mdelay(30);  // 消抖
//            if (rt_pin_read(PIN_KEY0) == PIN_HIGH)
//            {
//            /* 按键按下，设置 PWM 参数 */
//            set_pwm_param(freq, duty);
//            rt_thread_mdelay(10000);  // 延时 10 秒
//            inset_pwm_param(freq, duty);  // 关闭 PWM
//            }
//        }
//        else
//        {
//            //rt_pin_write(PIN_PIN1, PIN_LOW);  // 熄灭 LED
//        }
//
//        rt_thread_mdelay(10);  // 主循环延时
//    }
//}


/* 定义 gps_sim 线程函数 *//*语音:A6输入*/
void serial_thread_entry(void *parameter)
{
    /* 设置按键和输出引脚 */
    //rt_pin_mode(PIN_PIN1, PIN_MODE_OUTPUT);          // 设置输出引脚
    rt_pin_mode(PIN_KEY2, PIN_MODE_INPUT_PULLDOWN);  // 设置按键输入引脚
    rt_pin_mode(PIN_KEY4, PIN_MODE_INPUT_PULLDOWN);  // 设置按键输入引脚

    struct rx_msg msg;
       rt_err_t result;
       rt_uint32_t rx_length;
       static char rx_buffer[RT_SERIAL_RB_BUFSZ + 1];
       int timeout = 5000;  // 可以设定秒的等待时间
       int max_lines = 11;  // 最大接收11行数据
       int line_count = 0;   // 当前接收到的行数
       // 在主函数中调用发送短信的函数
       const char *character = "GSM";  // 替换为目标电话号码
       const char *phone_number = "18126845556";  // 替换为目标电话号码
       char message[60] = "Help!!! My location is ";

    while (1)
    {
        /* 检查A6是否为高电平 ，LD3320为P1.6输出高电平 && rt_pin_read(PIN_KEY4) == PIN_HIGH*/
        if (rt_pin_read(PIN_KEY4) == PIN_HIGH) //rt_pin_read(PIN_KEY2) == PIN_HIGH ||
        {
            rt_thread_mdelay(30);  // 消抖
            if (rt_pin_read(PIN_KEY4) == PIN_HIGH) //rt_pin_read(PIN_KEY2) == PIN_HIGH ||
            {
            //执行gps_sim的部分
                while (line_count < max_lines)  // 接收最多11行数据
                {
                    // 等待5秒钟
                    rt_thread_mdelay(timeout);  // 延时5秒

                    rt_memset(&msg, 0, sizeof(msg));
                    /* 从消息队列中读取消息*/
                    result = rt_mq_recv(&rx_mq, &msg, sizeof(msg), 0);  // 立即返回，不等待

                    if (result == RT_EOK)  // 如果接收到消息
                    {
                        /* 从串口读取数据*/
                        rx_length = rt_device_read(msg.dev, 0, rx_buffer, msg.size);
                        rx_buffer[rx_length] = '\0';  // 确保字符串以 null 结尾

                        if (rx_length > 0)
                        {
                            // 遍历多行输入数据
                            char *line = rx_buffer;
                            while (line != NULL && *line != '\0')
                            {
                                // 查找以 $GNRMC 开头的行
                                if (rt_strncmp(line, "$GNRMC", 6) == 0)
                                {
                                    // 提取经纬度部分
                                    char *start = line;
                                    int comma_count = 0;
                                    char filtered_data[50] = {0};
                                    int filtered_index = 0;

                                    // 遍历字符串，寻找目标经纬度部分
                                    while (*start != '\0')
                                    {
                                        if (*start == ',')
                                        {
                                            comma_count++;

                                            // 纬度
                                            if (comma_count == 3)
                                            {
                                                start++; // 跳过逗号
                                                while (*start != ',' && *start != '\0')
                                                {
                                                    filtered_data[filtered_index++] = *start;
                                                    start++;
                                                }
                                                filtered_data[filtered_index++] = ','; // 添加分隔符
                                                continue;
                                            }

                                            // 纬度方向
                                            if (comma_count == 4)
                                            {
                                                start++; // 跳过逗号
                                                filtered_data[filtered_index++] = *start;
                                                filtered_data[filtered_index++] = ','; // 添加分隔符
                                                continue;
                                            }

                                            // 经度
                                            if (comma_count == 5)
                                            {
                                                start++; // 跳过逗号
                                                while (*start != ',' && *start != '\0')
                                                {
                                                    filtered_data[filtered_index++] = *start;
                                                    start++;
                                                }
                                                filtered_data[filtered_index++] = ','; // 添加分隔符
                                                continue;
                                            }

                                            // 经度方向
                                            if (comma_count == 6)
                                            {
                                                start++; // 跳过逗号
                                                filtered_data[filtered_index++] = *start;
                                                filtered_data[filtered_index] = '\0'; // 结束字符串
                                                break;
                                            }
                                        }
                                        start++;
                                    }

                                    // 如果成功提取到目标内容，输出到串口
                                                           if (filtered_data[0] != '\0')
                                                           {
                                                               // 使用 rt_strncat 追加
                                                               strncat(message, filtered_data, sizeof(message) - strlen(message) - 1);
                                                               rt_kprintf("Result: %s\n", message);

                                                              // 调用发送短信的函数
                                                              sim800c_send_sms(character,phone_number, message);
                                                              rt_kprintf("This is a test command: Hello, RT-Thread!\n");

                                                               //rt_kprintf("Filtered data: %s\n", filtered_data);
                                                               //rt_device_write(serial, 0, filtered_data, rt_strlen(filtered_data));
                                    }
                                }

                                // 查找下一行数据
                                line = strchr(line, '\n');
                                if (line != NULL) line++;
                                line_count++;  // 每接收到一行数据，行数加一
                                if (line_count >= max_lines) break;  // 如果已经接收了11行数据，跳出循环
                            }
                        }
                    }
                    else
                    {
                        // 如果没有接收到数据，可以选择打印日志
                        rt_kprintf("No GPS data received after waiting for 5 seconds\n");
                    }
                }
            }

                // 处理完成后退出
                rt_kprintf("Finished processing 11 lines of data.\n");

        }
        else
        {
            //rt_pin_write(PIN_PIN1, PIN_LOW);  // 熄灭 LED
        }

        rt_thread_mdelay(10);  // 主循环延时
    }
}

///* 定义 gps_sim 线程函数 *//*按键:A8输入*/
//void serial_thread_entry1(void *parameter)
//{
//    /* 设置按键和输出引脚 */
//    //rt_pin_mode(PIN_PIN1, PIN_MODE_OUTPUT);          // 设置输出引脚
//    rt_pin_mode(PIN_KEY4, PIN_MODE_INPUT_PULLDOWN);  // 设置按键输入引脚
//
//    struct rx_msg msg;
//       rt_err_t result;
//       rt_uint32_t rx_length;
//       static char rx_buffer[RT_SERIAL_RB_BUFSZ + 1];
//       int timeout = 5000;  // 可以设定秒的等待时间
//       int max_lines = 11;  // 最大接收11行数据
//       int line_count = 0;   // 当前接收到的行数
//       // 在主函数中调用发送短信的函数
//       const char *character = "GSM";  // 替换为目标电话号码
//       const char *phone_number = "18126845556";  // 替换为目标电话号码
//       char message[60] = "Help!!! My location is ";
//
//    while (1)
//    {
//        /* 检查A8是否为高电平 */
//        if (rt_pin_read(PIN_KEY4) == PIN_HIGH)
//        {
//            rt_thread_mdelay(30);  // 消抖
//            if (rt_pin_read(PIN_KEY4) == PIN_HIGH)
//            {
//            //执行gps_sim的部分
//                while (line_count < max_lines)  // 接收最多11行数据
//                {
//                    // 等待5秒钟
//                    rt_thread_mdelay(timeout);  // 延时5秒
//
//                    rt_memset(&msg, 0, sizeof(msg));
//                    /* 从消息队列中读取消息*/
//                    result = rt_mq_recv(&rx_mq, &msg, sizeof(msg), 0);  // 立即返回，不等待
//
//                    if (result == RT_EOK)  // 如果接收到消息
//                    {
//                        /* 从串口读取数据*/
//                        rx_length = rt_device_read(msg.dev, 0, rx_buffer, msg.size);
//                        rx_buffer[rx_length] = '\0';  // 确保字符串以 null 结尾
//
//                        if (rx_length > 0)
//                        {
//                            // 遍历多行输入数据
//                            char *line = rx_buffer;
//                            while (line != NULL && *line != '\0')
//                            {
//                                // 查找以 $GNRMC 开头的行
//                                if (rt_strncmp(line, "$GNRMC", 6) == 0)
//                                {
//                                    // 提取经纬度部分
//                                    char *start = line;
//                                    int comma_count = 0;
//                                    char filtered_data[50] = {0};
//                                    int filtered_index = 0;
//
//                                    // 遍历字符串，寻找目标经纬度部分
//                                    while (*start != '\0')
//                                    {
//                                        if (*start == ',')
//                                        {
//                                            comma_count++;
//
//                                            // 纬度
//                                            if (comma_count == 3)
//                                            {
//                                                start++; // 跳过逗号
//                                                while (*start != ',' && *start != '\0')
//                                                {
//                                                    filtered_data[filtered_index++] = *start;
//                                                    start++;
//                                                }
//                                                filtered_data[filtered_index++] = ','; // 添加分隔符
//                                                continue;
//                                            }
//
//                                            // 纬度方向
//                                            if (comma_count == 4)
//                                            {
//                                                start++; // 跳过逗号
//                                                filtered_data[filtered_index++] = *start;
//                                                filtered_data[filtered_index++] = ','; // 添加分隔符
//                                                continue;
//                                            }
//
//                                            // 经度
//                                            if (comma_count == 5)
//                                            {
//                                                start++; // 跳过逗号
//                                                while (*start != ',' && *start != '\0')
//                                                {
//                                                    filtered_data[filtered_index++] = *start;
//                                                    start++;
//                                                }
//                                                filtered_data[filtered_index++] = ','; // 添加分隔符
//                                                continue;
//                                            }
//
//                                            // 经度方向
//                                            if (comma_count == 6)
//                                            {
//                                                start++; // 跳过逗号
//                                                filtered_data[filtered_index++] = *start;
//                                                filtered_data[filtered_index] = '\0'; // 结束字符串
//                                                break;
//                                            }
//                                        }
//                                        start++;
//                                    }
//
//                                    // 如果成功提取到目标内容，输出到串口
//                                                           if (filtered_data[0] != '\0')
//                                                           {
//                                                               // 使用 rt_strncat 追加
//                                                               strncat(message, filtered_data, sizeof(message) - strlen(message) - 1);
//                                                               rt_kprintf("Result: %s\n", message);
//
//                                                              // 调用发送短信的函数
//                                                              sim800c_send_sms(character,phone_number, message);
//                                                              rt_kprintf("This is a test command: Hello, RT-Thread!\n");
//
//                                                               //rt_kprintf("Filtered data: %s\n", filtered_data);
//                                                               //rt_device_write(serial, 0, filtered_data, rt_strlen(filtered_data));
//                                    }
//                                }
//
//                                // 查找下一行数据
//                                line = strchr(line, '\n');
//                                if (line != NULL) line++;
//                                line_count++;  // 每接收到一行数据，行数加一
//                                if (line_count >= max_lines) break;  // 如果已经接收了11行数据，跳出循环
//                            }
//                        }
//                    }
//                    else
//                    {
//                        // 如果没有接收到数据，可以选择打印日志
//                        rt_kprintf("No GPS data received after waiting for 5 seconds\n");
//                    }
//                }
//            }
//
//                // 处理完成后退出
//                rt_kprintf("Finished processing 11 lines of data.\n");
//
//        }
//        else
//        {
//            //rt_pin_write(PIN_PIN1, PIN_LOW);  // 熄灭 LED
//        }
//
//        rt_thread_mdelay(10);  // 主循环延时
//    }
//}


int main(void)
{
    rt_err_t ret = RT_EOK;

       char str[] = "hello RT-Thread!\r\n";
       static char msg_pool[256];

       /* 查找串口设备 */
         serial = rt_device_find(SAMPLE_UART_NAME);
         if (!serial)
         {
             rt_kprintf("find %s failed!\n", SAMPLE_UART_NAME);
             return RT_ERROR;
         }

         /* 初始化消息队列 */
          rt_mq_init(&rx_mq, "rx_mq", msg_pool, /* 存放消息的缓冲区 */
          sizeof(struct rx_msg), /* 一条消息的最大长度 */
          sizeof(msg_pool), /* 存放消息的缓冲区大小 */
          RT_IPC_FLAG_FIFO); /* 如果有多个线程等待，按照先来先得到的方法分配消息 */

           rt_device_open(serial, RT_DEVICE_FLAG_DMA_RX);
           /* 设置接收回调函数 */
           rt_device_set_rx_indicate(serial, uart_input);
           /* 发送字符串 */
           rt_device_write(serial, 0, str, (sizeof(str) - 1));

       /* 创建 GPS串口通信的serial 线程 *//* 定义 gps_sim 线程函数 *//*语音:A6输入*/
       rt_thread_t thread = rt_thread_create("serial", serial_thread_entry, RT_NULL, 1024, 25, 10);
        if (thread != RT_NULL)
        {
            rt_thread_startup(thread);
        }
        else
        {
            ret = RT_ERROR;
        }

//        /* 创建 GPS串口通信的serial 线程 *//* 定义 gps_sim 线程函数 *//*按键:A8输入*/
//        rt_thread_t thread1 = rt_thread_create("serial", serial_thread_entry1, RT_NULL, 1024, 25, 10);
//         if (thread1 != RT_NULL)
//         {
//             rt_thread_startup(thread1);
//         }
//         else
//         {
//             ret = RT_ERROR;
//         }

//        /* 语音控制 SYN6288 功能（类似音乐的播放和赞停）*//*语音:A7输入*/
//        // 初始化互斥量
//        state_mutex = rt_mutex_create("state_mutex", RT_IPC_FLAG_PRIO);
//        if (state_mutex == RT_NULL)
//        {
//            rt_kprintf("Failed to create mutex.\n");
//            return -1;
//        }
//
//        // 创建按键线程
//        rt_thread_t key_thread = rt_thread_create("key_thread", key_thread_entry, RT_NULL, 1024, 25, 10);
//        if (key_thread != RT_NULL)
//        {
//            rt_thread_startup(key_thread);
//        }
//
//        // 创建播放线程
//        rt_thread_t play_thread = rt_thread_create("play_thread", play_function, RT_NULL, 1024, 24, 10);
//        if (play_thread != RT_NULL)
//        {
//            rt_thread_startup(play_thread);
//        }

        /* 按键控制 SYN6288 功能（类似音乐的播放和赞停）*//*按键:A9输入*/
        // 初始化互斥量
        state_mutex1 = rt_mutex_create("state_mutex1", RT_IPC_FLAG_PRIO);
        if (state_mutex1 == RT_NULL)
        {
            rt_kprintf("Failed to create mutex.\n");
            return -1;
        }

        // 创建按键线程
        rt_thread_t key_thread1 = rt_thread_create("key_thread1", key_thread_entry1, RT_NULL, 1024, 25, 10);
        if (key_thread1 != RT_NULL)
        {
            rt_thread_startup(key_thread1);
        }

        // 创建播放线程
        rt_thread_t play_thread1 = rt_thread_create("play_thread1", play_function1, RT_NULL, 1024, 24, 10);
        if (play_thread1 != RT_NULL)
        {
            rt_thread_startup(play_thread1);
        }

    /* 创建 PWM 功能线程 *//*按键:A4输入*/
    rt_thread_t pwm_thread = rt_thread_create("pwm_thread", pwm_thread_entry, RT_NULL, 1024, 25, 10);
    if (pwm_thread != RT_NULL)
    {
        rt_thread_startup(pwm_thread);
    }
    else
    {
        rt_kprintf("Failed to create pwm_thread!\n");
        ret = RT_ERROR;
    }

//    /* 创建 PWM 功能线程 *//*语音:A5输入*/
//    rt_thread_t pwm_thread1 = rt_thread_create("pwm_thread1", pwm_thread_entry1, RT_NULL, 1024, 25, 10);
//    if (pwm_thread1 != RT_NULL)
//    {
//        rt_thread_startup(pwm_thread1);
//    }
//    else
//    {
//        rt_kprintf("Failed to create pwm_thread!\n");
//        ret = RT_ERROR;
//    }

    return ret;
}
