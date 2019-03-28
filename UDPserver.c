#include "UDPserver.h"

void (*message_handle)(int fd, char mbuf[]);

void read_cb(int fd, short event, void *arg)
{   
    threadpool_add_job(pool, (void*)pthread_handle_message, (void*)&(fd));
}

int init_socket(struct event *ev)
{
    int sock_fd;
    int ret;
    int flag = 1;
    struct sockaddr_in sin;

    /* Create endpoint */
    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket()");
        return -1;
    }

    /* Set socket option */
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) < 0)
    {
        perror("setsockopt()");
        return 1;
    }

    /* Set IP, port */
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr(SVR_IP);
    sin.sin_port = htons(SVR_PORT);

    /* Bind */
    if (bind(sock_fd, (struct sockaddr *)&sin, sizeof(struct sockaddr)) < 0)
    {
        perror("bind()");
        return -1;
    }
    else
    {
        printf("bind() success – [%s] [%u]\n", SVR_IP, SVR_PORT);
    }

    /* Init one event and add to active events */
    event_set(ev, sock_fd, EV_READ | EV_PERSIST, &read_cb, NULL);
    if (event_add(ev, NULL) == -1)
    {
        printf("event_add() failed\n");
    }

    return 0;
}

int UDPserver_run(int thread_num, int queue_max_num, void (*messag_handle)(int fd, char mbuf[]) )
{
    struct event ev;
    message_handle = messag_handle;

    /* Init. event */
    if (event_init() == NULL)
    {
        printf("event_init() failed\n");
        return -1;
    }

    /* Init pool */
    pool = threadpool_init(thread_num, queue_max_num);

    /* Init socket */
    if (init_socket(&ev) != 0)
    {
        printf("bind_socket() failed\n");
        return -1;
    } 

    /* Enter event loop */
    event_dispatch();

    printf("End!");
    return 0;
}

/*
pthread_handle_message – 线程处理 socket 上的消息收发
*/
void pthread_handle_message(int* sock_fd)
{
    struct timeval timeout={3,0};//3s
    setsockopt(*sock_fd,SOL_SOCKET,SO_SNDTIMEO,(const char*)&timeout,sizeof(timeout));
    //setsockopt(*sock_fd,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout,sizeof(timeout));

    char recvbuf[BUF_SIZE + 1] = {0};
    char sendbuf[BUF_SIZE + 1] = {0};
    int  ret;
    int  new_fd;
    struct sockaddr_in client_addr;
    socklen_t cli_len=sizeof(client_addr);

    new_fd=*sock_fd; 
    /* 开始处理每个新连接上的数据收发 */
    //bzero(recvbuf, BUF_SIZE + 1);
    //bzero(sendbuf, BUF_SIZE + 1);
    /* 接收客户端的消息 */
    ret = recvfrom(new_fd, recvbuf, BUF_SIZE, 0, (struct sockaddr *)&client_addr, &cli_len);
    if (ret==0) return;
    if (ret >= BUF_SIZE )
    {
        plog(LOG_ERROR,NULL);
        printf("recv too big %d\n",ret);
        fflush(stdout);
        return;
    }

    if (ret < 0 )
    {
        plog(LOG_ERROR,NULL);
        printf("fail: %d - '%s'\n", errno, strerror(errno));
        fflush(stdout);
        return;
    }

    plog(LOG_INFO,NULL);
    printf("socket %d - %s:%d - '%s'\n", new_fd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), recvbuf, ret);
    
    (*message_handle)(new_fd, recvbuf);

    sendto(new_fd, recvbuf, strlen(recvbuf), 0, (struct sockaddr*) &client_addr, cli_len);
    fflush(stdout); 
}


/* log */
void plog( int type, const unsigned char *str )
{
    time_t t;
    struct tm * lt;
    time (&t);//获取Unix时间戳。
    lt = localtime (&t);//转为时间结构。
    printf( "%d-%02d-%02d %02d:%02d:%02d",lt->tm_year+1900, (lt->tm_mon+1)%12, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);

    if( type==LOG_INFO ){
        printf(" [INFO] ");
    }
    else if( type==LOG_ERROR ){
        printf(" [ERROR] ");
    }
    if(str!=NULL){
        printf("%s", str);
    }
}