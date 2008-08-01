/* 
 * wr.h
 *
 * by Lukasz Wesolowski
 * 06.02.2008
 *
 * header containing declarations needed by the user of hybridAPI
 *
 */

#ifndef __WR_H__
#define __WR_H__

/* struct bufferInfo
 * 
 * purpose: 
 * structure to indicate which actions the runtime system should
 * perform in relation to the buffer
 *
 * usage: 
 *
 * value of device buffer in the runtime system table dictates
 * whether allocation  will take place;  NULL -> allocate; 
 * otherwise -> no allocation
 * 
 *  
 *
 */
typedef struct bufferInfo {

  /* ID of buffer in the runtime system's buffer table*/
  int bufferID; 

  /* flags to indicate if the buffer should be transferred */
  int transferToDevice; 
  int transferFromDevice; 
  
  /* flag to indicate if the device buffer memory should be freed
     after  execution of work request */
  int freeBuffer; 

  /* pointer to host data buffer */
  void *hostBuffer; 

  /* size of buffer in bytes */
  int size; 

} dataInfo; 



/* struct workRequest
 * 
 * purpose:  
 * structure for holding information about work requests on the GPU
 *
 * usage model: 
 * 1. declare a pointer to a workRequest 
 * 2. allocate dynamic memory for the work request
 * 3. call setupMemory to copy over the data to the GPU 
 * 4. enqueue the work request by using addWorkRequest
 */
typedef struct workRequest {

  /* parameters for kernel execution */

  dim3 dimGrid; 
  dim3 dimBlock; 
  int smemSize;
  
  /* buffer information for the execution of the work request */ 
  dataInfo *bufferInfo; 
  
  /* number of buffers used by the work request */ 
  int nBuffers; 


  /* pointers to queues and their lengths on the device(gpu) and
     host(cpu)  */
  /*
  void *readWriteDevicePtr;
  void *readWriteHostPtr; 
  int readWriteLen; 

  void *readOnlyDevicePtr; 
  void *readOnlyHostPtr; 
  int readOnlyLen; 

  void *writeOnlyDevicePtr;
  void *writeOnlyHostPtr; 
  int writeOnlyLen; 
  */

  /* to be called after the kernel finishes executing on the GPU */ 

  // void (*callbackFn)();
  void *callbackFn; 
 
  /* id to select the correct kernel in kernelSelect */

  int id; 

  /* event which will be polled to check if kernel has finished
     execution */

  cudaEvent_t completionEvent;  

  /* flags */

  int executing; 

} workRequest; 


/* struct workRequestQueue 
 *
 * purpose: container/mechanism for GPU work requests 
 *
 * usage model: 
 * 1. declare a workRequestQueue
 * 2. call init to allocate memory for the queue and initialize
 *    bookkeeping variables
 * 3. call enqueue for each request which needs to be 
 *    executed on the GPU
 * 4. in the hybrid API gpuProgressFn will execute periodically to
 *    handle the details of executing the work request on the GPU
 *             
 * implementation notes: 
 * the queue is implemented using a circular array; if the array fills
 * up, requests are transferred to a queue having additional
 * QUEUE_EXPANSION_SIZE slots, and the memory for the old queue is freed
 */
typedef struct {

  /* array of requests */
  workRequest* requests; 

  /* array index for the logically first item in the queue */
  int head; 

  /* array index for the last item in the queue */ 
  int tail; 

  /* number of work requests in the queue */
  int size; 

  /* size of the array of work requests */ 
  int capacity; 

} workRequestQueue; 

/* enqueue
 *
 * add a work request to the queue to be later executed on the GPU
 *
 */
void enqueue(workRequestQueue *q, workRequest *wr); 


#endif


