
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/fcntl.h>

#define LOG_DETAILE
#define LOG_ERROR
#define LOG_INFO

#include "log.h"
#include "utils.h"
#include "handle.h"



int main(int argc,char* argv[])
{

  assert(argc==2);

  int ret=0;

  //here just support IPV4
  sockaddr_in serverAddr; 

  memset(&serverAddr,0,sizeof(serverAddr));

  serverAddr.sin_family=AF_INET;

  ret = inet_pton(AF_INET,argv[1],&serverAddr.sin_addr);
  assert(ret == 1);

  serverAddr.sin_port=htons(SERVER1_PORT);

  int fds[10];

  for(int i=0;i<10;i++)
  {
    fds[i] = socket(AF_INET,SOCK_STREAM,0);

    ret = connect(fds[i],(SA*)&serverAddr,sizeof(serverAddr));
    if(ret !=0)
    {
      LOGE("%s\r\n",strerror(errno));
    }
    LOGI("connect success!\r\n");
  }


  FILE* pfClient=fopen("datafile","rb");
  assert(pfClient!=NULL);

  //sendToServer_01(stdin,fds[0]);
  //sendToServer_01(pfClient,fds[0]);


  //sendToServer_02(stdin,fds[0]);
  //sendToServer_02(pfClient,fds[0]);

  sendToServer_03(stdin,fds[0]);
  //sendToServer_03(pfClient,fds[0]);

  fclose(pfClient);


  return 0;
}
