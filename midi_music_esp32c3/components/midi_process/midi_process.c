#include "include/midi_process.h"


uint8_t midi_file_verify(uint8_t *midi_file_ptr, uint8_t *midi_file_format, uint16_t *midi_track_num, uint16_t *midi_time_unit_tick)
{
    uint32_t head_chunk_head = 0x0;
    uint32_t head_chunk_size = 0x0;

    uint8_t *temp_ptr = midi_file_ptr;

    for(uint8_t i = 0; i < 4; i++)
    {
        head_chunk_head |= *temp_ptr;

        if(i < 3)
            head_chunk_head = head_chunk_head << 8;
        
        temp_ptr++;
    }

    for(uint8_t i = 0; i < 4; i++)
    {
        head_chunk_size |= *temp_ptr;

        if(i < 3)
            head_chunk_size = head_chunk_size << 8;
        
        temp_ptr++;
    }
        
    if(head_chunk_head != MIDI_HEADER_CHUNK || head_chunk_size != MIDI_HEADER_CHUNK_END) return 1;
    else
    {
        uint8_t file_format = 0x00;
        file_format += *temp_ptr;
        temp_ptr++;
        file_format += *temp_ptr;
        temp_ptr++;

        if(file_format > 2) return 2;
        else
        {
            *midi_file_format = file_format;

            uint16_t track_num = 0x0;

            track_num |= *temp_ptr;
            track_num = track_num << 8;
            temp_ptr++;
            track_num |= *temp_ptr;

            *midi_track_num = track_num;


            uint16_t track_format = 0x0;
            temp_ptr++;
            track_format |= *temp_ptr;
            track_format = track_format << 8;
            temp_ptr++;
            track_format |= *temp_ptr;
                
            if((uint8_t) (track_format & 0x8000) != 0) return 3;
            else
            {
                // SMPTE file ↑

                *midi_time_unit_tick = (track_format & 0x0FFF);
            }
        }
    }

    return 0;
}


uint8_t midi_file_init(char *file_name, MIDI_FILE *midi_file)
{
    char file_absolute_name[MOUNT_PATH_LEN + strlen(file_name) + 1];
    memset(file_absolute_name, 0, MOUNT_PATH_LEN + strlen(file_name) + 1);

    memcpy(file_absolute_name, MOUNT_PATH, 6);
    memcpy(file_absolute_name + 6, "/", 1);
    memcpy(file_absolute_name + 6 + 1, file_name, strlen(file_name));

    memset(midi_file->midi_file_name, 0, 64);
    memcpy(midi_file->midi_file_name, file_name, strlen(file_name));


    if(flash_mount() == ESP_OK)
    {
        FILE *file;
        uint8_t *file_buf;
        uint8_t *file_buffer;

        file = fopen(file_absolute_name, "r");
        fseek(file, 0, SEEK_END);
        midi_file->midi_file_size = ftell(file);
        rewind(file);

        file_buffer = (uint8_t *) malloc(midi_file->midi_file_size);
        fread(file_buffer, 1, midi_file->midi_file_size, file);

        file_buf = (uint8_t *) malloc(midi_file->midi_file_size);
        memcpy(file_buf, file_buffer, midi_file->midi_file_size);

        free(file_buffer);
        fclose(file);

        flash_unmount();


        midi_file->midi_file_ptr = file_buf;
        uint8_t *chunk_ptr = file_buf;

        if(midi_file_verify(chunk_ptr, &midi_file->midi_file_format, &midi_file->midi_track_num, &midi_file->midi_time_unit_tick) != 0) return 2;
    }
    else return 1;

    return 0;
}


