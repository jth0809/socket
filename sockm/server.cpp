#include "./c.h"

char *SERVERIP = (char *)"223.195.109.32";
#define SERVERPORT 9000
#define BUFSIZE 256
int count = 0;
int seq = 0;
int i = 0;
pthread_mutex_t mutex;
struct __attribute__((packed)) msg{
        int seq = 0;
        char code = 'N';
        char data[BUFSIZE]; 
    };
struct msg msg;
struct msg msgb[7];

pthread_cond_t writeCond;
pthread_cond_t readCond;
pthread_mutex_t writeMutex;
pthread_mutex_t readMutex;
int writeDone = 0;
int readDone = 0;


void *ProcessClient(void *arg){
    count += 1;
    
    int retval;
	SOCKET client_sock = (SOCKET)(long long)arg;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	socklen_t addrlen;

    addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr *)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

    if (count >= 3){
        pthread_mutex_lock(&readMutex);
		readDone = 1;
		pthread_mutex_unlock(&readMutex);
		pthread_cond_signal(&readCond);
    }

    while(1){
        if(count == 3){
            pthread_mutex_lock(&writeMutex);
		    while (writeDone == 0)
			    pthread_cond_wait(&writeCond, &writeMutex);
		    writeDone = 0;
		    pthread_mutex_unlock(&writeMutex);

            if (msg.code == 'Q') break;
            retval = send(client_sock, (char*)&msg, sizeof(msg),0);
            retval = recv(client_sock, NULL, sizeof(int), 0);
            //printf("[TCP4/%s:%d] %d %c %s \n", addr, ntohs(clientaddr.sin_port), msg.seq, (char)msg.code, (char*)&msg.data);

            pthread_mutex_lock(&readMutex);
		    readDone = 1;
		    pthread_mutex_unlock(&readMutex);
		    pthread_cond_signal(&readCond);
        }
    }


    close(client_sock);
    printf("%s:%d\n",addr, ntohs(clientaddr.sin_port));
    return 0;
}

void *server(void *arg){
    int retval;

	SOCKET sock  = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serveraddr1;
    memset(&serveraddr1, 0, sizeof(serveraddr1));
    serveraddr1.sin_family = AF_INET;
    inet_pton(AF_INET, SERVERIP, &serveraddr1.sin_addr);
    serveraddr1.sin_port = htons(SERVERPORT);
    retval = connect(sock, (struct sockaddr *)&serveraddr1, sizeof(serveraddr1));
    if (retval == SOCKET_ERROR) err_quit("connect()");
    int num = 0;

    while(1){
        if(count >= 3){
            pthread_mutex_lock(&readMutex);
		    while (readDone == 0)
			    pthread_cond_wait(&readCond, &readMutex);
		    readDone = 0;
		    pthread_mutex_unlock(&readMutex);
            //printf("[TCP4/:%d] %d %c %s \n", ntohs(serveraddr1.sin_port), msg.seq, (char)msg.code, (char*)&msg.data);
            retval = send(sock, (char*)&msg, sizeof(msg),0);
            retval = recv(sock, (char*)&msg, sizeof(msg), MSG_WAITALL);
            if(msg.code == 'Q') {
                break;
            }

            pthread_mutex_lock(&writeMutex);
		    writeDone = 1;
		    pthread_mutex_unlock(&writeMutex);
		    pthread_cond_signal(&writeCond);
        }
    }

    close(sock);
    return 0;
}


int main(int argc, char *argv[]){
    int retval;

    SOCKET listen_sock  = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) err_quit("socket()");
    
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = bind(listen_sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("bind()");

    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR) err_quit("listen()");

    SOCKET client_sock;
    struct sockaddr_in clientaddr;
    socklen_t addrlen;
    pthread_t tid;
    pthread_create(&tid, NULL, server, NULL);
    
    while(1){
        addrlen = sizeof(clientaddr);
        client_sock = accept(listen_sock, (struct sockaddr *)&clientaddr, &addrlen);
        if(client_sock == INVALID_SOCKET){
            err_display("accept()");
            break;
        }
        
        char addr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
        printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			addr, ntohs(clientaddr.sin_port));

        retval = pthread_create(&tid, NULL, ProcessClient,
            (void *)(long long)client_sock);
        if (retval != 0) {close(client_sock);}
    }

    close(listen_sock);
    return 0;

    }