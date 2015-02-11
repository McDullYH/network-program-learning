
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/select.h>

#include "utils.h"
#include "handle.h"

#define LOG_INFO
#define LOG_DEBUG
#define LOG_ERROR
#define LOG_DETAILE
#include "log.h"

#define BUFFER_SIZE 1024



/*
   here the echoToClient and sendToServer all trans text data, so, no data error
   happened, 
   but if we want to trans binary data, data errors will happened between big
   endian computer and little endian computer

   Two way can fixed it:
   1. let all the binary data trans in text way
   2. or specify the data length, big or little endian when we trans the data
 */
int echoToClient(int fd)
{

  char *buffer=new char[BUFFER_SIZE];
  ssize_t readSize=0;

  while(true)
  {
    readSize = read(fd,buffer,BUFFER_SIZE);
    if(readSize>0)
    {
      LOGI("client trans %s\r\n",buffer);
      write(fd,buffer,readSize);
    }
    else if(errno == EINTR)
    {
      LOGD("EINTR happend\r\n");
    }
    else
    {
      LOGD("read nothing\r\n");
      break;
    }
  }
  delete [] buffer;
  return 0;
}

int sendToServer_01(FILE* pf,int fd)
{
  char *buffer=new char[BUFFER_SIZE];

  char *p=NULL;
  ssize_t writeSize;
  ssize_t readSize;

  while(true)
  {
    // here we block in fgets
    // if now server's process dead or server shutdown, it(the server)  will send a FIN, 
    // but client can't recieve the FIN and response correctly right now,
    // so bugs happened. detail is:
    // now we continue input something for fget,
    // then the write send what we input
    // then the server get what we input and send a RST back
    // then we client call read, and this time FIN was in read queue,  and the  RST may not in
    // if RST not in, the read will get a EOF, else the read will set errno to ECONNRESET
    // now the client not exist, we client continu to fgets and write(send) data
    // but write a fd which has recieved a RST will triggle a SIGPIPE signal,
    // which default behavior is to terminal current process,
    // that's why when we input twice then the client exit itself after the server process dead

    // that's 
    // 1 we need to use select and poll to recieve the FIN immediately
    // 2 we need to handle the SIGPIPE

    //  sendToServer_02(FILE* pf,int fd) will fix 1


    p=fgets(buffer,BUFFER_SIZE,pf);
    if(!p)
      break;

    //+1 for send last '\0'
    writeSize = write(fd,buffer,strlen(buffer) + 1);
    assert(writeSize>=0);


    // if now server crash or network crash, 
    // then it(server) can't send anything or client can't recieve anything from network
    // we'll hang on in read for a long while
    // to solve this, we have two method
    // 1 set a timeout for read
    // 2 set a option  SO_KEEPALIVE

    readSize = read(fd,buffer,BUFFER_SIZE);
    LOGD("readSize is %d\r\n",readSize);
    if(readSize >0)
    {
      fputs(buffer,stdout);
    }
    else if(errno == EINTR)
    {
      LOGD("EINTR happend\r\n");
    }
    else
    {
      LOGE("read nothing error is %s \r\n",strerror(errno));
      break;
    }
  }

  delete [] buffer;

  return 0;
}


#if 0
子进程结束，会被设置为僵尸状态，因为*nix的设计是子进程退出后还要保留一些信息
让父进程通过wait*函数获知这些信息，
所以此时子进程还没有完全“死”，我们称之为僵尸状态，留给父进程获取信息的僵尸状态

同时，子进程退出变为僵尸状态的时候，父进程会收到一个信号 SIGCHLD
所以，我们编写这个信号处理函数(仅仅wait一下即可)，就可以让子进程“安心的死掉了”
#endif

void server1_handle_SIGCHLD_bad(int signo)
{
  pid_t  pid;
  int stat;

  pid=wait(&stat);

  LOGI("child %d terminated\r\n",pid);

  return ;
}


#if 0

这里我详细说明一下为何这个是better
首先记住两点
1.*nix的信号不排队，即来一个处理一个，处理的时候又来了的话忽略
2.wait调用是阻塞的
我们来说说上个信号处理为什么不行
比方说，此刻有10个子进程同时死掉了（多个客户连接同时断开就是这个情况）
那么这10个子进程都会发送 SIGCHLD ,紧接着 父进程处理收到的第一个
SIGCHLD，让其中一个子进程安心的,真正的“死去”，但是在这期间，剩下的9个 SIGCHLD
可能也来了,但父进程不知道，所以剩下的 9 个子进程会一直处于僵尸状态,因为未能进入信号处理函数

即使运气好一点，父进程处理的够快，可能还会接收到第9或者10个子进程的 SIGCHLD，
然后进入信号处理函数处理这个信号,但上面的处理，也只能处理一个僵尸子进程
这里必须明白的一点是（我猜测），父进程所wait到的并非一定是发送该 SIGCHLD
的子进程，而是任意一个已经“死掉”、等待父进程处理的进程

于是，僵尸进程就这样产生了，直到父进程退出


