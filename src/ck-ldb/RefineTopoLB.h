#ifndef _REFINETOPOLB_H_
#define _REINETOPOLB_H_

#include "CentralLB.h"
#include "TopoLB.h"
#include "topology.h"

#ifndef INFTY
#define INFTY 999999999
#endif

void CreateTopoLB ();

class RefineTopoLB : public TopoLB
{
  public:
    RefineTopoLB (const CkLBOptions &opt);
    RefineTopoLB (CkMigrateMessage *m) : TopoLB (m) { };
  
    void work (CentralLB::LDStats *stats, int count);
    void pup (PUP::er &p) { TopoLB::pup(p); }
    	
    //LBTopology			*topo;
  
  protected:
    double getCpartHopBytes(int cpart,int proc, int count);
    double findSwapGain(int cpart1, int cpart2,int count);
    double getInterMedHopBytes(CentralLB::LDStats *stats,int count, int *newmap);
    CmiBool QueryBalanceNow (int step);
}; 


#endif /* _TOPOLB_H_ */
