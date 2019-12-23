#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"
int
main(int argc, char *argv[])
{
  struct pstat p;
  printf(1, "Process Statistics\n");
  getpinfo(&p);
  printf(1, "inuse tickets pid ticks\n");
  for(int i=0; i<NPROC; i++){
    if (p.pid[i] != 0){
      printf(1,"%d %d %d %d\n",
              p.inuse[i], p.tickets[i], p.pid[i], p.ticks[i]);
    }
  }

  printf(1, "Graph to visualise time ticks received over time\n");
  printf(1, "|Time |P1 |P2 |P3 |\n");
  for(int i=0;i<5;i++){
    getpinfo(&p);
    printf(1,"%d %d %d %d\n", i, p.ticks[0], p.ticks[1], p.ticks[2]);    
    sleep(100);
  }
  exit();
}
