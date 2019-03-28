/*#ifndef __UDP_SERVER_H_
#define __UDP_SERVER_H_*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <event.h>
#include <event2/listener.h>
#include <unistd.h> //sleep()
#include <pthread.h>
#include <assert.h>
#include <time.h>


#include "threadpool.h"

#define SVR_IP "0.0.0.0"  //绑定的地址
#define SVR_PORT 8888     //绑定的端口
#define BUF_SIZE 512     //接受buf大小

#define LOG_INFO 0
#define LOG_ERROR 1

pthread_t thread;
pthread_attr_t attr;

struct threadpool *pool;
/*
pthread_handle_message – 线程处理 socket 上的消息收发
*/
void pthread_handle_message(int* sock_fd);

int UDPserver_run(int thread_num, int queue_max_num, void (*message_handle)(int fd, char mbuf[]));

void plog( int type, const unsigned char *str );