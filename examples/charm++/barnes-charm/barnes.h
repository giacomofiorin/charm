#ifndef _BARNES_H_
#define _BARNES_H_

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "defs.h"
#include "stdinc.h"

using namespace std;

#include "barnes.decl.h"

#define MAX_PARTS_PER_TP 1000
#define MAX_PARTICLES_PER_MSG 500
#define INIT_PARTS_PER_CHILD 1000

#define G_MALLOC malloc


extern CProxy_Main mainChare;
extern CProxy_TreePiece pieces;
extern CProxy_ParticleChunk chunks;

extern int numTreePieces;
extern int numParticleChunks;
extern int maxPartsPerTp;
extern int nbody;
extern int NPROC;
extern int maxmybody;
extern real fcells;
extern real fleaves;
extern real tstop;
extern int nbody;
extern real dtime;
extern real eps;
extern real epssq;
extern real tol;
extern real tolsq;
extern real dtout;
extern real dthf;
/*
extern vector rmin;
extern real rsize;
*/

extern int maxmycell;
extern int maxmyleaf;

extern CkReduction::reducerType minmax_RealVectorType;


class ParticleMsg : public CMessage_ParticleMsg {
  public:
  bodyptr *particles;
  int num;
};

/*
const char *defv[] = {                 
    "in=",                        
    "out=",                       

    "nbody=16384",                
    "seed=123",                   

    "dtime=0.025",                
    "eps=0.05",                   
    "tol=1.0",                    
    "fcells=2.0",                 
    "fleaves=0.5",                

    "tstop=0.075",                
    "dtout=0.25",                 

    "NPROC=1"                    
};
*/

static string headline = "Hack code: Plummer model";

class Main : public CBase_Main {

  
  int maxleaf;
  int maxcell;

  string infile;
  string outfile;

  real tnow;
  real tout;
  real mymtot;
  int iterations;

  vector rmin;
  real rsize;
  
  // these are used instead of proc. 0's data structures
  bodyptr *mybodytab;
  bodyptr bodytab;


  CkVec<string> defaults; 

  // j: don't need these data structures
  //cellptr *mycelltab;
  //leafptr *myleaftab;

  //void initparam (string *argv, const char **defv);
  void initoutput();

  string extrvalue(string &arg);
  //bool matchname(string &bind, string &name);
  //int scanbind(CkVec<string> &bvec, string &name);
  double getdparam(string name);
  bool getbparam(string name);
  long getlparam(string name);
  int getiparam(string name);
  string getparam(string name);

  void init_root(unsigned int ProcessId);
  int createTopLevelTree(cellptr node, int depth);
  cellptr G_root; 
  CkVec<nodeptr> topLevelRoots;

  void updateTopLevelMoments();
  nodeptr moments(nodeptr node, int depth);

#ifdef PRINT_TREE
  void graph();
#endif

  void inputdata();
  
  public:
  Main(CkArgMsg *m);
  Main(CkMigrateMessage *m){}
  void startSimulation();
  void pickshell(real vec[], real rad);
  void testdata();
  void startrun();
  void Help();
  void tab_init();
  void setbound();

  void recvGlobalSizes(CkReductionMsg *msg);

};

class ParticleChunk : public CBase_ParticleChunk {

  int mynbody;
  // pointer to my pointers to particles
  bodyptr *mybodytab;
  // pointer to array of particles (required for find_my_initial_bodies)
  bodyptr bodytab;

  cellptr G_root; // obtained from 0th member of array,
                  // after it has finished creating the
                  // top level tree
  nodeptr Current_Root;
  int Root_Coords[NDIM];

  real tnow;
  real tstop;
  int nstep;

  CkCallback mainCb;

  // how many messages containing particles have been sent to 
  // each of the top-level treepieces
  int *numMsgsToEachTp;
  // which of my particles are to be sent to which top-level 
  // treepieces
  CkVec<CkVec<bodyptr> >particlesToTps;
  void HouseKeep();
  void find_my_bodies(nodeptr mycell, int work, int direction, unsigned int ProcessId);

   int myn2bcalc; 	/* body-body force calculations for each processor */
   int mynbccalc; 	/* body-cell force calculations for each processor */
   int myselfint; 	/* count self-interactions for each processor */
   int myn2bterm; 	/* count body-body terms for a body */
   int mynbcterm; 	/* count body-cell terms for a body */
   bool skipself; 	/* true if self-interaction skipped OK */
   bodyptr pskip;       /* body to skip in force evaluation */
   vector pos0;         /* point at which to evaluate field */
   real phi0;           /* computed potential at pos0 */
   vector acc0;         /* computed acceleration at pos0 */
   vector dr;  		/* data to be shared */
   real drsq;      	/* between gravsub and subdivp */
   nodeptr pmem;	/* remember particle data */

