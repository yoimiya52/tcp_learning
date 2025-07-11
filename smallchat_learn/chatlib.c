#define _POSIX_C_SOURCE 200112L
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int socketSetNonBlockNoDelay(int fd){
    int flags,yes = 1;
    if((flags = fcntl(fd,F_GETFL,0)) == -1) return -1;

    if(fcntl(fd,F_SETFL,flags | O_NONBLOCK) == -1) return -1;

    setsockopt(fd, IPPROTO_TCP,TCP_NODELAY,&yes,sizeof(yes));
    return 0;
}

int createTCPServer(int port){
    int s, yes = 1;
    struct sockaddr_in sa;
    if((s = socket(AF_INET, SOCK_STREAM,0)) == -1) return -1;

    setsockopt(s,SOL_SOCKET,SO_REUSEADDR, &yes, sizeof(yes));

    memset(&sa,0,sizeof(sa));

    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = htonal(INADDR_ANY);
    
    if(bind(s,(struct socaddr *)&sa,sizeof(sa))==-1 ||
       listen(s, 511) == -1) 
    {
        close(s);
        return -1;
    }
    return s;
}

int TCPConnect(char *addr, int port, int nonblock){
    int s, retval = -1;
    struct addrinfo hints, *servinfo, *p;
    
    char portstr[6];
    snprintf(portstr,sizeof(portstr),"%d",port);
    memset(&hints,0,sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if(getaddrinfo(addr,portstr,&hints,&servinfo) != 0) return -1;

    for(p = servinfo;p!=NULL;p = p->ai_next){
        if((s = socket(p->ai_family,p->ai_socktype,p->ai_protocol))==-1) continue;

        if(nonblock && socketSetNonBlockNoDelay(s) == -1){
            close(s);
            break;
        }

        if(connect(s,p->ai_addr,p->ai_addrlen) == -1){
            if(errno == EINPROGRESS && nonblock) return s;
            close(s);
            break;
        }

        retval = s;
        break;
    }
    freeaddrinfo(servinfo);
    return retval;
}


int acceptClient(int server_socket){
    int s;
    while(1){
        struct sockaddr_in sa;
        socklen_t slen = sizeof(sa);
        s = accept(server_socket,(struct sockaddr*)&sa,&slen);
        if(s==-1){
            if(errno == EINTR) continue;
            else return -1;
        }
        break;
    }
    return s;
}

void *chatMalloc(size_t size){
    void *ptr = malloc(size);
    if(ptr == NULL){
        perror("Out of memory");
        exit(1);
    }
    return ptr;
}

void *chatRealloc(void *ptr, size_t size){
    ptr = realloc(ptr,size);
    if(ptr == NULL){
        perror("Out of memory");
        exit(1);
    }
    return ptr;
}