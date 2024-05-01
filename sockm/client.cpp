#include "./c.h"

char *SERVERIP = (char *)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE 256
struct __attribute__((packed)) msg{
        int seq;
        char code;
        char data[BUFSIZE]; 
    };
struct msg msg;
int main(int argc, char *argv[]){
    int retval;
    if (argc > 1) SERVERIP = argv[1];

    SOCKET sock  = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) err_quit("socket()");
    
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = connect(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("connect()");

    char buf[BUFSIZE];
    int len;

    while(1){
        retval = recv(sock, (char*)&msg, sizeof(msg), MSG_WAITALL);
        printf("\n%d %c %s\n",  msg.seq, (char)msg.code, (char*)&msg.data);
        retval = send(sock, &len, sizeof(int), 0);
        printf("[TCP 클라이언트] %d바이트를 보냈습니다.\n", retval);
    }

    close(sock);
    return 0;

    }