  real workMin;
  real workMax;

  real rsize;
  vector rmin;

  
  void hackgrav(bodyptr p, unsigned ProcessId);
  void gravsub(nodeptr p, unsigned ProcessId);
  void hackwalk(unsigned ProcessId);
  void walksub(nodeptr n, real dsq, unsigned ProcessId);
  bool subdivp(nodeptr p, real dsq, unsigned ProcessId);

  public:
  ParticleChunk(int maxleaf, int maxcell);
  ParticleChunk(CkArgMsg *m);
  ParticleChunk(CkMigrateMessage *m){}
  void find_my_initial_bodies(bodyptr btab, int nb, unsigned ProcessId);

  void flushParticles();
  void sendParticlesToTp(int tp);

  //void SlaveStart(bodyptr *bb, bodyptr b, CkCallback &cb);
  void SlaveStart(CmiUInt8 bb, CmiUInt8 b, CkCallback &cb);
  //void SlaveStart(CmiUInt8 bb, CmiUInt8 b, CmiUInt8 mct, CmiUInt8 mlt, CkCallback &cb);
  //void startIteration(CkCallback &cb);
  void acceptRoot(CmiUInt8 root, real rx, real ry, real rz, real rs, CkCallback &cb);
  void stepsystemPartII(CkReductionMsg *msg);

  void maketree(unsigned int ProcessId);
  nodeptr loadtree(bodyptr p, cellptr root, unsigned ProcessId);
  void doneSendingParticles();

  void doneTreeBuild();

  void partition(CkCallback &cb);
  void ComputeForces(CkCallback &cb);
  void advance(CkCallback &cb);
  void cleanup();
};

class TreePiece : public CBase_TreePiece {

  int mynumcell;
  int mynumleaf;

  // for debugging purposes
  int done;
 
  bool isTopLevel;
  int myLevel;
  int numTotalMsgs;
  int numRecvdMsgs;
  bool haveChildren;
  bool haveCounts;
  int sentTo[NSUB]; // one counter for each child
  CkVec<bodyptr> myParticles;
  int myNumParticles;
  CkArrayIndex1D parentIndex;
  int pendingChildren;
  CkVec<CkVec<bodyptr> >partsToChild;

  nodeptr myRoot;
  nodeptr parent;
  int whichChildAmI;
  int childrenTreePieces[NSUB];

  CkVec<cellptr> mycelltab;
  CkVec<leafptr> myleaftab;
  int mynleaf;
  int myncell;

  CkVec<ParticleMsg *> bufferedMsgs;
  bool haveParent;

  void checkCompletion();
  leafptr InitLeaf(cellptr parent, unsigned ProcessId);
  cellptr InitCell(cellptr parent, unsigned ProcessId);
  void hackcofm(int nc, unsigned ProcessId);
  void updateMoments(int which);

  real rsize;
  vector rmin;

  public:
  TreePiece(CmiUInt8 parent, int whichChild, int level, CkArrayIndex1D parent);
  TreePiece(CmiUInt8 p, int which, int level, real rx, real ry, real rz, real rs, CkArrayIndex1D parent); 
  TreePiece(CkMigrateMessage *m){}
  void recvRootFromParent(CmiUInt8 r, real rx, real ry, real rz, real rs);
  // used to convey message counts from chunks to top-level
  // treepieces
  void acceptRoots(CmiUInt8 root, real rsize, real rminx, real rminy, real rminz, CkCallback &cb);
  void recvTotalMsgCountsFromChunks(CkReductionMsg *msg);
  // used to convey message counts from treepieces to their
  // children
  void recvTotalMsgCountsFromPieces(int num);
  void buildTree();
  void recvParticles(ParticleMsg *msg);
  nodeptr loadtree(bodyptr p, cellptr root, unsigned int ProcessId);
  cellptr SubdivideLeaf (leafptr le, cellptr parent, unsigned int l, unsigned int ProcessId);
  void childDone(int which);
  void cleanup(CkCallback &cb);

};

int subindex(int *xp, int level);
bool intcoord(int *xp, vector p, vector rmin, real rsize);
cellptr makecell(unsigned ProcessId);
leafptr makeleaf(unsigned ProcessId);
int log8floor(int arg);

real xrand(real lo, real hi);
void pranset(int seed);
double prand();

#endif
