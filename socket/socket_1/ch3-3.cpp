#include "..\..\Common.h"
#define TESTNAME "www.google.com"

bool GetIPAddr(const char *name, struct in_addr *addr);
bool GetDomainName(struct in_addr addr, char *name, int namelen);


int main(int argc, char *argv[]){

    WSADATA wsa;
    
    if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
        return 1;
    
    printf("%s\n", TESTNAME);

    struct in_addr addr;
    if (GetIPAddr(TESTNAME, &addr)){
        char str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr, str, sizeof(str));
        pritnf("%s\n", str);

        char name[256];
        if(GetDomainName(addr,name,sizeof(name))){
            printf("%s\n", name);
        }
    }

    WSACleanup();
    return 0;
}

bool GetIPAddr(const char *name, struct in_addr *addr){
    struct hostent *ptr = gethostbyname(name);
    if (ptr == NULL){
        err_display("gethostbyname()");
        return false;
    }
    if(ptr -> h_addrtype != AF_INET)
        return false;
    memcpy(addr, ptr->h_addr, ptr -> h.length);
    return true;
}

bool GetDomainName(struct in_addr addr, char *name, int namelen){

    struct hostent *ptr = gethostbyaddr((const char *) &addr,
        sizeof(addr), AF_INET);
    if(ptr == NULL){
        err_display("gethostbyaddr()");
        return false;
    }
    if(ptr -> h_addrtype != AF_INET)
        return false;

    strncpy(name, ptr -> h_name, namelen);
    return true;
}