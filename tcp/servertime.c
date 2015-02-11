
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <string.h>
#include "utils.h"

#define LOG_INFO
#define LOG_DEBUG
#define LOG_ERROR
#define LOG_WARN
#define LOG_DETAILE
#include "log.h"


// just give port is all OK
int main(int argc,char* argv[])
{

  socklen_t addrlen;
  int listenFd;

  if(argc==2)
    listenFd=tcp_listen(NULL,argv[1],&addrlen);
  if(argc==3)
    listenFd=tcp_listen(argv[1],argv[2],&addrlen);

  if(listenFd<0)
  {
    LOGE("tcp_listen failed\r\n");
    return -1;
  }

  int acceptFd=-1;
  sockaddr_storage clientAddr;
  sockaddr *psa=(sockaddr*)&clientAddr;
  socklen_t clientAddrLen;

  time_t ticks;
  char* strTime;

  while(true)
  {
    acceptFd=accept(listenFd,psa,&clientAddrLen);

    if(acceptFd<0)
      continue;

    char *str=new char[clientAddrLen];

    // or sizeof(ss)?
    if(psa->sa_family==AF_INET)
    {
      sockaddr_in* pad4=(sockaddr_in*)psa;
      LOGI("connect from %s\r\n",inet_ntop(psa->sa_family,&pad4->sin_addr,str,clientAddrLen));
      LOGI("port is %d\r\n",ntohs(pad4->sin_port));
    }
    else if(psa->sa_family==AF_INET6)
    {
      sockaddr_in6* pad6=(sockaddr_in6*)psa;
      LOGI("connect from %s\r\n",inet_ntop(psa->sa_family,&pad6->sin6_addr,str,clientAddrLen));
      LOGI("port is %d\r\n",ntohs(pad6->sin6_port));
    }
    else
    {
      // here if we use g++ -m32 in 64bits system, the first client connect to
      // this server will show following, i don't know why now
      LOGW("clinet is not ipv4 or ipv6\r\n");
    }

    delete [] str;

    ticks=time(NULL);
    strTime=ctime(&ticks);
    write(acceptFd,strTime,strlen(strTime));
    close(acceptFd);
  }

  return 0;
}
