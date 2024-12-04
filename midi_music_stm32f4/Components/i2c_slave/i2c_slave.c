#include "i2c_slave.h"



uint8_t i2c2_write_command = 0;
uint8_t i2c2_write_protection = 0;
uint8_t i2c2_read_command = 0;
uint8_t i2c2_read_protection = 0;

uint8_t I2C_SRAM_BLOCK = I2C_SRAM_BLOCK_BASIC;
uint8_t I2C_SRAM[I2C_SRAM_SIZE];

uint32_t i2c2_sram_ptr = 0;
uint8_t *i2c2_midi_ptr = NULL;



void I2C_SRAM_Init(void)
{
    memset(I2C_SRAM, 0, sizeof(uint8_t) * I2C_SRAM_SIZE);
    I2C_SRAM[I2C_SLAVE_ADDR_STATUS] = System_Status_Waiting;
}

void I2C2_Slave_Init(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;    // PB3 SDA | PB10 SCL
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    GPIO_SetBits(GPIOB, GPIO_Pin_3);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    GPIO_SetBits(GPIOB, GPIO_Pin_10);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF9_I2C2);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_I2C2);


    I2C_InitTypeDef I2C_InitStruct;
    I2C_DeInit(I2C2);

    I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStruct.I2C_ClockSpeed = 400000;
    I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStruct.I2C_OwnAddress1 = I2C_SLAVE_ADDRESS;
    I2C_Init(I2C2, &I2C_InitStruct);

    I2C_ITConfig(I2C2, I2C_IT_EVT | I2C_IT_BUF | I2C_IT_ERR, ENABLE); 
    I2C_Cmd(I2C2, ENABLE);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = I2C2_EV_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel = I2C2_ER_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStruct);
}


void I2C2_Clear_ADDR(void)
{
    //(void)I2C2->SR1;
    //(void)I2C2->SR2;

    while((I2C2->SR1 & I2C_SR1_ADDR) == I2C_SR1_ADDR) 
    {
        (void) I2C2->SR1;
        (void) I2C2->SR2;
    }
}


void I2C2_Clear_STOPF(void)
{
    //(void)I2C2->SR1;
    //I2C2->CR1 |= I2C_CR1_PE;

    while((I2C2->SR1&I2C_SR1_STOPF) == I2C_SR1_STOPF) 
    {
        (void) I2C2->SR1;
        I2C2->CR1 |= 0x1;
    }
}


