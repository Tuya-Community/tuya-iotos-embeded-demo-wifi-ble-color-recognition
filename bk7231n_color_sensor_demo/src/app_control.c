/*
 * @Author: zgw
 * @email: liang.zhang@tuya.com
 * @LastEditors: zgw
 * @file name: sht21.h
 * @Description: SHT21 IIC drive src file
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2020-12-16 18:51:29
 * @LastEditTime: 2021-03-19 10:58:16
 */

#include "app_control.h"
#include "tuya_gpio.h"
#include "tuya_uart.h"
#include "BkDriverUart.h"
#include "sys_timer.h"
#include "uni_time.h"
#include "app_pwm.h"
#include "TCS34725.h"
/***********************************************************
*************************types define***********************
***********************************************************/
typedef enum
{
    LOW = 0,
    HIGH,
}default_level;

APP_CTRL_DATA_T app_ctrl_data = {0};

COLOR_RGBC rgb;
COLOR_RGBC color_ratio;
/***********************************************************
*************************IO control device define***********
***********************************************************/

/***********************************************************
*************************about adc init*********************
***********************************************************/
#define IIC_SDA_PIN                         (6)
#define IIC_SCL_PIN                         (7)

STATIC TCS34725_init_t tcs34725_init_param = {IIC_SDA_PIN, IIC_SCL_PIN};


/***********************************************************
*************************about iic init*********************
***********************************************************/

/***********************************************************
*************************function***************************
***********************************************************/

STATIC VOID __ctrl_gpio_init(CONST TY_GPIO_PORT_E port, CONST BOOL_T high)
{   
    //Set IO port as output mode
    tuya_gpio_inout_set(port, FALSE);
    //Set IO port level
    tuya_gpio_write(port, high);
}


VOID app_device_init(VOID)
{
    tuya_hal_system_sleep(1000);
    tcs34725_init(&tcs34725_init_param);
    tuya_hal_system_sleep(1000);
    tcs34725_enable();
}



VOID app_ctrl_handle(VOID)
{   
    if(tcs34725_get_raw_data(&rgb)) {
		RAW_to_RGB(&rgb,&color_ratio);
        app_ctrl_data.r_value = color_ratio.r;
        app_ctrl_data.g_value = color_ratio.g;
        app_ctrl_data.b_value = color_ratio.b;
	}
}

VOID app_ctrl_all_off(VOID)
{   

}

 