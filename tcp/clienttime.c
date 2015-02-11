
#include <unistd.h>
#include <arpa/inet.h>
#include "utils.h"

#define LOG_ERROR

#define LOG_INFO
#define LOG_DEBUG
#define LOG_ERROR
#define LOG_WARN
#define LOG_DETAILE
#include "log.h"


typedef sockaddr SA;

int main(int argc, char* argv[])
{
  socklen_t sslen;
  int connectFd;

  if(argc==2)
    connectFd=tcp_connect(NULL,argv[1],&sslen);
  if(argc==3)
    connectFd=tcp_connect(argv[1],argv[2],&sslen);

  if(connectFd<0)
  {
    LOGE("tcp connect failed\r\n");
    return -1;
  }

  sockaddr_storage ss;
  sockaddr *pss= (sockaddr*)&ss;

  if(getpeername(connectFd,pss,&sslen)!=0)
  {
    LOGE("getpeername  failed\r\n");
    return -1;
  }


  char *str =new char[sslen];

  // or sizeof(ss)?
  if(pss->sa_family==AF_INET)
  {
    // attention!!  here  inet_ntop need sin_addr !!
    // it is really to wrap the inet_ntop!
    sockaddr_in* pad4=(sockaddr_in*)pss;
    LOGI("ip is %s\r\n",inet_ntop(pss->sa_family,&pad4->sin_addr,str,sslen));
    LOGI("port is %d\r\n",ntohs(pad4->sin_port));
  }
  else if(pss->sa_family==AF_INET6)
  {
    sockaddr_in6* pad6=(sockaddr_in6*)pss;
    LOGI("ip is %s\r\n",inet_ntop(pss->sa_family,&pad6->sin6_addr,str,sslen));
    LOGI("port is %d\r\n",ntohs(pad6->sin6_port));
  }
  else
  {
    LOGW("clinet is not ipv4 or ipv6\r\n");
  }
  delete[] str;

  char buffer[1024];

  ssize_t readSize = read(connectFd,buffer,1024);

  if(readSize>0)
  {
    write(STDOUT_FILENO,buffer,readSize);
  }

  close(connectFd);
  return 0;
}
