#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/select.h>
#include <unistd.h>

#include <chatlib.h>

#define MAX_CLIENTS 100
#define SERVER_PORT 7711

struct client
{
    int fd;
    char *name;
};

struct chatState
{
    int serversock;
    int numclients;
    int maxclient;
    struct client *clients[MAX_CLIENTS];
};

struct chatState *Chat;

struct client *createClient(int fd){
    char name[32];
    int namelen = snprintf(name,sizeof(name), "user:%d",fd);
    struct client *c = chatMalloc(sizeof(*c));
    c->fd = fd;
    c->name = chatMalloc(namelen + 1);
    memcpy(c->name,name,namelen);
    assert(Chat->clients[c->fd]==NULL);
    Chat->clients[c->fd] = c;

    if(c->fd > Chat->maxclient) Chat->maxclient = c->fd;
    Chat->numclients++;
    return c;
}

void freeClient(struct client *c){
    free(c->name);
    close(c->fd);
    Chat->clients[c->fd] = NULL;
    Chat->numclients--;
    if(Chat->maxclient == c->fd){
        int j;
        for (j=Chat->maxclient-1;j>=0; j--){
            if(Chat->clients[j] !=NULL){
                Chat->maxclient = j;
                break;
            }
        }
        if(j==-1) Chat->maxclient = -1;
    }

    free(c);
}

void initChat(void){
    Chat = chatMalloc(sizeof(*Chat));
    memset(Chat,0,sizeof(*Chat));
    Chat->maxclient = -1;
    Chat->numclients = 0;
    Chat->serversock = createTCPServer(SERVER_PORT);

    if(Chat->serversock == -1){
        perror("Creating listening socket");
        exit(1);
    } 
}


void sendMsgToAllClientsBut(int excluded,char *s,size_t len){
    for (int j = 0;j<= Chat->maxclient; j++){
        if(Chat->clients[j] == NULL || Chat->clients[j]->fd == excluded) continue;

        write(Chat->clients[j]->fd,s,len);
    }
}

int main(void){
    initChat();

    while(1){
        fd_set readfds;
        struct timeval tv;
        int retval;

        FD_ZERO(&readfds);

        FD_SET(Chat->serversock,&readfds);
        for(int j=0;j<=Chat->maxclient;j++){
            if(Chat->clients[j]) FD_SET(j,&readfds);
        }

        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int maxfd = Chat->maxclient;
        if(maxfd < Chat->serversock) maxfd = Chat->serversock;
        retval = select(maxfd+1,&readfds,NULL,NULL,&tv);

        if(retval == -1){
            perror("select() error");
            exit(1);
        }else if(retval){
            if(FD_ISSET(Chat->serversock,&readfds)){
                int fd = acceptClient(Chat->serversock);
                struct client *c = createClient(fd);
                char *welcome_msg = "Welcome to the QQ chat!\n Use /name <name> to set your name.\n";
                write(c->fd,welcome_msg,strlen(welcome_msg));
                printf("Connected client fd=%d\n", fd);
        }

        char readbuf[256];
        for(int j=0;j<=Chat->maxclient;j++){
            if(Chat->clients[j] == NULL) continue;
            if(FD_ISSET(j,&readfds)){
                int nread = read(j,readbuf,sizeof(readbuf)-1);
                if(nread <=0){
                    printf("Disconnected client fd=%d,name=%s\n",j,Chat->clients[j]->name);
                    freeClient(Chat->clients[j]);
                }else{
                    struct client *c =  Chat->clients[j];
                    readbuf[nread] = 0;

                    if(readbuf[0] == '/'){
                        char *p;
                        p = strchr(readbuf,'\r'); if(p) *p = 0;
                        p = strchr(readbuf,'\n'); if(p) *p = 0;

                        char *arg = strchr(readbuf,' ');
                        if(arg){
                            *arg = 0;
                            arg++;
                        }
                        if(!strcmp(readbuf,"/name") && arg){
                            free(c->name);
                            int namelen = strlen(arg);
                            c->name = chatMalloc(namelen +1);
                            memcpy(c->name,arg,namelen+1);

                        }else{
                            char *errmsg = "Unsupported command.\n";
                            write(c->fd,errmsg,strlen(errmsg));
                        }

                    }else{
                        char msg[256];
                        int msglen = snprintf(msg,sizeof(msg),"%s>%s",c->name,readbuf);

                        if(msglen >= (int)sizeof(msg))
                        msglen = sizeof(msg) -1;
                        printf("%s",msg);
                        sendMsgToAllClientsBut(j,msg,msglen);
                    }

                }
            }
        }

    }else{
        // Timeout, do nothing

    }

}
return 0;
}

