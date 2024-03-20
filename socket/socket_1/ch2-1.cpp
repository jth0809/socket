#include "..\..\Common.h"

int main(int argc, char *argv[]){

    WSADATA wsa;
    
    if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
        return 1;
    
    printf("[알림] 윈속 초기화 성공\n");

    WSACleanip();
    return 0;
}