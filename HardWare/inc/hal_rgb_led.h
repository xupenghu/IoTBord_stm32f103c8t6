#ifndef _HAL_RGB_LED_H
#define _HAL_RGB_LED_H

#include <stdio.h>
#include <stm32f1xx.h>
#include <drivers/pin.h>


#define  R_MAX  255
#define  G_MAX  255
#define  B_MAX  255

#define RGB_EN_PIN	10	//参考drv_gpio.c PA0
#define RGB_SCL_PIN	45
#define RGB_SDA_PIN	46


#define SCL_LOW 	rt_pin_write(RGB_SCL_PIN, PIN_LOW)
#define SCL_HIGH 	rt_pin_write(RGB_SCL_PIN, PIN_HIGH)

#define SDA_LOW		rt_pin_write(RGB_SDA_PIN, PIN_LOW)
#define SDA_HIGH	rt_pin_write(RGB_SDA_PIN, PIN_HIGH)

///*兼容V2.2及以上,RGB开关IO*/
#define RGB_Enable()		rt_pin_write(RGB_EN_PIN, PIN_HIGH)
#define RGB_Disable() 	rt_pin_write(RGB_EN_PIN, PIN_LOW)

void rgbLedInit(void);
void ledRgbControl(uint8_t R,uint8_t B,uint8_t G);


#endif /*_HAL_RGB_LED_H*/

