/*
 * @file name: 
 * @Descripttion: 
 * @Author: zgw
 * @email: liang.zhang@tuya.com
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2021-03-17 10:42:50
 * @LastEditors: zgw
 * @LastEditTime: 2021-03-19 11:11:54
 */

#include "TCS34725.h"
#include "soc_i2c.h"  


STATIC int __iic_start(VOID)
{
    vI2CSDASet();
    vI2CSCLSet();

    vI2CDelay(5);

    vI2CSDAReset();

    vI2CDelay(5);

    vI2CSCLReset();

    return 0;
}


STATIC int __iic_stop(VOID)
{   
    vI2CSCLReset();
    vI2CSDAReset();

    vI2CDelay(5);
    
    vI2CSCLSet();
    vI2CSDASet();

    vI2CDelay(5);

    return 0;      
}


STATIC VOID __iic_send_ack(VOID)
{   
    vI2CSCLReset();
        
    vI2CSDAReset();

    vI2CDelay(2);
    vI2CSCLSet();

    vI2CDelay(2);

    vI2CSCLReset();
}

STATIC VOID __iic_send_nack(VOID)
{   
    vI2CSCLReset();
        
    vI2CSDASet();

    vI2CDelay(2);
    vI2CSCLSet();

    vI2CDelay(2);

    vI2CSCLReset();
}

STATIC UINT8_T __iic_recv_ack(VOID)
{   
    UINT8_T ucErrTime=0;
    UINT8_T recv_ack;
    vI2CSDARelease();
    vI2CSCLSet();
    vI2CDelay(5);
    
    while(ucI2CSDAInputRead()) {
        ucErrTime++;
        if(ucErrTime>250) {
			__iic_stop();
            PR_NOTICE("-------iic ack error-----");
			return 1;
		}
    }

    
    vI2CSCLReset();

    return 0;
}

STATIC VOID __iic_send_byte(UCHAR_T sendbyte)
{
    UCHAR_T i = 0;
    vI2CSCLReset;
    for(i = 0x80; i > 0; i >>= 1)
    {
        if((sendbyte & i) == 0) {
            vI2CSDAReset();
        } else {
            vI2CSDASet();
        }
        vI2CDelay(2);   
        vI2CSCLSet();
        vI2CDelay(2); //vI2CDelay(5);
        vI2CSCLReset();
        vI2CDelay(2);
    }
}

STATIC UINT8_T __iic_read_byte(UCHAR_T ack)
{
    UCHAR_T i = 0;
    
    UCHAR_T readbyte = 0;
    vI2CSDARelease();
    for(i = 0x80; i > 0; i >>= 1)
    {
        vI2CSCLReset();
        vI2CDelay(2);
        vI2CSCLSet();
        if(ucI2CSDAInputRead()) {
            readbyte |= i;    
        }
        vI2CDelay(2);
    }
    if(!ack) {
        __iic_send_nack(); 
    }else {
        __iic_send_ack();
    }
    
    return readbyte;    
}


VOID tcs34725_iic_write(UINT8_T drv_addr, UINT8_T* data_buffer, UINT8_T bytes_num, UINT8_T stop_bit)
{   
	UCHAR_T i = 0;
	
	__iic_start();
	__iic_send_byte((drv_addr << 1) | 0x00);	   //发送从机地址写命令
	__iic_recv_ack();
	for(i = 0; i < bytes_num; i++)
	{
		__iic_send_byte(*(data_buffer + i));
		__iic_recv_ack();
	}
	if(stop_bit == 1) __iic_stop();
}



VOID tcs34725_iic_read(UINT8_T drv_addr, UINT8_T* data_buffer, UINT8_T byte_num, UINT8_T stop_bit)
{
	UCHAR_T i = 0;
	
	__iic_start();
	__iic_send_byte((drv_addr << 1) | 0x01);	   //发送从机地址读命令
	__iic_recv_ack();
	for(i = 0; i < byte_num; i++)
	{
		if(i == byte_num - 1)
		{
			*(data_buffer + i) = __iic_read_byte(0);//读取的最后一个字节发送NACK
		}
		else
		{
			*(data_buffer + i) = __iic_read_byte(1);
		}
	}
	if(stop_bit == 1) __iic_stop();
}


