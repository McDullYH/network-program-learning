#ifndef _HANDLE_H_
#define _HANDLE_H_

#include <stdio.h>

int echoToClient(int fd);

int sendToServer_01(FILE* pf,int fd);

//this use select
int sendToServer_02(FILE* pf,int fd);

// this not use stdio for that we can use select correctly
// this use shutdown so that we won't close the fd to early
int sendToServer_03(FILE* pf,int fd);

void server1_handle_SIGCHLD_bad(int signo);
void server1_handle_SIGCHLD_better(int signo);


#endif
