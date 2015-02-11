#ifndef _UTILS_H_
#define _UTILS_H_

#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>


#define SERVER1_PORT 9877

typedef struct sockaddr SA;
typedef struct sockaddr_storage sockaddr_storage;
typedef struct sockaddr_in sockaddr_in;

int max(int a,int b);



addrinfo* get_host_serv(const char* hostname,const char* service,int family,int socktype);
void free_host_serv(addrinfo* result);

int tcp_connect(const char* hostname, const char* service, socklen_t *addrlenp);

int tcp_listen(const char* hostname, const char* service, socklen_t *addrlenp);

int daemonize(const char* pname, int facility);





#endif //_UTILS_H_
