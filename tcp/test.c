
#include <unistd.h>

#include "utils.h"


int main(int argc,char*argv[])
{
  daemonize(argv[0],0);
  sleep(5);
  return 0;
}