VOID TCS34725_Write(UCHAR_T sub_addr, UCHAR_T* data_buffer, UCHAR_T byte_num)
{
    unsigned char send_buffer[10] = {0, };
    unsigned char byte = 0;
    
    send_buffer[0] = sub_addr | TCS34725_COMMAND_BIT;
    for(byte = 1; byte <= byte_num; byte++)
    {
        send_buffer[byte] = data_buffer[byte - 1];
    }
	tcs34725_iic_write(TCS34725_ADDRESS, send_buffer, byte_num + 1, 1);
}


VOID TCS34725_Read(UCHAR_T sub_addr, UCHAR_T* data_buffer, UCHAR_T byte_num)
{
	sub_addr |= TCS34725_COMMAND_BIT;
	
	tcs34725_iic_write(TCS34725_ADDRESS, (UCHAR_T*)&sub_addr, 1, 0);
	tcs34725_iic_read(TCS34725_ADDRESS, data_buffer, byte_num, 1);
}


VOID tcs34725_set_integration_time(UINT8_T time)
{   
    UCHAR_T cmd = time;
    
	TCS34725_Write(TCS34725_ATIME, &cmd, 1);
}


VOID tcs34725_set_gain(UINT8_T gain)
{
    UCHAR_T cmd = gain;
	TCS34725_Write(TCS34725_CONTROL, &cmd, 1);
}


VOID tcs34725_set_up(VOID)
{
    tcs34725_set_integration_time(TCS34725_INTEGRATIONTIME_240MS);
	tcs34725_set_gain(TCS34725_GAIN_4X);
}


VOID tcs34725_enable(VOID)
{
    UINT8_T cmd = TCS34725_ENABLE_PON;

    TCS34725_Write(TCS34725_ENABLE, &cmd, 1);

    cmd = TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN;

	TCS34725_Write(TCS34725_ENABLE, &cmd, 1);

    vI2CDelay(3000);
}

VOID tcs34725_disable(VOID)
{
    UINT8_T cmd = 0;

	TCS34725_Read(TCS34725_ENABLE, &cmd, 1);
	cmd = cmd & ~(TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN);
	TCS34725_Write(TCS34725_ENABLE, &cmd, 1);
}

VOID tcs34725_init(TCS34725_init_t* param)
{
    UINT8_T id = 0;
    UCHAR_T status[1] = {0};

    i2c_pin_t i2c_config = {
        .ucSDA_IO = param ->SDA_PIN,
        .ucSCL_IO = param ->SCL_PIN,
    };
    
    opSocI2CInit(&i2c_config);          /* SDA&SCL GPIO INIT */
    vI2CDelay(1000);

	TCS34725_Read(TCS34725_ID, status, 1);
    return status[0];

}

UINT16_T tcs34725_get_channel_data(UINT8_T reg)
{
	UINT8_T tmp[2] = {0,0};
	UINT16_T data = 0;
	
	TCS34725_Read(reg, tmp, 2);
	data = ((UINT16_T)tmp[1] << 8) | tmp[0];
	
	return data;
}

UINT8_T tcs34725_get_raw_data(COLOR_RGBC *rgbc)
{
	UCHAR_T status[1] = {0};

	status[0] = TCS34725_STATUS_AVALID;
	
	TCS34725_Read(TCS34725_STATUS, status, 1);
	
	if(status[0] & TCS34725_STATUS_AVALID)
	{
		rgbc->c = tcs34725_get_channel_data(TCS34725_CDATAL);	
		rgbc->r = tcs34725_get_channel_data(TCS34725_RDATAL);	
		rgbc->g = tcs34725_get_channel_data(TCS34725_GDATAL);	
		rgbc->b = tcs34725_get_channel_data(TCS34725_BDATAL);
		return 1;
	}
	return 0;
}

VOID RAW_to_RGB(COLOR_RGBC *rgbc, COLOR_RGBC *rgb)
{
	rgb->r = rgbc->r*255/rgbc->c;  
	rgb->g = rgbc->g*255/rgbc->c;
	rgb->b = rgbc->b*255/rgbc->c;
}