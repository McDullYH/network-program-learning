#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
#include <syslog.h>




#include "utils.h"

#define LOG_ERROR
#define LOG_DETAILE
#include "log.h"


int max(int a,int b)
{
  return a>b?a:b;
}


addrinfo* get_host_serv(const char* hostname,const char* service,int family,int socktype)
{
  addrinfo hints, *result;

  memset(&hints,0,sizeof(hints));
  hints.ai_flags|=AI_CANONNAME;
  hints.ai_family=family;
  hints.ai_socktype=socktype;


  int ret = getaddrinfo(hostname,service,&hints,&result); 
  if(ret!=0)
  {
    LOGE("getaddrinto error: %s\r\n",gai_strerror(ret));
    return NULL;
  }

  return result; 
}

void free_host_serv(addrinfo* result)
{
  freeaddrinfo(result);
  return;
}



int tcp_connect(const char* hostname, const char* service, socklen_t *addrlenp)
{

  if(hostname==NULL && service == NULL)
  {
    LOGE("host name and service all NULL\r\n");
    return -1;
  }

  addrinfo hints,*result,*resPtr;

  memset(&hints,0,sizeof(hints));

  //ipv4 or ipv6 is all ok
  hints.ai_family=AF_UNSPEC;
  //just for tcp
  hints.ai_socktype=SOCK_STREAM;

  int ret=getaddrinfo(hostname,service,&hints,&result);
  if(ret!=0)
  {
    LOGE("getaddrinto error: %s\r\n",gai_strerror(ret));
    return -1;
  }

  int connectFd;
  for(resPtr=result;resPtr!=NULL;resPtr=resPtr->ai_next)
  {
    connectFd=socket(resPtr->ai_family,resPtr->ai_socktype,resPtr->ai_protocol);
    if(connectFd<0)
      continue;

    // here no need to transform, it is exactly right
    // do not use sizeof(resPtr->ai_addr), use resPtr->ai_addrlen is well
    ret = connect(connectFd,resPtr->ai_addr,resPtr->ai_addrlen);

    // succeed
    if(ret == 0)
    {
      *addrlenp=resPtr->ai_addrlen;
      break;
    }

    close(connectFd);
    connectFd=-1;
  }


  freeaddrinfo(result);

  return connectFd;
}



int tcp_listen(const char* hostname, const char* service, socklen_t *addrlenp)
{
  if(hostname == NULL && service == NULL)
  {
    LOGE("host name and service all NULL\r\n");
    return -1;
  }

  addrinfo hints, *result, *resPtr;

  memset(&hints,0,sizeof(hints));
  hints.ai_flags|=AI_PASSIVE;
  hints.ai_family=AF_UNSPEC;
  hints.ai_socktype=SOCK_STREAM;

  int ret = getaddrinfo(hostname,service,&hints,&result);
  if(ret!=0)
  {
    LOGE("getaddrinfo error %s\r\n",gai_strerror(ret));
    return -1;
  }

  int listenFd;

  for(resPtr=result;resPtr!=NULL;resPtr=resPtr->ai_next)
  {
    listenFd=socket(resPtr->ai_family,resPtr->ai_socktype,resPtr->ai_protocol);
    if(listenFd<0)
      continue;

    const int on =1;

    // set SO_REUSEADDR will fix a program:
    // sometime a parent process exit but son process not exit,
    // the sons will still use the parents port
    // then you can't bind the same port when parent process start up again
    // but if you set SO_REUSEADDR, we can start up successfully
    setsockopt(listenFd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

    ret = bind (listenFd,resPtr->ai_addr,resPtr->ai_addrlen);
    if(ret==0)
    {
      *addrlenp=resPtr->ai_addrlen;
      break;
    }

    close(listenFd);
    listenFd=-1;
  }


  freeaddrinfo(result);

  if(resPtr==NULL)
    return listenFd;

  ret = listen(listenFd,16);
  if(ret != 0)
    listenFd=-1;

  return listenFd;
}




#define MAXFD 64
int daemonize(const char* pname, int facility)
{
  pid_t pid;

  pid=fork();
  if(pid<0)
    return -1;
  else if(pid>0)
    _exit(0);
  //use _exit not exit bcs _exit do not clean up

  if(setsid()<0);
  return -1;

  signal(SIGHUP,SIG_IGN);

  pid=fork();
  if(pid<0)
    return -1;
  else if(pid>0)
    _exit(0);

  chdir("/");

  for (int i =0; i<MAXFD; i++)
  {
    close(i);
  }

  open("/dev/NULL",O_RDONLY);
  open("/dev/NULL",O_RDWR);
  open("/dev/NULL",O_RDWR);
  //The file descriptor returned by a successful call will be the lowest-numbered  file  descriptor  not currently open for the process
  
  openlog(pname,LOG_PID,facility);

  return 0;
}

















