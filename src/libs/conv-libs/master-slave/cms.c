/*****************************************************************************
 * $Source$
 * $Author$
 * $Date$
 * $Revision$
 *****************************************************************************/


/* converse master-slave (or manager-worker or agenda) paradigm library */


#include <stdlib.h>
#include <converse.h>

typedef  int (* cms_WorkerFn)(void *, void *);

typedef int (* cms_consumer)(void *, int);

typedef struct {
  int ref;
  char * response;
} ResponseRecord;

CpvDeclare(int, stopHandlerIndex);
CpvDeclare(int, workHandler);
CpvDeclare(int,responseHandler);
CpvDeclare(int,infoIdx);
CpvDeclare (ResponseRecord *, responses);
CpvDeclare(cms_WorkerFn, clientWorker);
CpvDeclare(cms_consumer, consumerFunction);
CpvDeclare(int,counter);
CpvDeclare(int, tableSize);

void initForCms(int argc,char *argv[])
{
}

void cms_infoFn(void *msg, CldPackFn *pfn, int *len,
            int *queueing, int *priobits, unsigned int **prioptr)
{
  *pfn = 0;
  *len = *( (int*) ((char *) msg +CmiExtHeaderSizeBytes)) + CmiExtHeaderSizeBytes + 8;
  *queueing = CQS_QUEUEING_FIFO;
  *priobits = 0;
  *prioptr = 0;
}

void stopHandler(char * msg)
{
  CsdExitScheduler();
  ConverseExit();
}

void callWorker(char *msg)
{
 char * m, *r;
 int size, msgSize, ref;
char * msg2;

 ref = *((int *) (msg + sizeof(int) + CmiExtHeaderSizeBytes));
 m = msg + 2*sizeof(int) + CmiExtHeaderSizeBytes;
 size = CpvAccess(clientWorker)(m, &r);

  msgSize = 2*sizeof(int) + CmiMsgHeaderSizeBytes + size;
  msg2 = CmiAlloc( msgSize );
  m = msg2 +  CmiMsgHeaderSizeBytes;
  *((int *) m) = size;
  m += sizeof(int);
  *((int *) m) = ref;
  m += sizeof(int);
  memcpy( m, r, size);
  CmiSetHandler(msg2, CpvAccess(responseHandler));
  CmiSyncSendAndFree( 0, msgSize, msg2);
};

void response(char * msg)
{
  char * r, *m;
  int size, ref;
   m = msg + CmiMsgHeaderSizeBytes;
  size = *( (int *) m);
  m += sizeof(int);
  ref = *( (int *) m);
  m += sizeof(int);
  r = (char *) malloc(size);
  memcpy(r, m, size);
  if (CpvAccess(consumerFunction) == NULL) {
    CpvAccess(responses)[ref].ref = ref;
    CpvAccess(responses)[ref].response = r;}
  else {
    CpvAccess(consumerFunction)(r, ref);
  }
    
  if (--CpvAccess(counter) == 0) {
   CsdExitScheduler();
  }
}

 
char *CmsGetResponse(int ref)
{
  return (CpvAccess(responses)[ref].response);
}


void cms_registerHandlers(cms_WorkerFn f)
{
  CpvInitialize(int,stopHandlerIndex);
  CpvInitialize(int,workHandler);
  CpvInitialize(int,responseHandler);
  CpvInitialize(int,infoIdx);
  CpvInitialize (ResponseRecord *, responses);
  CpvInitialize(cms_WorkerFn, clientWorker);
  CpvInitialize(cms_consumer, consumerFunction);
  CpvInitialize(int,counter);
  CpvInitialize(int,tableSize);


  CpvAccess(stopHandlerIndex) = CmiRegisterHandler((CmiHandler) stopHandler);
  CpvAccess(workHandler) = CmiRegisterHandler((CmiHandler) callWorker);
  CpvAccess(responseHandler) = CmiRegisterHandler((CmiHandler) response);
  CpvAccess(infoIdx) = CldRegisterInfoFn(cms_infoFn);
  CpvAccess(clientWorker) = f;
  CpvAccess(consumerFunction) = (cms_consumer) NULL;
}



void CmsInit(cms_WorkerFn f, int maxResponses)
{
  char* argv[100];
  int argc = 0;
  argv[0] = 0;
  ConverseInit(argc, argv,(CmiStartFn)initForCms,1,1);
  cms_registerHandlers(f);
  CpvAccess(tableSize) = maxResponses;
  if (CmiMyPe() == 0) { /* I am the manager */
    CpvAccess(responses) = malloc(maxResponses*sizeof(ResponseRecord));
    CpvAccess(counter) = 0;
  }
  else { /* I am a worker */
  CsdScheduler(-1);
  ConverseExit();
  }    
}

void CmsFireTask(int ref, char * t, int size)
{
  char *m;
  char * msg;

  msg = CmiAlloc( 2*sizeof(int) + CmiExtHeaderSizeBytes + size);

  CmiSetHandler(msg, CpvAccess(workHandler));
  m = msg +  CmiExtHeaderSizeBytes;
  *((int *) m) = size;
  m += sizeof(int);
  *((int *) m) = ref;
  m += sizeof(int);
  memcpy( m, t, size);

  CldEnqueue(CLD_ANYWHERE, msg, CpvAccess(infoIdx)); 

  CpvAccess(counter)++;
}

void CmsAwaitResponses(void)
/* allows the system to use processor 0 as a worker.*/
{

  CsdScheduler(-1); /* be a worker for a while, and also process responses */
  /* when back from the sceduler, return. Because all response have been recvd */
} 

void CmsProcessResponses(cms_consumer f)
{
  CpvAccess(consumerFunction) = f;
  CsdScheduler(-1); 
}

void CmsExit(void)
{
  char * msg;

  msg = (char *) CmiAlloc(CmiMsgHeaderSizeBytes + 8);
  CmiSetHandler(msg, CpvAccess(stopHandlerIndex));
  CmiSyncBroadcastAndFree( CmiMsgHeaderSizeBytes + 8, msg);

  ConverseExit();
}