那么，我们怎么处理呢？
思路是，一旦接收到一个 SIGCHLD 我们就把所有的已经“死”的，和正在处理 SIGCHLD的时候“死”的的子进程全部wait了,
    这样，即使某些 SIGCHLD 没有被捕捉到，我们还是能把发送这个 SIGCHLD 的子进程(发送SIGCHLD意味着已经僵尸)给wait了,
    简单点说就是，一旦接收到 SIGCHLD
    我们就把所有的能处理的子进程都处理掉，直到没有僵尸的子进程,

    但是这个时候我们就不能用wait了，因为wait只能是阻塞的，为什么呢？

    举个例子就是说:
    假设进程p有4个子进程c1，c2，c3，c4
    在a时刻，c1,c2死了，p接收到了c1发过来的信号，进入 SIGCHLD 的处理函数,
    （假设现在我们是使用的waitpid的非阻塞模式处理），然后p处理了或正在处理c1,
    这个时候，(假设是时刻b，或者是在时刻b稍微前一段的时刻)，c2也发过来了一个信号 SIGCHLD 并变成了“僵尸状态”，
    此时，p很可能就错过了这个 SIGCHLD，
    但是不要紧，我们的处理函数会循环waitpid，所以会继续让c2安心的“死去”,即使没有收到c2的 SIGCHLD （甚至c2不发送这个信号都不要紧！！），
    处理完了c2之后，waitpid函数又被调用，假设这是时刻x，此时发现没有僵尸子进程了(c3,c4还在运行)，waitpid返回，信号处理函数就退出了

    所以，如果时刻x调用的是wait，那么进程就阻塞在这个信号处理函数，直到下一个子进程“僵尸”，如此循环，也就是说，信号处理函数永远退不出来了


#endif

void server1_handle_SIGCHLD_better(int signo)
{
  pid_t  pid;
  int stat;

  while((pid=waitpid(-1,&stat,WNOHANG))>0)
  {
    //here may return -1 , i don't know why now 2014.11.21
    LOGI("child %d terminated\r\n",pid);
  }

  return ;
}


int sendToServer_02(FILE* pf,int fd)
{

  // i have tested this, but not work well
  // setvbuf(pf,0,_IONBF,0);

  int maxfd=0;

  int sourceFd=fileno(pf);
  fd_set rset;
  FD_ZERO(&rset);

  char *buffer=new char[BUFFER_SIZE];

  char *p=NULL;
  ssize_t writeSize;
  ssize_t readSize;

  while(true)
  {
    FD_SET(sourceFd,&rset);
    FD_SET(fd,&rset);
    maxfd=max(sourceFd, fd)+1;
    select(maxfd,&rset,NULL,NULL,NULL);

    if(FD_ISSET(sourceFd,&rset))
    {
      // TODO: when fget get EOF(that is stdin's ctrl+d and file's end), 
      // circle break and client exit immediately, but some data send by server may
      // still in network(if pf is file stream will mostly happened)

      // TODO: here has another bug, fget actully call read inside, *unix will
      // use a buffer for stdio to reduce the counts we call read
      // so here fget may(exactly, must be) read from file for more than one line but return one line everytime we call it
      // but question is, the select only test if some fd whether can
      // read/write/error( which is os function call by fxxx etc.)
      // so, here client may only fgets the first one line, and never reach
      // fgets again, what make left lines still in fgets' buf
      // so, !!!!! an important point is, never use select and stdio in the same time! !!!!!

      // sendToServer_03(FILE* pf,int fd) will fix up all

      p=fgets(buffer,BUFFER_SIZE,pf);
      if(!p)
        break;

      writeSize = write(fd,buffer,strlen(buffer) + 1);
      assert(writeSize>=0);
    }
    if(FD_ISSET(fd,&rset))
    {
      readSize = read(fd,buffer,BUFFER_SIZE);
      LOGD("readSize is %d\r\n",readSize);
      if(readSize >0)
      {
        fputs(buffer,stdout);
      }
      else if(readSize ==0)
      {
        LOGE("read end of file  \r\n");
        break;
      }
      else if(errno == EINTR)
      {
        LOGD("EINTR happend\r\n");
      }
      else
      {
        LOGE("read nothing error is %s \r\n",strerror(errno));
        break;
      }
    }
  }
  delete [] buffer;
  return 0;
}


int sendToServer_03(FILE* pf,int fd)
{

  int maxfd=0;

  int sourceFd=fileno(pf);
  fd_set rset;
  FD_ZERO(&rset);

  char *buffer=new char[BUFFER_SIZE];

  ssize_t writeSize;
  ssize_t readSize;

  bool is_stdin_eof=false;

  while(true)
  {
    if(!is_stdin_eof)
      FD_SET(sourceFd,&rset);

    FD_SET(fd,&rset);
    maxfd=max(sourceFd, fd)+1;
    select(maxfd,&rset,NULL,NULL,NULL);

    if(FD_ISSET(sourceFd,&rset))
    {
      readSize=read(sourceFd,buffer,BUFFER_SIZE);
      if(readSize == 0)
      {
        is_stdin_eof = true;
        shutdown(fd,SHUT_WR);
        //here will send a FIN, so just continue to select sock fd, no need to write
        FD_CLR(sourceFd,&rset);
        continue;
      }
      LOGD("readSize is %d\r\n",readSize);

      writeSize = write(fd,buffer,readSize);
      sleep(1000);
      assert(writeSize>=0);
    }
    if(FD_ISSET(fd,&rset))
    {
      readSize = read(fd,buffer,BUFFER_SIZE);
      if(readSize >0)
      {
        write(fileno(stdout),buffer,readSize);
      }
      else if(readSize ==0)
      {
        if(is_stdin_eof)
        {
          LOGI("mostlt get all we send\r\n");
        }
        else
        {
          LOGI("server process dead and send a FIN to us!\r\n");
        }
        break;
      }
      else if(errno == EINTR)
      {
        LOGD("EINTR happend\r\n");
        break;
      }
      else
      {
        LOGE("read nothing error is %s \r\n",strerror(errno));
        break;
      }
    }
  }
  delete [] buffer;
  return 0;
}



