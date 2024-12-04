#include "midi.h"

uint8_t midi_play_status = MIDI_PLAY_OFF;
uint32_t midi_file_size = 0;
uint16_t midi_file_tick = MIDI_FILE_DEFAULT_TICK;

MIDI_COMMAND MIDI_ACTION_SRAM[MIDI_FILE_MAX_SIZE];


void MIDI_SRAM_Init(void)
{
    memset(&I2C_SRAM[I2C_SLAVE_ADDR_MIDI_SIZE], 0, sizeof(uint8_t) * 4);
    memset(MIDI_ACTION_SRAM, 0, 13 * MIDI_FILE_MAX_SIZE);
}

void MIDI_Load_File(uint32_t *f_len)
{
    uint32_t temp_f = 0;

    temp_f = (I2C_SRAM[I2C_SLAVE_ADDR_MIDI_SIZE] << 24) | 
                (I2C_SRAM[I2C_SLAVE_ADDR_MIDI_SIZE + 1] << 16) | 
                (I2C_SRAM[I2C_SLAVE_ADDR_MIDI_SIZE + 2] << 8) | 
                (I2C_SRAM[I2C_SLAVE_ADDR_MIDI_SIZE + 3]);

    printf("file size: %d\n", temp_f);

    if(temp_f > (MIDI_FILE_MAX_SIZE * 9)) 
    {
        *f_len = 0;

        MIDI_SRAM_Init();
        I2C_SRAM_Init();
    }
    else *f_len = temp_f;
}

void MIDI_Load_Tick(uint16_t *f_tick)
{
    uint32_t temp_t = 0;

    temp_t = (I2C_SRAM[I2C_SLAVE_ADDR_MIDI_TICK] << 8) | (I2C_SRAM[I2C_SLAVE_ADDR_MIDI_TICK + 1]);
    printf("file tick: %d\n", temp_t);

    if(temp_t == 0)
    {
        *f_tick = 0;

        MIDI_SRAM_Init();
        I2C_SRAM_Init();
    }
    else *f_tick = temp_t;
}

void MIDI_Handler(void)
{
    MIDI_Load_File(&midi_file_size);
    MIDI_Load_Tick(&midi_file_tick);


    uint32_t midi_pack_num = (midi_file_size * 13) / 65535;
    uint32_t midi_last_pack_size = (midi_file_size * 13) - (midi_pack_num * 65535);

    uint8_t *midi_ptr = (uint8_t *) MIDI_ACTION_SRAM;

    if(midi_pack_num == 0)
    {
        UART2_DMA_Init(midi_ptr, midi_last_pack_size);

        while(UART2_DMA_FLAG == 0)
        {
			LED_ON();
        }
        UART2_DMA_FLAG = 0;
		
		LED_OFF();
		
    }
    else
    {
        for(uint32_t i = 0; i < midi_pack_num; i++)
        {
            UART2_DMA_Init(midi_ptr, 65535);
			
            while(UART2_DMA_FLAG == 0)
            {
				LED_ON();
            }			
			LED_OFF();
			
            for(uint32_t j = 0; j < 65535; j++)
                midi_ptr++;
        }

        UART2_DMA_Init(midi_ptr, midi_last_pack_size);

        while(UART2_DMA_FLAG == 0)
		{
			LED_ON();
		}
		UART2_DMA_FLAG = 0;
		
		LED_OFF();
    }
    
    while(UART2_DMA_FLAG != 0)
    {

    }
}

void MIDI_Play(void)
{
	LED_STATUS(1);
	
	uint32_t standard_tempo = MIDI_FILE_DEFAULT_TEMPO;
    uint16_t unit_tempo = midi_file_tick;
	
	uint32_t delay_tick_previous = 0;
    for(uint32_t i = 0; i < midi_file_size; i++)
    {		
    	MIDI_COMMAND midi_action = MIDI_ACTION_SRAM[i];
    	
        uint32_t delay_tick = midi_action.midi_delay_tick - delay_tick_previous;
    	float delay_time_ms = ((float) ((float) standard_tempo / (float) unit_tempo)) * (float) delay_tick;
        float temp_speed_up_delay_ms = (float) (delay_time_ms / 1000.0f) * 0.94f;
        
        delay_tick_previous = midi_action.midi_delay_tick;

        LED_Toggle();
        if(delay_time_ms != 0) delay_ms((uint32_t) ceilf(temp_speed_up_delay_ms));
        		
        switch(midi_action.midi_action)
        {
        	case MIDI_ACTION_RELEASE:
            {
                // Func release
                
                if(Tone_Free(tone_table[(uint8_t) midi_action.midi_content]) != 0)
                    printf("<Info> Release Error!\n");

                break;
            }

            case MIDI_ACTION_PRESS:
            {
                // Func press

                if(Tone_Alloc(tone_table[(uint8_t) midi_action.midi_content]))
                    printf("<Info> Press Error!\n");
                                    
                break;
            }

            case MIDI_ACTION_SET_TEMPO:
            {
                // Func set tempo

                standard_tempo = midi_action.midi_content;
    			
                break;
            }
/*
            default:
            {
                printf("<Info> Unknown Action [%d]\n", midi_action.midi_action);
                
                break;
            }
*/
		}

        TIM_Freqency_onLoop(tone_track);
	}
}
