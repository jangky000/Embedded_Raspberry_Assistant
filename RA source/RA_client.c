
#include<stdio.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<string.h>

#define MSG_MAX 256 // 바이트 단위
#define SERVER_PORT 8578

char send_buf[MSG_MAX];
char recv_buf[MSG_MAX];

int DATA_SEND_RECV(int sock_flag, char *msg)
{
    memset(send_buf, 0x00, MSG_MAX);
    strcpy(send_buf, msg);
    // msg_buf[MSG_MAX-1] = '\0';
    write(sock_flag, send_buf, strlen(send_buf));
    
    
    memset(recv_buf, 0x00, MSG_MAX);
    read(sock_flag, recv_buf, sizeof(recv_buf)-1);
    recv_buf[MSG_MAX-1] = '\0';
    printf("%s\n", recv_buf);
    
    //cmd name -> file?
    
    while(strcmp(recv_buf, "done"))
    {
        memset(recv_buf, 0x00, MSG_MAX);
        read(sock_flag, recv_buf, sizeof(recv_buf)-1);
        recv_buf[MSG_MAX-1] = '\0';
        printf("%s\n", recv_buf);
    }
    
    
    sleep(10); // nuknukhi
    memset(send_buf, 0x00, MSG_MAX);
    strcpy(send_buf, "연결 해제");
    // msg_buf[MSG_MAX-1] = '\0';
    write(sock_flag, send_buf, strlen(send_buf));
}

int main(int argc, char *argv[])
{
    int i;
    int sock_flag, conn_flag;
    struct sockaddr_in server_addr;

    if( (sock_flag = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 )
    {
        printf("Socket 생성 실패\n");
        exit(1);
    }
    else
        printf("socket 생성 성공\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("169.254.33.230");
    server_addr.sin_port = htons(SERVER_PORT);

    if( (connect(sock_flag, (struct sockaddr*)&server_addr, sizeof(server_addr))) < 0 )
    {
        printf("서버 - 클라이언트 연결 실패\n");
        exit(2);
    }
    else
        printf("서버 - 클라이언트 연결 성공\n");
    
    char msg[100]={0, };
    for(i=1; i<argc; i++)
    {
        sprintf(msg, "%s %s", msg, argv[i]);
    }
    
    DATA_SEND_RECV(sock_flag, msg+1);
    close(sock_flag);
    printf("\n소켓 해제 성공\n");
    return 0;
}
