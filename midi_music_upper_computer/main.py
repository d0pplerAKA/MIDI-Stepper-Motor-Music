import os
import socket
import binascii
import math
import time


target_midi_file_path = "./midi_file/going_to_california.mid"

socket_tcp_port1 = 22
socket_tcp_port2 = 23
socket_tcp_address = "192.168.4.1"


ACK_CODE_FAIL = "fail"
ACK_CODE_RECV = "recv"
ACK_CODE_STRT = "strt"
ACK_CODE_RTRY = "rtry"
ACK_CODE_NTRY = "ntry"
ACK_CODE_CNTU = "cntu"



def read_into_buffer(filename):
    
    buf = bytearray(os.path.getsize(filename))
    
    with open(filename, 'rb') as f:
        f.readinto(buf)
    
    f.close()
    
    return buf


def int_to_vla_bytes(number, byte_order='big'):

    bytes_list = bytearray()
    bytes_list.append(number & 0x7F)
    number >>= 7

    while number > 0:
        bytes_list.append((number & 0x7F) | 0x80)
        number >>= 7

    return bytes(reversed(bytes_list))



def calculate_crc32(data):
    return binascii.crc32(data) & 0xffffffff


def byte_concat(data, crc):
    return data + crc.to_bytes(4, 'little')


def midi_processing(midi_file_path):
    
    midi_file_name = midi_file_path.split('/')[-1]
    midi_file_size = os.stat(midi_file_path)[6]
    midi_file_pack_num = math.floor(midi_file_size / 1024) + 1
    midi_file_content = read_into_buffer(midi_file_path)
    
    midi_file_size_to_vla = int_to_vla_bytes(midi_file_size, 'little')
    midi_file_pack_num_to_vla = int_to_vla_bytes(midi_file_pack_num, 'little')
    
    midi_info = []
    midi_info.append(midi_file_name)
    midi_info.append(int(midi_file_size))
    midi_info.append(int(midi_file_pack_num))
    
    midi_info_chain = b""
    midi_info_chain += midi_file_size_to_vla
    midi_info_chain += midi_file_pack_num_to_vla
    for i in midi_file_name:
        midi_info_chain += str(i).encode()
    
    midi_info_chain += b'\0'
        
    return midi_info_chain, midi_file_content, midi_info
    

def socket_task(port):
    
    info_chain, sbuf, info_list = midi_processing(target_midi_file_path)
    print("midi processed")
    
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect((socket_tcp_address, port))
    
    client.send("midi".encode())
    recv = client.recvfrom(4)[0]
    
    if recv.decode() == ACK_CODE_RECV:    # VLA file size + VLA pack num + file name
        time.sleep(2)
        client.send(info_chain)
        
        '''There could be delay happen'''
        recv = client.recvfrom(4)[0]
                
        if recv.decode() == ACK_CODE_STRT:
            
            i = 0
            while i < info_list[-1]:
                
                time.sleep(0.35)
                
                if(i == (info_list[-1]-1)):
                    sbuf_slice = sbuf[i * 1024 : ]
                else:
                    sbuf_slice = sbuf[i * 1024 : (i+1) * 1024]                
                
                client.send(sbuf_slice)
                recv = client.recvfrom(4)[0]
                
                i = i + 1
        else:
            client.send("disc".encode())
            client.close()

    else:
        client.send("disc".encode())
        client.close()



if __name__ == "__main__":
    #midi_processing(target_midi_file_path)
    print("Task Start")
    socket_task(socket_tcp_port1)