uint8_t midi_file_load_track(MIDI_FILE *midi_file)
{
    MIDI_TRACK *temp_track = (MIDI_TRACK *) malloc(midi_file->midi_track_num * sizeof(MIDI_TRACK));

    if(temp_track == NULL) return 1;
    else midi_file->midi_file_track = temp_track;


    uint8_t *temp_ptr = midi_file->midi_file_ptr;

    for(uint8_t i = 0; i < 14; i++)
        temp_ptr++;

    uint32_t track_chunk_head = 0x0;
    uint32_t track_chunk_size = 0x0;

    for(uint16_t i = 0; i < midi_file->midi_track_num; i++)
    {        
        track_chunk_head = 0x0;
        track_chunk_size = 0x0;

        for(uint16_t j = 0; j < 4; j++)
        {
            track_chunk_head |= *temp_ptr;
			
            if(j < 3)
                track_chunk_head = track_chunk_head << 8;
            
            temp_ptr++;
        }

        for(uint16_t j = 0; j < 4; j++)
        {
            track_chunk_size |= *temp_ptr;

            if(j < 3)
                track_chunk_size = track_chunk_size << 8;
            
            temp_ptr++;
        }
				
        if(track_chunk_head != MIDI_TRACK_CHUNK) return 2;
        else midi_file->midi_file_track[i].midi_track_size = track_chunk_size;

        midi_file->midi_file_track[i].midi_track_ptr = temp_ptr;
        midi_file->midi_file_track[i].midi_cnt_track = temp_ptr;
				
		if(i < (midi_file->midi_track_num - 1))
	        for(uint16_t j = 0; j < track_chunk_size; j++)
	            temp_ptr++;
    }

    return 0;
}


uint32_t midi_read_dynamimc_byte(uint8_t* ptr, uint32_t* length)
{
    uint32_t value = 0;
    uint32_t bytesRead = 0;
    uint8_t currentByte = 0;

    do
    {
        currentByte = *ptr++;
        value = (value << 7) | (currentByte & 0x7F);
        bytesRead++;
    } while(currentByte & 0x80);

    if(length) *length = bytesRead;

    return value;
}


uint8_t midi_read_delay(MIDI_FILE *midi_file, uint32_t *delay_time_in_us, uint16_t track_to_read)
{
    uint8_t *temp_ptr = midi_file->midi_file_track[track_to_read].midi_cnt_track;

    uint32_t delay_chunk_len = 0;
    uint32_t delay_time = midi_read_dynamimc_byte(temp_ptr, &delay_chunk_len);
    
    for(uint32_t i = 0; i < delay_chunk_len; i++)
        temp_ptr++;
    
    midi_file->midi_file_track[track_to_read].midi_cnt_track = temp_ptr;
    *delay_time_in_us = delay_time;
    
    return 0;
}


