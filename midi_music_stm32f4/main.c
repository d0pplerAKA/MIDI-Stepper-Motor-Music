#include "stm32f4xx.h"
#include "arm_math.h"

#include "delay/delay.h"
#include "uart/uart.h"
#include "led/led.h"
#include "timer/timer.h"
#include "tone/tone.h"
#include "midi/midi.h"
#include "i2c_slave/i2c_slave.h"
#include "status/status.h"


int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	TIM_GPIO_Init();
	TIM_Channel_Init();
	TIM_Freqency_Clear(tone_track);
	
	Tone_Table_Init();
	MIDI_SRAM_Init();
	I2C_SRAM_Init();

	UART1_Init(115200);
	UART2_Init(921600);
	
	I2C2_Slave_Init();

	LED_Init();
	LED_blink();
	LED_STATUS(0);
	printf("Hello World!\n");
	

	while(1)
	{
		// OLED Task

		if(I2C_SRAM[I2C_SLAVE_ADDR_STATUS] == System_Status_Waiting)	// Waiting for command
		{
			if(I2C_SRAM[I2C_SLAVE_ADDR_MIDI_CLR] != 0)
			{
				I2C_SRAM[I2C_SLAVE_ADDR_MIDI_CLR] = 0;

				MIDI_SRAM_Init();
				I2C_SRAM_Init();
			}
		}
		else									// MIDI Play
		{
			LED_OFF();		
			LED_STATUS(1);
			
			MIDI_Handler();
			LED_STATUS(0);
			
			LED_blink();
			
			if(midi_file_size != 0 && midi_file_tick != 0)
			{				
				while(I2C_SRAM[I2C_SLAVE_ADDR_MIDI_PLAY_STATUS] == 0)
				{
				}
				//{
					//printf("file size: %d, file tick: %d\n", midi_file_size, midi_file_tick);

				//}

				// ...
				
				printf("Midi Play.\n");
				
				MIDI_Play();
				
				LED_STATUS(0);
				
				LED_blink();

				TIM_Freqency_Clear(tone_track);
				TIM_GPIO_Init();
				TIM_Channel_Init();
				
				Tone_Table_Init();
				MIDI_SRAM_Init();
				I2C_SRAM_Init();
				
				UART2_Init(921600);

				I2C_SRAM[I2C_SLAVE_ADDR_MIDI_PLAY_STATUS] = 0;
				I2C_SRAM[I2C_SLAVE_ADDR_STATUS] = System_Status_Waiting;
			}
			else
			{
				I2C_SRAM[I2C_SLAVE_ADDR_STATUS] = System_Status_Waiting;

				MIDI_SRAM_Init();
				I2C_SRAM_Init();

				LED_STATUS(0);
			}
		}
	}
}

void stm_restart(void)
{
	//SCB->AIRCR = 0x05FA0000 | (uint32_t) 0x04;

	__set_FAULTMASK(ENABLE);

	NVIC_SystemReset();
}

