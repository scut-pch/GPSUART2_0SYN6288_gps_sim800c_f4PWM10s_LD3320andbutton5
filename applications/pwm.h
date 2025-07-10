#ifndef APPLICATIONS_INC_PWM_H_
#define APPLICATIONS_INC_PWM_H_

#include <rtthread.h>
#include <rtdevice.h>

#if 1
#define PWM_DEV_NAME        "pwm12"     // PWM设备名称
#define PWM_DEV_CHANNEL     2           // PWM通道

#define PWM_GPIO_PORT   GPIOB           // pwm端口号 -- TIM12_CH2
#define PWM_GPIO_PIN    GPIO_PIN_15     // pwm引脚号
#else
#define PWM_DEV_NAME        "pwm3"     // PWM设备名称
#define PWM_DEV_CHANNEL     2           // PWM通道

#define PWM_GPIO_PORT   GPIOA           // pwm端口号 -- TIM3_CH2
#define PWM_GPIO_PIN    GPIO_PIN_7      // pwm引脚号
#endif

struct rt_device_pwm *pwm_dev;          // PWM设备句柄

extern int set_pwm_param(rt_uint32_t fre, rt_uint32_t duty);
extern int inset_pwm_param(rt_uint32_t fre, rt_uint32_t duty);
#endif /* APPLICATIONS_INC_PWM_H_ */