uint8_t midi_read_command(MIDI_FILE *midi_file, MIDI_ACTION *midi_action, uint32_t track_to_read)
{
    uint8_t *temp_ptr = midi_file->midi_file_track[track_to_read].midi_cnt_track;    

    switch(*temp_ptr & 0xF0)
    {
        case MIDI_STATUS_BYTE_RELEASE:
        {
        	//printf("<Info> Tone Release [%x]\n", MIDI_STATUS_BYTE_RELEASE);

            midi_action->midi_note_info.midi_action = MIDI_ACTION_RELEASE;
            midi_action->midi_note_info.midi_channel = *temp_ptr & 0x0F;

            temp_ptr++;
            midi_action->midi_note_info.midi_tone = *temp_ptr;

            temp_ptr++;
            midi_action->midi_note_info.midi_velocity = *temp_ptr;

            temp_ptr++;

            break;
        }
        
        case MIDI_STATUS_BYTE_PRESS:
        {
        	//printf("<Info> Tone Press [%x]\n", MIDI_STATUS_BYTE_PRESS);

        	midi_action->midi_note_info.midi_action = MIDI_ACTION_PRESS;
            midi_action->midi_note_info.midi_channel = *temp_ptr & 0x0F;

            temp_ptr++;
            midi_action->midi_note_info.midi_tone = *temp_ptr;

            temp_ptr++;
            midi_action->midi_note_info.midi_velocity = *temp_ptr;
            if(midi_action->midi_note_info.midi_velocity == 0) midi_action->midi_note_info.midi_action = MIDI_ACTION_RELEASE;

            temp_ptr++;

            break;
        }

        case MIDI_STATUS_BYTE_AF_TOUCH:
        {
        	//printf("<Info> Tone After Touch [%x]\n", MIDI_STATUS_BYTE_AF_TOUCH);

			midi_action->midi_note_info.midi_action = MIDI_ACTION_AF_TOUCH;
            midi_action->midi_note_info.midi_channel = *temp_ptr & 0x0F;

            temp_ptr++;
            midi_action->midi_note_info.midi_tone = *temp_ptr;

            temp_ptr++;
            midi_action->midi_note_info.midi_velocity = *temp_ptr;
			
            temp_ptr++;

            break;
        }
        
        case MIDI_STATUS_BYTE_CTRL_CHANGE:
        {
            //printf("<Info> BX Control Change\n");

            midi_action->midi_note_info.midi_action = MIDI_ACTION_UNKNOWN;
            midi_action->midi_note_info.midi_channel = *temp_ptr & 0x0F;

            temp_ptr++;
            temp_ptr++;
            
            temp_ptr++;

            break;
        }

        case MIDI_STATUS_BYTE_INST_CHANGE:
        {
            //printf("<Info> CX Instrument Change\n");

            midi_action->midi_note_info.midi_action = MIDI_ACTION_UNKNOWN;
            midi_action->midi_note_info.midi_channel = *temp_ptr & 0x0F;

            temp_ptr++;
            
            temp_ptr++;

            break;
        }

        case MIDI_STATUS_BYTE_AF_PRESSURE:
        {
            //printf("<Info> DX AfterTouch Pressure Change\n");
            midi_action->midi_note_info.midi_action = MIDI_ACTION_UNKNOWN;
            midi_action->midi_note_info.midi_channel = *temp_ptr & 0x0F;

            temp_ptr++;
    
            temp_ptr++;

            break;
        }

        case MIDI_STATUS_BYTE_WEEL_CHANGE:
        {
            //printf("<Info> EX Bend Tone Wheel Action\n");
            midi_action->midi_note_info.midi_action = MIDI_ACTION_UNKNOWN;
            midi_action->midi_note_info.midi_channel = *temp_ptr & 0x0F;

            temp_ptr++;
            temp_ptr++;
            
            temp_ptr++;

            break;
        }

        case MIDI_STATUS_BYTE_META:
        {
            temp_ptr++;
            
            switch(*temp_ptr)
            {
                case MIDI_STATUS_BYTE_META_TRACK_END:
                {
                    //printf("<Info> Meta Event [%x], Track End\n", *temp_ptr);
                    //printf("<Info> Renew Track pointer\n");

                    midi_action->midi_note_info.midi_action = MIDI_ACTION_END;
                    
                    temp_ptr = midi_file->midi_file_track[track_to_read].midi_track_ptr;

					break;
                }

                case MIDI_STATUS_BYTE_META_TEMPO:
                {
                    //printf("<Info> Meta Event [%x], Set Tempo to ", *temp_ptr);
                    midi_action->midi_note_info.midi_action = MIDI_ACTION_SET_TEMPO;

                    temp_ptr++;
					
					uint8_t i_index = *temp_ptr;
					
                    uint32_t track_tempo = 0;
                    for(uint8_t i = 0; i < i_index; i++)
                    {
                        temp_ptr++;
                        track_tempo |= *temp_ptr;
                        if(i < (i_index-1)) track_tempo = track_tempo << 8;
                    }
                    
                    temp_ptr++;
					
                    midi_action->midi_track_tempo = track_tempo;
                    //printf("%d\n", track_tempo);
					
                    break;
                }
                
                default:
                {
                    //printf("<Info> Unrecognized Meta Event [%x]\n", *temp_ptr);
                    midi_action->midi_note_info.midi_action = MIDI_ACTION_UNKNOWN;

                    temp_ptr++;                    
										
                    uint32_t event_pos = 0;
                    uint32_t event_size = midi_read_dynamimc_byte(temp_ptr, &event_pos);
                    					         
                    for(uint32_t i = 0; i < (event_size + event_pos); i++)
                        temp_ptr++;
                    					         
                    break;
                }
            }
            
            break;
        }

        default:
        {
            printf("<Error> Unable to read the message. Error Code: [%x]\n", *temp_ptr);
            midi_file->midi_file_track[track_to_read].midi_cnt_track = midi_file->midi_file_track[track_to_read].midi_track_ptr;

            return MIDI_TRACK_READ_FAIL;
        }
    }

    midi_file->midi_file_track[track_to_read].midi_cnt_track = temp_ptr;

    return 0;
}


