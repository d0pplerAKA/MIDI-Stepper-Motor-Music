#include "include/midi_play.h"

midi_motor_t midi_motor_status[MIDI_MOTOR_NUM];


void midi_motor_channel_init(midi_motor_t *motor_channel)
{
    memset(motor_channel, 0, sizeof(midi_motor_t) * MIDI_MOTOR_NUM);
}


uint8_t midi_motor_process(midi_motor_t *motor_channel, midi_info_t midi_info, midi_command_t *cmd)
{
    if(midi_info.note == MIDI_MOTOR_RUN)
    {
        int8_t channel_available = -1;

        for(uint8_t i = 0; i < MIDI_MOTOR_NUM; i++)
        {
            if(motor_channel[i].motor_status == MIDI_MOTOR_STOP)
            {
                channel_available = i;

                motor_channel[i].motor_status = MIDI_MOTOR_RUN;
                motor_channel[i].midi_info.note = midi_info.note;
                motor_channel[i].midi_info.tone = midi_info.tone;

                break;
            }
        }

        if(channel_available != -1)
        {
            cmd->motor_channel = channel_available;
            cmd->motor_status = MIDI_MOTOR_RUN;
            cmd->motor_tone = midi_info.tone;
        } else return MIDI_COMMAND_CANTDO;
    }
    else
    {
        int8_t channel_available = -1;

        for(uint8_t i = 0; i < MIDI_MOTOR_NUM; i++)
        {
            if(motor_channel[i].midi_info.tone == midi_info.tone)
            {
                channel_available = i;

                motor_channel[i].motor_status = MIDI_MOTOR_STOP;
                motor_channel[i].midi_info.note = 0;
                motor_channel[i].midi_info.tone = 0;

                break;
            }
        }

        if(channel_available != -1)
        {
            cmd->motor_channel = channel_available;
            cmd->motor_status = MIDI_MOTOR_STOP;
            //cmd->motor_tone = midi_info.tone;
        } else return MIDI_COMMAND_CANTDO;
    }

    return MIDI_COMMAND_CANDO;
}




void midi_play_task(void *pt)
{
    char *midi_file_name;
    midi_file_name = (char *) pt;
    

    MIDI_FILE midi_file_on_flash;

    if(midi_file_init(midi_file_name, &midi_file_on_flash) == 0)
    {
        if(midi_file_load_track(&midi_file_on_flash) == 0)
        {
            uint32_t midi_command_amount = 0;
            MIDI_COMMAND *midi_command = NULL;

            if(midi_file_compile(&midi_file_on_flash, &midi_command, &midi_command_amount) == 0)
            {
                if(midi_file_play(&midi_file_on_flash, midi_command, midi_command_amount) != 0) printf("<ERROR> midi data transfer failed\n");

                //midi_play_debug(MIDI_FILE_DEFAULT_TEMPO, midi_file_on_flash.midi_time_unit_tick, midi_command, midi_command_amount);
            }
            else printf("<ERROR> midi data compile failed\n");

        }
        else printf("<ERROR> midi file track load failed\n");
    }
    else printf("<ERROR> midi file open failed\n");

    vTaskDelete(NULL);
}


void midi_play_debug(uint32_t midi_tempo, uint16_t midi_unit_tempo, MIDI_COMMAND *midi_command, uint32_t midi_command_num)
{
    midi_tick_bubble_sort(midi_command, midi_command_num);

    uint32_t standard_tempo = midi_tempo;
    uint16_t unit_tempo = midi_unit_tempo;

	uint32_t delay_tick_previous = 0;
    for(uint32_t i = 0; i < midi_command_num; i++)
    {		
    	MIDI_COMMAND midi_action = midi_command[i];
        uint32_t current_delay_tick = midi_action.midi_delay_tick;
    	uint32_t delay_tick = current_delay_tick - delay_tick_previous;

        if(delay_tick != 0) printf("tick: %d\n", delay_tick);

        float current_tempo = (float) standard_tempo / (float) unit_tempo;
        float delay_time_ms = current_tempo * (float) delay_tick;
        float temp_speed_up_delay_ms = (float) (delay_time_ms / 1000.0f) * 0.93f;

        delay_tick_previous = midi_action.midi_delay_tick;

        //if(temp_speed_up_delay_ms != 0) vTaskDelay((uint32_t) temp_speed_up_delay_ms);


        switch(midi_action.midi_action)
        {
        	case MIDI_ACTION_RELEASE:
            {
                // Func release
                //printf("<Info> Release Tone: %d\n", midi_action.midi_content);

                break;
            }

            case MIDI_ACTION_PRESS:
            {
                // Func press
                
                //printf("<Info> Press Tone: %d\n", midi_action.midi_content);
                                    
                break;
            }

            case MIDI_ACTION_SET_TEMPO:
            {
                // Func set tempo
				
                printf("<Info> set tempo from %d to %d\n", 
                        standard_tempo, 
                        midi_action.midi_content);
                
                standard_tempo = midi_action.midi_content;
    			
                break;
            }
            
            default:
            {
                printf("<Info> Unknown Action [%d]\n", midi_action.midi_action);
                
                break;
            }
		}

	}
}
