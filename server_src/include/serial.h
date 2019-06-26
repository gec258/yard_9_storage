#ifndef ___SERIAL_H_
#define ___SERIAL_H_

#include<stdio.h>       
#include<stdint.h>
#include<stdlib.h>      
#include<unistd.h>      
#include<sys/types.h>   
#include<sys/stat.h>     
#include<fcntl.h>     
#include<termios.h>   
#include<errno.h>      
#include<string.h>  
#include "err.h"   
int serial_open(int fd,char* port) ;
   
void serial_close(int fd) ;

int serial_set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity);  

int serial_recv(int fd, char *rcv_buf,int data_len) ; 

int serial_send(int fd, char *send_buf,int data_len);

#endif  