uint8_t midi_file_compile(MIDI_FILE *midi_file, MIDI_COMMAND **midi_cmd, uint32_t *midi_cmd_num)
{
    uint16_t midi_track_num = midi_file->midi_track_num;

    uint32_t available_mission_num = 0;

    for(uint16_t i = 0; i < midi_track_num; i++)
    {
        uint32_t temp_delay = 0;
        uint32_t midi_track_end_flag = 0;

        MIDI_ACTION midi_action;
        memset(&midi_action, 0, sizeof(MIDI_ACTION));

        while(midi_track_end_flag == 0)
        {
            midi_read_delay(midi_file, &temp_delay, i);
            midi_read_command(midi_file, &midi_action, i);
                        
            if(midi_action.midi_note_info.midi_action == MIDI_ACTION_END) midi_track_end_flag = 1;
			else
            {
                if(midi_action.midi_note_info.midi_action != MIDI_ACTION_UNKNOWN)
                    available_mission_num++;
            }
        }
    }

    uint32_t available_action_num = 0;
    *midi_cmd = (MIDI_COMMAND *) malloc(available_mission_num * sizeof(MIDI_COMMAND));
    memset(*midi_cmd, 0, available_mission_num * sizeof(MIDI_COMMAND));

    for(uint32_t i = 0; i < midi_track_num; i++)
    {
        uint32_t temp_delay = 0;
        uint32_t temp_combined_delay = 0;
        uint32_t midi_track_end_flag = 0;
        
        MIDI_ACTION midi_action;
        memset(&midi_action, 0, sizeof(MIDI_ACTION));

        while(midi_track_end_flag == 0)
        {
            midi_read_delay(midi_file, &temp_delay, i);
            midi_read_command(midi_file, &midi_action, i);
			
            temp_combined_delay += temp_delay;
            midi_action.midi_delay_tick = temp_combined_delay;
									
            if(midi_action.midi_note_info.midi_action == MIDI_ACTION_END) midi_track_end_flag = 1;
            else
            {
                if(midi_action.midi_note_info.midi_action != MIDI_ACTION_UNKNOWN)
                {
                    (*midi_cmd)[available_action_num].midi_delay_tick = midi_action.midi_delay_tick;
                    (*midi_cmd)[available_action_num].midi_action = midi_action.midi_note_info.midi_action;
                    
                    if(midi_action.midi_note_info.midi_action == MIDI_ACTION_SET_TEMPO)
                    {
                        (*midi_cmd)[available_action_num].midi_content = midi_action.midi_track_tempo;
                    }
                    else
                    {
                        (*midi_cmd)[available_action_num].midi_content = midi_action.midi_note_info.midi_tone;
                    }

                    available_action_num++;
                    *midi_cmd_num = available_action_num;
                }
            }
        }
	}

    return 0;
}


void midi_array_init(ARRAY_LIST_MIDI_ACTION *array_list, size_t allocate_size)
{
    array_list->array = malloc(allocate_size * sizeof(MIDI_COMMAND));
    array_list->used = 0;
    array_list->size = allocate_size;

    array_list->head_array = array_list->array;
}


