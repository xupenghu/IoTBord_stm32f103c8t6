#ifndef _HAL_HEMP_HUM_H
#define _HAL_HEMP_HUM_H

#include <stdio.h>
#include <stm32f1xx_hal.h>
#include <drivers/pin.h>

#define DHT11_PIN	39

//Set GPIO Direction
#define	DHT11_DQ_OUT_1	 rt_pin_write(DHT11_PIN,PIN_HIGH)
#define	DHT11_DQ_OUT_0 	 rt_pin_write(DHT11_PIN,PIN_LOW)
#define	DHT11_DQ_IN   	 rt_pin_read(DHT11_PIN)

#define MEAN_NUM            10

typedef struct
{
    uint8_t curI;
    uint8_t thAmount; 
    uint8_t thBufs[10][2];
}thTypedef_t; 

typedef struct 
{
	uint8_t tempreture;
	uint8_t humidity;
}DHT11INFO;

extern DHT11INFO DHT11Info;

/* Function declaration */
uint8_t dht11Init(void); //Init DHT11
uint8_t dht11Read(uint8_t *temperature, uint8_t *humidity); //Read DHT11 Value
static uint8_t dht11ReadData(uint8_t *temperature, uint8_t *humidity);
static uint8_t dht11ReadByte(void);//Read One Byte
static uint8_t dht11ReadBit(void);//Read One Bit
static uint8_t dht11Check(void);//Chack DHT11
static void dht11Rst(void);//Reset DHT11    
void dht11SensorTest(void);

#endif /*_HAL_HEMP_HUM_H*/

