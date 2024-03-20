#include "..\..\Common.h"

int main(int argc, char *argv[]){

    WSADATA wsa;
    
    if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
        return 1;

    const char *ipv4test = "147.46.114.78"
    
    struct in_addr ipv4num;
    inet_pton(AF_INET, ipv4test, &ipv4num);
    printf("%s\n", ipv4test)

    char ipv4str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ipv4num, ipv4str, sizeof(ipv4str));
    printf("%#x\n",ipv4num.s_addr)
    const char ipv6test = "2001:0230:abcd:ffab:0023:eb00:ffff:1111"
    
    struct in6_addr ipv6num;
    inet_pton(AF_INET6, ipv6test, &ipv6num);
    for(int i =0; i<16; i++)
        printf("%02x", ipv6num.s6addr[i]);
    printf("\n");
    
    char ipv6str[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &ipv6num, ipv6str, sizeof(ipv6str));
    printf("%s\n",ipv6str);

    WSACleanup();
    return 0;
}