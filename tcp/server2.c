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

#define BUFFER_SIZE 1024


int main(int argc,char* argv[])
{

  int ret=0;
  assert(ret==0);

  int maxFd, listenFd, clientFd,clientFds[FD_SETSIZE];

  // for specify the last usable client fd position in clientFds 
  int lastFdIndex;

  for(int i=0;i<FD_SETSIZE;i++)
  {
    // init clientFds
    clientFds[i]=-1;
  }

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

  // as select will change set everytime,
  // so we need to update the fd_set we  care about everytime before we call select
  // that is before select, we do rset_for_select = rset_we_care;

  fd_set rset_for_select, rset_we_care;


  ret = bind (listenFd,(SA*)&serverAddr,sizeof(serverAddr));
  assert(ret==0);


  LOGI("will listen\r\n");
  ret = listen(listenFd,16);
  assert(ret==0);

  FD_ZERO(&rset_we_care);
  FD_SET(listenFd,&rset_we_care);
  maxFd=listenFd;

  int ready_count=-1;


  char *buffer = new char[BUFFER_SIZE];
  ssize_t readSize=0;

  while(1)
  {
    rset_for_select = rset_we_care;

    ready_count=select(maxFd+1,&rset_for_select,NULL,NULL,NULL);

    if(FD_ISSET(listenFd,&rset_for_select))
    {

      if((clientFd = accept(listenFd, (SA*)&clientAddr, &clientAddrLen)) < 0)
      {
        if(errno == EINTR)
          continue;
        else
        {
          LOGE("accept error\r\n");
          break;
        }
        --ready_count;
      }
      else
      {
        int i;
        //here just traverse to find a fit position, for just show how to use select
        for( i=0;i<FD_SETSIZE;++i)
        {
          if(clientFds[i]==-1)
          {
            clientFds[i]=clientFd;
            break;
          }
        }
        if(FD_SETSIZE==i)
        {
          LOGE("too many clients\r\n");
        }

        FD_SET(clientFds[i],&rset_we_care);

        if(clientFds[i]>maxFd)
        {
          // in this code maxFd only become bigger never become smaller,
          // use some data struct can forbid this,
          // but it is just for show how to use select, so, let it go
          maxFd=clientFds[i];
        }

        if(i > lastFdIndex)
        {
          // the lastFdIndex also only become bigger,
          // use some data struct can forbid this
          // but it is just for show how to use select, so, let it go
          lastFdIndex=i;
        }

      }
      if(--ready_count<=0)
        continue;
    }

    for(int i=0;i<=lastFdIndex;++i)
    {
      if(clientFds[i]==-1)
      {
        continue;
      }
      if(FD_ISSET(clientFds[i],&rset_for_select))
      {
        // here we must insure that we have call only one read once in a select
        // if we server have two or more read , but client just send data once, 
        // we server will hang on in second or later read, then we can't serve any other client!

        readSize = read(clientFds[i],buffer,BUFFER_SIZE);
        if(readSize == 0)
        {
          // EOF come
          close(clientFds[i]);
          FD_CLR(clientFds[i],&rset_we_care);
          clientFds[i]=-1;
        }
        else
        {

          write(fileno(stdout),buffer,readSize);
          write(clientFds[i],buffer,readSize);
        }
        if(--ready_count == 0)
        {
          break;
          // to another select
        }
      }
    }
  }
  close(listenFd);
  delete [] buffer;
  return 0;
}