void midi_array_add(ARRAY_LIST_MIDI_ACTION *array_list, MIDI_COMMAND midi_cmd)
{
    if(array_list->used == array_list->size)
    {
        array_list->size += 5;
        array_list->array = realloc(array_list->array, array_list->size * sizeof(MIDI_COMMAND));
    }

    array_list->array[array_list->used++] = midi_cmd;
}


void midi_array_free(ARRAY_LIST_MIDI_ACTION *array_list)
{
    free(array_list->head_array);

    array_list->array = NULL;
    array_list->head_array = NULL;

    array_list->used = 0;
    array_list->size = 0;
}


void midi_tick_bubble_sort(MIDI_COMMAND *arr, uint32_t n)
{
    uint32_t i, j;
    MIDI_COMMAND temp;

    for(i = 0; i < n-1; i++)
    {     
        for(j = 0; j < n-i-1; j++)
        {
            if(arr[j].midi_delay_tick > arr[j+1].midi_delay_tick)
            {
                temp = arr[j];

                arr[j] = arr[j+1];

                arr[j+1] = temp;
            }
        }
    }
}


uint8_t midi_file_play(MIDI_FILE *midi_file, MIDI_COMMAND *midi_cmd, uint32_t midi_cmd_num)
{
    midi_tick_bubble_sort(midi_cmd, midi_cmd_num);

    uint8_t *file_size_ptr = (uint8_t *) &midi_cmd_num;
    uint8_t midi_cmd_size[4];
    for(int8_t i = 3; i >= 0; i--)
    {
        midi_cmd_size[i] = *file_size_ptr;
        file_size_ptr++;
    }


    uint8_t *file_tick_ptr = (uint8_t *) &midi_file->midi_time_unit_tick;
    uint8_t midi_tick_size[2];
    for(int8_t i = 1; i >= 0; i--)
    {
        midi_tick_size[i] = *file_tick_ptr;
        file_tick_ptr++;
    }


    midi_command_i2c_write_byte(I2C_SLAVE_ADDR_STATUS, 0);
    printf("System Mode selected\n");

    vTaskDelay(100);

    midi_command_i2c_write_bytes(I2C_SLAVE_ADDR_MIDI_SIZE, midi_cmd_size, 4);
    printf("File Size Sent\n");

    vTaskDelay(100);

    midi_command_i2c_write_bytes(I2C_SLAVE_ADDR_MIDI_TICK, midi_tick_size, 2);
    printf("File Tick Sent\n");

    vTaskDelay(100);

    midi_command_i2c_write_byte(I2C_SLAVE_ADDR_STATUS, 1);
    printf("Enter playing process\n");

    vTaskDelay(250);

    uint32_t midi_pack_num = (midi_cmd_num * 13) / 65535;
    uint32_t midi_last_pack_size = (midi_cmd_num * 13) - (midi_pack_num * 65535);

    uint8_t *action_ptr = (uint8_t *) midi_cmd;

    printf("Total Send Pack: %d\n", midi_pack_num + 1);
    printf("Last Pack Remain: %d\n", midi_last_pack_size);

    if(midi_pack_num == 0)
    {
        uart_write_bytes(UART_NUM_1, action_ptr, midi_last_pack_size);
        vTaskDelay(200);
    }
    else
    {
        for(uint32_t i = 0; i < midi_pack_num; i++)
        {
            uart_write_bytes(UART_NUM_1, action_ptr, 65535);
            vTaskDelay(200);

            for(uint32_t j = 0; j < 65535; j++)
                action_ptr++;
        }

        uart_write_bytes(UART_NUM_1, action_ptr, midi_last_pack_size);
        vTaskDelay(200);
    }
    
    printf("File Sent. In total of %.4f kbyte(s), %d midi action(s)\n", ((float) (midi_cmd_num * 13) / 1024.0f), midi_cmd_num);

    vTaskDelay(100);
    midi_command_i2c_write_byte(I2C_SLAVE_ADDR_MIDI_PLAY_STATUS, 1);

    // END OF THE FILE (EOF)
    free(midi_file->midi_file_track);
    free(midi_cmd);

    return 0;
}


