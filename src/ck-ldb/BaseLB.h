/*****************************************************************************
 * $Source$
 * $Author$
 * $Date$
 * $Revision$
 *****************************************************************************/

/**
 \defgroup CkLdb  Charm++ Load Balancing Framework 
*/
/*@{*/

#ifndef BASELB_H
#define BASELB_H

#include "LBDatabase.h"

/// Base class for all LB strategies.
/**
  BaseLB is the base class for all LB strategy class.
  it does some tracking about how many lb strategies are created.
  it also defines some common functions.
*/
class BaseLB: public CBase_BaseLB
{
protected:
  int  seqno;
  char *lbname;
  LBDatabase *theLbdb;
  LDBarrierReceiver receiver;
  int  notifier;
  int  startLbFnHdl;
private:
  void initLB(const CkLBOptions &);
public:
  BaseLB(const CkLBOptions &opt)  { initLB(opt); }
  BaseLB(CkMigrateMessage *m):CBase_BaseLB(m) {}
  virtual ~BaseLB();

  void unregister(); 
  inline char *lbName() { return lbname; }
  virtual void turnOff() { CmiAbort("turnOff not implemented"); }
  virtual void turnOn()  { CmiAbort("turnOn not implemented"); }
  void pup(PUP::er &p);
  virtual void flushStates();
};

/// migration decision for an obj.
struct MigrateInfo {  
    int index;   // object index in objData array
    LDObjHandle obj;
    int from_pe;
    int to_pe;
    int async_arrival;	    // if an object is available for immediate migrate
    MigrateInfo():  async_arrival(0) {}
};

/**
  message contains the migration decision from LB strategies.
*/
class LBMigrateMsg : public CMessage_LBMigrateMsg {
public:
  int n_moves;
  MigrateInfo* moves;

  char * avail_vector;
  int next_lb;

  double * expectedLoad;
};

// for a FooLB, the following macro defines these functions for each LB:
// CreateFooLB(): which register with LBDatabase with sequence ticket
// , 
// AllocateFooLB(): which only locally allocate the class
// static void lbinit(): which is an init call
#if CMK_LBDB_ON
#define CreateLBFunc_Def(x, str)		\
void Create##x(void) { 	\
  int seqno = LBDatabaseObj()->getLoadbalancerTicket();	\
  CProxy_##x::ckNew(CkLBOptions(seqno)); 	\
}	\
BaseLB *Allocate##x(void) { \
  return new x((CkMigrateMessage*)NULL);	\
}	\
static void lbinit(void) {	\
  LBRegisterBalancer(#x,	\
                     Create##x,	\
                     Allocate##x,	\
                     str);	\
}
#else		/* CMK_LBDB_ON */
#define CreateLBFunc_Def(x, str)	\
void Create##x(void) {} 	\
BaseLB *Allocate##x(void) { return NULL; }	\
static void lbinit(void) {}
#endif

#endif

/*@}*/
