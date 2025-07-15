#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/select.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

#include "chatlib.h"

void disableRawModeAtExit(void);
int setRawMode(int fd,int enable){
    static struct termios orig_termios;
    static int atexit_registered = 0;
    static int rawmode_is_set = 0;

    struct termios raw;

    if(enable == 0){
        if(rawmode_is_set && tcsetattr(fd,TCSAFLUSH,&orig_termios) != -1)
            rawmode_is_set = 0;
        return 0;
    }

    if(!isatty(fd)) goto fatal;

    raw = orig_termios;

    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_cflag &= ~(CS8);
    raw.c_lflag &= ~(ECHO | ICANON |IEXTEN);
    raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0;
    if(tcsetattr(fd,TCSAFLUSH,&raw) < 0) goto fatal;
    rawmode_is_set = 1;
    return 0;
fatal:
    errno = ENOTTY;
    return -1;

}

void disableRawModeAtExit(void){
    setRawMode(STDIN_FILENO,0);
}

void terminalCleanCurrentLine(void){
    write(fileno(stdout),"\e[2k]",4);
}

void terminalCursorAtLineStart(void){
    write(fileno(stdout),"\r",1);
}


#define IB_MAX 128

struct InputBuffer {
    char buf[IB_MAX];
    int len;
};

#define IB_ERR 0
#define IB_OK 1
#define IB_GOTLINE 2

int inputBufferAppend(struct InputBuffer *ib,int c){
    if(ib->len >= IB_MAX)  return IB_ERR;

    ib->buf[ib->len] = c;
    ib->len++;
    return IB_OK;
}

void inputBufferHide(struct InputBuffer *ib);
void inputBufferShow(struct InputBuffer *ib);

int inputBufferFeedChar(struct InputBuffer *ib, int c) {
    switch(c) {
    case '\n':
        break;          
    case '\r':
        return IB_GOTLINE;
    case 127:           
        if (ib->len > 0) {
            ib->len--;
            inputBufferHide(ib);
            inputBufferShow(ib);
        }
        break;
    default:
        if (inputBufferAppend(ib,c) == IB_OK)
            write(fileno(stdout),ib->buf+ib->len-1,1);
        break;
    }
    return IB_OK;
}

void inputBufferHide(struct InputBuffer *ib) {
    (void)ib; // Not used var, but is conceptually part of the API.
    terminalCleanCurrentLine();
    terminalCursorAtLineStart();
}

void inputBufferShow(struct InputBuffer *ib) {
    write(fileno(stdout),ib->buf,ib->len);
}

void inputBufferClear(struct InputBuffer *ib) {
    ib->len = 0;
    inputBufferHide(ib);
}

int main(int argc, char **argv){
    if(argc != 3) {
        printf("Usage: %s <host> <port>\n", argv[0]);
        exit(1);
    }
    int s = TCPConnect(argv[1],atoi(argv[2]),0);
    if(s==-1){
        perror("Connecting to server");
        exit(1);
    }
    setRawMode(fileno(stdin),1);
    fd_set readfds;
    int stdin_fd = fileno(stdin);

    struct InputBuffer ib;
    inputBufferShow(&ib);

    while(1){
        FD_ZERO(&readfds);
        FD_SET(s,&readfds);
        FD_SET(stdin_fd,&readfds);
        int maxfd = s > stdin_fd ? s: stdin_fd;
        int num_events = select(maxfd+1,&readfds,NULL,NULL,NULL);
        if(num_events == -1){
            perror("select() error");
            exit(1);
        }else if(num_events){
            char buf[128];
            if(FD_ISSET(s,&readfds)){
                ssize_t count = read(s,buf,sizeof(buf));
                if(count <=0){
                    printf("connection lost\n");
                    exit(1);
                }
                inputBufferHide(&ib);
                write(fileno(stdout),buf,count);
                inputBufferShow(&ib);
            }else if(FD_ISSET(stdin_fd,&readfds)){
                ssize_t count = read(stdin_fd,buf,sizeof(buf));

                for(int j=0;j<count;j++){
                    int res = inputBufferFeedChar(&ib,buf[j]);
                    switch(res) {
                        case IB_GOTLINE:
                            inputBufferAppend(&ib,'\n');
                            inputBufferHide(&ib);
                            write(fileno(stdout),"you>",5);
                            write(s,ib.buf,ib.len);
                            inputBufferClear(&ib);
                            break;
                        case IB_OK:
                            break;
                }
            }
        }

    }
}
    close(s);
    return 0;
}