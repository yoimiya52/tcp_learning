## 关于TCP socket编程
```c++
int createTCPServer(int port){
    int s, yes = 1;
    struct sockaddr_in sa;
    if((s = socket(AF_INET, SOCK_STREAM,0)) == -1) return -1;

    setsockopt(s,SOL_SOCKET,SO_REUSEADDR, &yes, sizeof(yes));

    memset(&sa,0,sizeof(sa));

    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if(bind(s,(struct sockaddr *)&sa,sizeof(sa))==-1 ||
       listen(s, 511) == -1) 
    {
        close(s);
        return -1;
    }
    return s;
}

```
> 1. 创建一个TCP socket服务端口,关于函数
>  extern int socket (int __domain, int __type, int __protocol) __THROW;
>

