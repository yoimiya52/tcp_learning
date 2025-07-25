## 关于TCP socket接口
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
>
> ``````c++
> extern int socket (int __domain, int __type, int __protocol) __THROW
> ``````
>
> - domain  通信域，AF_INET指的是IPV4通信。
> - type 套接字的类型，SOCK_STREAM指的是面向流的套接字，通常用于TCP通信
> - protocol 协议，对于SOCK_STREAM，通常指定0来使用默认协议（即TCP协议）
>
> - 返回值：成功时一个非负整数，套接字文件描述符。这个文件描述符用于后续的套接字操作，比如绑定、监听、连接、发送、接受数据；失败时返回-1，并设置error变量来指示具体的错误。
>   - EACCES: 权限被拒绝。
>   - EAFNOSUPPORT: 指定的地址族不受支持。
>   - EINVAL: 无效的参数。
>   - EMFILE: 进程的文件描述符达到上限。
>   - ENFILE: 系统的文件描述符达到上限。
>   - ENOBUFS 或 ENOMEM: 内存不足。