void I2C2_EV_IRQHandler(void)
{
    uint32_t i2c_event = I2C_GetLastEvent(I2C2);
    
    if(i2c_event == I2C_EVENT_SLAVE_TRANSMITTER_ADDRESS_MATCHED)        // 主机读地址 EV1
    {
        //I2C2_Clear_ADDR();

        i2c2_read_command = 0;
    }
    else if(i2c_event == I2C_EVENT_SLAVE_BYTE_TRANSMITTING)             // 主机读数据 在读 EV3
    {
        if(i2c2_sram_ptr < I2C_SRAM_SIZE && i2c2_read_protection == 0) I2C_SendData(I2C2, I2C_SRAM[i2c2_sram_ptr]);
        else I2C_SendData(I2C2, 0xCE);
    }
    else if(i2c_event == I2C_EVENT_SLAVE_BYTE_TRANSMITTED)              // 主机读数据 读到 EV3
    {
        i2c2_sram_ptr++;
    }
    else if(i2c_event == I2C_EVENT_SLAVE_RECEIVER_ADDRESS_MATCHED)      // 主机写地址 EV1
    {
        //I2C2_Clear_ADDR();

        i2c2_sram_ptr = 0;
        i2c2_write_command = 0;
    }
    else if(i2c_event == I2C_EVENT_SLAVE_BYTE_RECEIVED)                 // 主机写数据 EV2
    {
        uint8_t recv = I2C_ReceiveData(I2C2);

        i2c2_write_command++;

        if(i2c2_write_command > 1)
        {
            if(I2C_SRAM_BLOCK == I2C_SRAM_BLOCK_BASIC)
            {
                if(i2c2_sram_ptr < I2C_SRAM_SIZE)
                {
                    *i2c2_midi_ptr = recv;
                    i2c2_midi_ptr++;

                    i2c2_sram_ptr++;
                }
            }

            if(I2C_SRAM_BLOCK == I2C_SRAM_BLOCK_INVALID)
                if(i2c2_write_command % 3 == 0) LED_Toggle();
        }
        else 
        {
            i2c2_sram_ptr = recv;

            if(i2c2_sram_ptr > I2C_SRAM_SIZE)
            {
                if(i2c2_sram_ptr == I2C_SLAVE_ADDR_WHO_READ_ONLY)
                {
                    i2c2_read_protection = 1;

                    I2C_SRAM_BLOCK = I2C_SRAM_BLOCK_INVALID;
                }
                else if(i2c2_sram_ptr == I2C_SLAVE_ADDR_RST)
                {
                    // System restart
                    I2C_SRAM_BLOCK = I2C_SRAM_BLOCK_INVALID;
                }
                else I2C_SRAM_BLOCK = I2C_SRAM_BLOCK_INVALID;
            }
            else
            {
                i2c2_midi_ptr = (uint8_t *) I2C_SRAM;

                for(uint8_t i = 0; i < i2c2_sram_ptr; i++)
                    i2c2_midi_ptr++;
                
                I2C_SRAM_BLOCK = I2C_SRAM_BLOCK_BASIC;
            }      
        }
    }
    else if(i2c_event == I2C_EVENT_SLAVE_STOP_DETECTED)                 // STOP EV4
    {
        I2C_ClearFlag(I2C2, I2C_FLAG_STOPF);
        I2C2_Clear_ADDR();
        I2C2_Clear_STOPF();

        i2c2_write_command = 0;
        i2c2_write_protection = 0;
        i2c2_read_command = 0;
        i2c2_read_protection = 0;

        i2c2_sram_ptr = 0;
        i2c2_midi_ptr = NULL;

        I2C_Cmd(I2C2, ENABLE);
    }

    /*
    i2c2_write_command = 0;
    i2c2_write_protection = 0;
    i2c2_read_command = 0;
    i2c2_read_protection = 0;

    i2c2_sram_ptr = 0;
    i2c2_midi_ptr = NULL;
    */

    /*
    else                                                                // (i2c_event == I2C_EVENT_SLAVE_ACK_FAILURE)
    {
        //printf("ACK ERROR!\n");
        I2C_ClearFlag(I2C2, I2C_FLAG_AF);
        I2C_ClearITPendingBit(I2C2, I2C_IT_AF);
        I2C2_Clear_ADDR();
        I2C2_Clear_STOPF();

        i2c2_write_command = 0;
        i2c2_write_protection = 0;
        i2c2_read_command = 0;
        i2c2_read_protection = 0;

        i2c2_sram_ptr = 0;
        i2c2_midi_ptr = NULL;

        I2C_Cmd(I2C2, ENABLE);
    }
    */
}

void I2C2_ER_IRQHandler(void) 
{
    if(I2C_GetITStatus(I2C2, I2C_IT_AF)) 
    {
        I2C_ClearITPendingBit(I2C2, I2C_IT_AF);
    }

    if(I2C_GetITStatus(I2C2, I2C_IT_BERR)) 
    {
        I2C_ClearITPendingBit(I2C2, I2C_IT_BERR);
    }

    if(I2C_GetITStatus(I2C2, I2C_IT_ARLO)) 
    {
        I2C_ClearITPendingBit(I2C2, I2C_IT_ARLO);
    }

    if(I2C_GetITStatus(I2C2, I2C_IT_OVR)) 
    {
        I2C_ClearITPendingBit(I2C2, I2C_IT_OVR);
    }

    if(I2C_GetITStatus(I2C2, I2C_IT_PECERR)) 
    {
        I2C_ClearITPendingBit(I2C2, I2C_IT_PECERR);
    }

    if(I2C_GetITStatus(I2C2, I2C_IT_TIMEOUT)) 
    {
        I2C_ClearITPendingBit(I2C2, I2C_IT_TIMEOUT);
    }
}
