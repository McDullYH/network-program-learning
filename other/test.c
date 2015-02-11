
#include <netdb.h>
#include <arpa/inet.h>
#include <assert.h>
#include <string.h>




#define LOG_DETAILE
#define LOG_INFO
#define LOG_ERROR
#include "log.h"


//usage: ./test www.sina.com
int test_gethostbyname(const char* hostname);

//usage: ./test 180.97.33.107
int test_gethostbyaddr(const char* addr);


//usage: ./test 180.97.33.107
int test_getaddrinfo(const char* hostname,const char* service);

int main(int argc, char* argv[])
{


  //test_gethostbyname(argv[1]);

  //test_gethostbyaddr(argv[1]);

  test_getaddrinfo(argv[1],argv[2]);

  return 0;
}

int test_gethostbyname(const char* hostname)
{

  hostent *phost;

  if((phost=gethostbyname(hostname))==NULL)
  {
    LOGE("gethostbyname error %s\r\n",hstrerror(h_errno));
    return 1;
  }

  LOGI("official hostname is %s\r\n",phost->h_name);

  char ** pptr;

  for(pptr=phost->h_aliases;*pptr!=NULL;pptr++)
  {
    LOGI("alias name is %s\r\n",*pptr);
  }

  char str[INET_ADDRSTRLEN];
  if(phost->h_addrtype == AF_INET)
  {
    for(pptr=phost->h_addr_list;*pptr!=NULL;pptr++)
    {
      LOGI("address is %s\r\n",inet_ntop(AF_INET,*pptr,str,sizeof(str)));
    }
  }

  return 0;
}

int test_gethostbyaddr(const char* addr)
{
  sockaddr_in testAddr;
  testAddr.sin_family = AF_INET;
  int ret=inet_pton(AF_INET,addr,&testAddr.sin_addr);
  testAddr.sin_port=htons(80);
  assert(ret==1);


  hostent *phost;

  if((phost=gethostbyaddr(&testAddr,sizeof(testAddr),AF_INET))==NULL)
  {
    LOGE("gethostbyname error %s\r\n",hstrerror(h_errno));
    return 1;
  }

  LOGI("official hostname is %s\r\n",phost->h_name);

  char ** pptr;

  for(pptr=phost->h_aliases;*pptr!=NULL;pptr++)
  {
    LOGI("alias name is %s\r\n",*pptr);
  }

  char str[INET_ADDRSTRLEN];
  if(phost->h_addrtype == AF_INET)
  {
    for(pptr=phost->h_addr_list;*pptr!=NULL;pptr++)
    {
      LOGI("address is %s\r\n",inet_ntop(AF_INET,*pptr,str,sizeof(str)));
    }
  }

  return 0;
}


int test_getaddrinfo(const char* hostname, const char *service)
{

  addrinfo hints, *result , *iter;

  sockaddr_in *paddr;




  memset(&hints,0,sizeof(hints));

  //to aquire official name
  hints.ai_flags=AI_CANONNAME;
  
  //just aquire IPV4 info
  hints.ai_family = AF_INET;

  //just aquire TCP info
  hints.ai_socktype = SOCK_STREAM;

  int ret = getaddrinfo(hostname,service,&hints,&result);
  if(ret !=0)
  {
    LOGE("getaddrinfo error %s\r\n",gai_strerror(ret));
    return -1;
  }

  char str[INET_ADDRSTRLEN];

  for (iter = result;iter!=NULL;iter=iter->ai_next)
  {
    LOGI("address cannon name is %s\r\n",iter->ai_canonname);
    paddr=(sockaddr_in*)iter->ai_addr;
    LOGI("ip address is %s\r\n",inet_ntop(AF_INET,&paddr->sin_addr,str,INET_ADDRSTRLEN));
    LOGI("port is %d\r\n",ntohs(paddr->sin_port));
  }

  freeaddrinfo(result);


  return 0;
}
