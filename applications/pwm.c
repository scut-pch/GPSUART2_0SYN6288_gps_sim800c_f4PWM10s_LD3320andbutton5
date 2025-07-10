#include "pwm.h"


/**
 * @brief 设置PWM的参数
 * @param fre 频率 0-999
 * @param duty 占空比 0-100
 * @return
 */
int set_pwm_param(rt_uint32_t fre, rt_uint32_t duty)
{
    rt_uint32_t period, pulse;

    if ((fre < 0 && fre > 1000000) && duty > 0)
    {
        rt_kprintf("set param error...\n");
        return -RT_ERROR;
    }
    else
    {
        if (duty >= 100)
        {
            duty = 100;
        }
    }

    period = 1000000000 / fre;                 /* 周期为0.5ms，单位为纳秒ns */
    pulse = period * duty / 100;               /* PWM脉冲宽度值 */

    /* 查找设备 */
    pwm_dev = (struct rt_device_pwm *)rt_device_find(PWM_DEV_NAME);
    if (pwm_dev == RT_NULL)
    {
        rt_kprintf("pwm sample run failed! can't find %s device!\n", PWM_DEV_NAME);
        return RT_ERROR;
    }

    /* 设置PWM周期和脉冲宽度默认值 */
    rt_pwm_set(pwm_dev, PWM_DEV_CHANNEL, period, pulse);

    /* 使能设备 */
    rt_pwm_enable(pwm_dev, PWM_DEV_CHANNEL);

    return 0;
}

int inset_pwm_param(rt_uint32_t fre, rt_uint32_t duty)
{
    rt_uint32_t period, pulse;

    if ((fre < 0 && fre > 1000000) && duty > 0)
    {
        rt_kprintf("set param error...\n");
        return -RT_ERROR;
    }
    else
    {
        if (duty >= 100)
        {
            duty = 100;
        }
    }

    period = 1000000000 / fre;                 /* 周期为0.5ms，单位为纳秒ns */
    pulse = period * duty / 100;               /* PWM脉冲宽度值 */

    /* 查找设备 */
    pwm_dev = (struct rt_device_pwm *)rt_device_find(PWM_DEV_NAME);
    if (pwm_dev == RT_NULL)
    {
        rt_kprintf("pwm sample run failed! can't find %s device!\n", PWM_DEV_NAME);
        return RT_ERROR;
    }

    /* 设置PWM周期和脉冲宽度默认值 */
    rt_pwm_set(pwm_dev, PWM_DEV_CHANNEL, period, pulse);

    /* 使能设备 */
    rt_pwm_disable(pwm_dev, PWM_DEV_CHANNEL);

    return 0;
}
