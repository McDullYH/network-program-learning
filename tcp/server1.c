
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>


#define LOG_INFO
#define LOG_DETAIL

#include "log.h"
#include "utils.h"
#include "handle.h"


int main(int argc,char* argv[])
{

  int ret=0;
  assert(ret==0);
  //here will exit
  //assert is should be XXXX, or will return 

  int listenFd,connectFd;

  pid_t pid;


  //here just for IPV4
  sockaddr_in serverAddr,clientAddr;

  socklen_t clientAddrLen = sizeof(clientAddr);

  memset(&serverAddr,0,sizeof(serverAddr));

  serverAddr.sin_family=AF_INET;

  serverAddr.sin_addr.s_addr=(INADDR_ANY);
  ret = inet_pton(AF_INET,"127.0.0.1",&serverAddr.sin_addr);
  assert(ret == 1);

  serverAddr.sin_port=htons(SERVER1_PORT);

  listenFd = socket(AF_INET, SOCK_STREAM,0);


  ret = bind (listenFd,(SA*)&serverAddr,sizeof(serverAddr));
  assert(ret==0);
  //should be 0, or return;


  LOGI("will listen\r\n");
  ret = listen(listenFd,16);
  assert(ret==0);

  // handle SIGCHLD forbiden child become zombies

  // TODO: in solaris,the interupt may not restart, so wo need write ourselves signal!

  // the better handle will handle all the zombies, which that bad handle can't do

  signal(SIGCHLD,server1_handle_SIGCHLD_better);
  while(1)
  {
    LOGI("enter in accept while\r\n");

    // in solaris, if parent process get a SIG, following accept function(慢系统调用函数) may return and set erron to EINTR,
    // we should to handle this(that is to say, call accept again! see <<apue>> P115)

    // TODO: if the TCP connect dead after client finish connect function and before
    // server call(or finish) accept, some bugs may happen, 
    // later we'll handle this

      if((connectFd= accept(listenFd, (SA*)&clientAddr, &clientAddrLen)) < 0)
      {
        // some OS will return EINTR when accept is interrupt by kernel (accept is a block function )
        // what interrupt can be a signal like SIGCHLD and so on we need handle
        if(errno == EINTR)
          continue;
        else
          LOGE("accept error\r\n");
      }
      
    if((pid=fork())==0)
    {
      LOGI("in child\r\n");
      close(listenFd);
      echoToClient(connectFd);
      close(connectFd);
      exit(0);
    }
    else
    {
      LOGI("in parent\r\n");
      close(connectFd);
    }
  }

  close(listenFd);
  return 0;
}
