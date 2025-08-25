//
// _RS_RISCVBARELIB_IMPL_C_
//
// Copyright (C) 2022-2024 Texas Tech University
// All Rights Reserved
// michael.beebe@ttu.edu
//
// See LICENSE in the top level directory for licensing details
//

#include <sys/types.h>
#include "common/threads.h"
#include "common/ticket_mutex.h"

#define RS_MAX_THREADS 32

typedef struct {
  double scalar;
  double *a;
  double *b;
  double *c;
  ssize_t *idx1;
  ssize_t *idx2;
  ssize_t *idx3;
  ssize_t beg;
  ssize_t end;
} KernelArgs;

thread_t THREADS[RS_MAX_THREADS];
KernelArgs KERNEL_ARGS[RS_MAX_THREADS];
ticket_mutex_t thread_mutex;

static int seqCopyThreadFunc(void *args)
{
  KernelArgs* __args = (KernelArgs*)args;
  ssize_t beg = __args->beg;
  ssize_t end = __args->end;

  for (ssize_t j = beg; j < end; j++)
    __args->c[j] = __args->a[j];

  return 0;
}

static int seqScaleThreadFunc(void *args)
{
  KernelArgs* __args = (KernelArgs*)args;
  ssize_t beg = __args->beg;
  ssize_t end = __args->end;

  for (ssize_t j = beg; j < end; j++)
    __args->b[j] = __args->scalar * __args->c[j];

  return 0;
}

static int seqAddThreadFunc(void *args)
{
  KernelArgs* __args = (KernelArgs*)args;
  ssize_t beg = __args->beg;
  ssize_t end = __args->end;

  for (ssize_t j = beg; j < end; j++)
    __args->c[j] = __args->a[j] + __args->b[j];

  return 0;
}

static int seqTriadThreadFunc(void *args)
{
  KernelArgs* __args = (KernelArgs*)args;
  ssize_t beg = __args->beg;
  ssize_t end = __args->end;

  for (ssize_t j = beg; j < end; j++)
    __args->a[j] = __args->b[j] + __args->scalar * __args->c[j];

  return 0;
}

static int gatherCopyThreadFunc(void *args)
{
  KernelArgs* __args = (KernelArgs*)args;
  ssize_t beg = __args->beg;
  ssize_t end = __args->end;

  for (ssize_t j = beg; j < end; j++)
    __args->c[j] = __args->a[__args->idx1[j]];

  return 0;
}

static int gatherScaleThreadFunc(void *args)
{
  KernelArgs* __args = (KernelArgs*)args;
  ssize_t beg = __args->beg;
  ssize_t end = __args->end;

  for (ssize_t j = beg; j < end; j++)
    __args->b[j] = __args->scalar * __args->c[__args->idx1[j]];

  return 0;
}

static int gatherAddThreadFunc(void *args)
{
  KernelArgs* __args = (KernelArgs*)args;
  ssize_t beg = __args->beg;
  ssize_t end = __args->end;

  for (ssize_t j = beg; j < end; j++)
    __args->c[j] = __args->a[__args->idx1[j]] + __args->b[__args->idx2[j]];

  return 0;
}

static int gatherTriadThreadFunc(void *args)
{
  KernelArgs* __args = (KernelArgs*)args;
  ssize_t beg = __args->beg;
  ssize_t end = __args->end;

  for (ssize_t j = beg; j < end; j++)
    __args->a[j] = __args->b[__args->idx1[j]] +
      __args->scalar * __args->c[__args->idx2[j]];

  return 0;
}

static int scatterCopyThreadFunc(void *args)
{
  KernelArgs *__args = (KernelArgs*)args;
  ssize_t beg = __args->beg;
  ssize_t end = __args->end;

  for (ssize_t j = beg; j < end; j++)
    __args->c[__args->idx1[j]] = __args->a[j];

  return 0;
}

static int scatterScaleThreadFunc(void *args)
{
  KernelArgs *__args = (KernelArgs*)args;
  ssize_t beg = __args->beg;
  ssize_t end = __args->end;

  for (ssize_t j = beg; j < end; j++)
    __args->b[__args->idx1[j]] = __args->scalar * __args->c[j];

  return 0;
}

static int scatterAddThreadFunc(void *args)
{
  KernelArgs *__args = (KernelArgs*)args;
  ssize_t beg = __args->beg;
  ssize_t end = __args->end;

  for (ssize_t j = beg; j < end; j++)
    __args->c[__args->idx1[j]] = __args->a[j] + __args->b[j];

  return 0;
}

static int scatterTriadThreadFunc(void *args)
{
  KernelArgs *__args = (KernelArgs*)args;
  ssize_t beg = __args->beg;
  ssize_t end = __args->end;

  for (ssize_t j = beg; j < end; j++)
    __args->a[__args->idx1[j]] = __args->b[j] + __args->scalar * __args->c[j];

  return 0;
}

static int sgCopyThreadFunc(void *args)
{
  KernelArgs *__args = (KernelArgs*)args;
  ssize_t beg = __args->beg;
  ssize_t end = __args->end;

  for (ssize_t j = beg; j < end; j++)
    __args->c[__args->idx1[j]] = __args->a[__args->idx2[j]];

  return 0;
}

static int sgScaleThreadFunc(void *args)
{
  KernelArgs *__args = (KernelArgs*)args;
  ssize_t beg = __args->beg;
  ssize_t end = __args->end;

  for (ssize_t j = beg; j < end; j++)
    __args->b[__args->idx2[j]] = __args->scalar * __args->c[__args->idx1[j]];

  return 0;
}

static int sgAddThreadFunc(void *args)
{
  KernelArgs *__args = (KernelArgs*)args;
  ssize_t beg = __args->beg;
  ssize_t end = __args->end;

  for (ssize_t j = beg; j < end; j++)
    __args->c[__args->idx1[j]] = __args->a[__args->idx2[j]] +
      __args->b[__args->idx3[j]];

  return 0;
}

static int sgTriadThreadFunc(void *args)
{
  KernelArgs *__args = (KernelArgs*)args;
  ssize_t beg = __args->beg;
  ssize_t end = __args->end;

  for (ssize_t j = beg; j < end; j++)
    __args->a[__args->idx2[j]] = __args->b[__args->idx3[j]] +
      __args->scalar * __args->c[__args->idx1[j]];

  return 0;
}

static int centralCopyThreadFunc(void *args)
{
  KernelArgs *__args = (KernelArgs*)args;
  ssize_t beg = __args->beg;
  ssize_t end = __args->end;

  for (ssize_t j = beg; j < end; j++)
    __args->c[0] = __args->a[0];

  return 0;
}

static int centralScaleThreadFunc(void *args)
{
  KernelArgs *__args = (KernelArgs*)args;
  ssize_t beg = __args->beg;
  ssize_t end = __args->end;

  for (ssize_t j = beg; j < end; j++)
    __args->b[0] = __args->scalar * __args->c[0];

  return 0;
}

static int centralAddThreadFunc(void *args)
{
  KernelArgs *__args = (KernelArgs*)args;
  ssize_t beg = __args->beg;
  ssize_t end = __args->end;

  for (ssize_t j = beg; j < end; j++)
    __args->c[0] = __args->a[0] + __args->b[0];

  return 0;
}

static int centralTriadThreadFunc(void *args)
{
  KernelArgs *__args = (KernelArgs*)args;
  ssize_t beg = __args->beg;
  ssize_t end = __args->end;

  for (ssize_t j = beg; j < end; j++)
    __args->a[0] = __args->b[0] + __args->scalar * __args->c[0];

  return 0;
}

//thread_t *threads = malloc((numThreads - 1)*sizeof(thread_t));
//KernelArgs *args = malloc(numThreads*sizeof(KernelArgs));
//free(threads);
//free(args);
#define FORK_JOIN_COMPUTE(ARGS, FUNC) do {                                    \
  int status;                                                                 \
  int numThreads = mp_get_cpu_count();                                        \
  int elemsThread = streamArraySize/numThreads;                               \
  thread_t *threads = THREADS;                                                \
  if (threads == NULL) {                                                      \
      printf("error: threads allocation failed\n");                           \
      exit(EXIT_FAILURE);                                                     \
  }                                                                           \
                                                                              \
  KernelArgs *args = KERNEL_ARGS;                                             \
  if (args == NULL) {                                                         \
      printf("error: args allocation failed\n");                              \
      exit(EXIT_FAILURE);                                                     \
  }                                                                           \
                                                                              \
  memset((void*)threads, 0, (numThreads - 1)*sizeof(thread_t));               \
  memset((void*)args, 0, numThreads*sizeof(KernelArgs));                      \
                                                                              \
  int i;                                                                      \
  ssize_t rems = streamArraySize;                                             \
  for (i = 0; i < numThreads-1; i++) {                                        \
    ARGS                                                                      \
    args[i].beg = i*elemsThread;                                              \
    args[i].end = (i + 1)*elemsThread;                                        \
    rems -= elemsThread;                                                      \
    status = thread_create(&threads[i], FUNC, (void*)&args[i]);               \
    if (status < 0) {                                                         \
      printf("error: thread_create(%d) returns %d\n", i, status);             \
      exit(EXIT_FAILURE);                                                     \
    }                                                                         \
  }                                                                           \
                                                                              \
  ARGS                                                                        \
  args[i].beg = i*elemsThread;                                                \
  args[i].end = streamArraySize;                                              \
  status = FUNC((void*)&args[i]);                                             \
                                                                              \
  for (i = 0; i < numThreads-1; i++) {                                        \
    status = thread_join(&threads[i]);                                        \
    if (status < 0) {                                                         \
      printf("error: thread_join(%d) returns %d\n", i, status);               \
      exit(EXIT_FAILURE);                                                     \
    }                                                                         \
  }                                                                           \
} while (0)

/**************************************************
 * @brief Copies data from one stream to another.
 *
 * @param streamArraySize Size of the stream array.
 **************************************************/
void seqCopy(
  double *a, double *b, double *c,
  ssize_t streamArraySize)
{
  FORK_JOIN_COMPUTE(
    args[i].a = a;
    args[i].c = c;,
    seqCopyThreadFunc);
}

/**************************************************
 * @brief Scales data in a stream.
 *
 * @param streamArraySize Size of the stream array.
 * @param scalar Scalar value for operations.
 **************************************************/
void seqScale(
  double *a, double *b, double *c,
  ssize_t streamArraySize, double scalar)
{
  FORK_JOIN_COMPUTE(
    args[i].scalar = scalar;
    args[i].b = b;
    args[i].c = c;,
    seqScaleThreadFunc);
}

/**************************************************
 * @brief Adds data from two streams.
 *
 * @param streamArraySize Size of the stream array.
 **************************************************/
void seqAdd(
  double *a, double *b, double *c,
  ssize_t streamArraySize)
{
  FORK_JOIN_COMPUTE(
    args[i].a = a;
    args[i].b = b;
    args[i].c = c;,
    seqAddThreadFunc);
}

/**************************************************
 * @brief Performs triad operation on stream data.
 *
 * @param streamArraySize Size of the stream array.
 * @param scalar Scalar value for operations.
 **************************************************/
void seqTriad(
  double *a, double *b, double *c,
  ssize_t streamArraySize, double scalar)
{
  FORK_JOIN_COMPUTE(
    args[i].scalar = scalar;
    args[i].a = a;
    args[i].b = b;
    args[i].c = c;,
    seqTriadThreadFunc);
}

/**************************************************
 * @brief Copies data using gather operation.
 *
 * @param streamArraySize Size of the stream array.
 **************************************************/
void gatherCopy(
  double *a, double *b, double *c,
  ssize_t *idx1, ssize_t streamArraySize)
{
  FORK_JOIN_COMPUTE(
    args[i].a = a;
    args[i].c = c;
    args[i].idx1 = idx1;,
    gatherCopyThreadFunc);
}

/**************************************************
 * @brief Scales data using gather operation.
 *
 * @param streamArraySize Size of the stream array.
 * @param scalar Scalar value for operations.
 **************************************************/
void gatherScale(
  double *a, double *b, double *c,
  ssize_t *idx1,
  ssize_t streamArraySize, double scalar)
{
  FORK_JOIN_COMPUTE(
    args[i].scalar = scalar;
    args[i].b = b;
    args[i].c = c;
    args[i].idx1 = idx1;,
    gatherScaleThreadFunc);
}

/**************************************************
 * @brief Adds data using gather operation.
 *
 * @param streamArraySize Size of the stream array.
 **************************************************/
void gatherAdd(
  double *a, double *b, double *c,
  ssize_t *idx1, ssize_t *idx2,
  ssize_t streamArraySize)
{
  FORK_JOIN_COMPUTE(
    args[i].a = a;
    args[i].b = b;
    args[i].c = c;
    args[i].idx1 = idx1;
    args[i].idx2 = idx2;,
    gatherAddThreadFunc);
}

/**************************************************
 * @brief Performs triad operation using gather.
 *
 * @param streamArraySize Size of the stream array.
 * @param scalar Scalar value for operations.
 **************************************************/
void gatherTriad(
  double *a, double *b, double *c,
  ssize_t *idx1, ssize_t *idx2,
  ssize_t streamArraySize, double scalar)
{
  FORK_JOIN_COMPUTE(
    args[i].scalar = scalar;
    args[i].a = a;
    args[i].b = b;
    args[i].c = c;
    args[i].idx1 = idx1;
    args[i].idx2 = idx2;,
    gatherTriadThreadFunc);
}

/**************************************************
 * @brief Copies data using scatter operation.
 *
 * @param streamArraySize Size of the stream array.
 **************************************************/
void scatterCopy(
  double *a, double *b, double *c,
  ssize_t *idx1,
  ssize_t streamArraySize)
{
  FORK_JOIN_COMPUTE(
    args[i].a = a;
    args[i].c = c;
    args[i].idx1 = idx1;,
    scatterCopyThreadFunc);
}

/**************************************************
 * @brief Scales data using scatter operation.
 *
 * @param streamArraySize Size of the stream array.
 * @param scalar Scalar value for operations.
 **************************************************/
void scatterScale(
  double *a, double *b, double *c,
  ssize_t *idx1,
  ssize_t streamArraySize, double scalar)
{
  FORK_JOIN_COMPUTE(
    args[i].scalar = scalar;
    args[i].b = b;
    args[i].c = c;
    args[i].idx1 = idx1;,
    scatterScaleThreadFunc);
}

/**************************************************
 * @brief Adds data using scatter operation.
 *
 * @param streamArraySize Size of the stream array.
 **************************************************/
void scatterAdd(
  double *a, double *b, double *c,
  ssize_t *idx1,
  ssize_t streamArraySize)
{
  FORK_JOIN_COMPUTE(
    args[i].a = a;
    args[i].b = b;
    args[i].c = c;
    args[i].idx1 = idx1;,
    scatterAddThreadFunc);
}

/**************************************************
 * @brief Performs triad operation using scatter.
 *
 * @param streamArraySize Size of the stream array.
 * @param scalar Scalar value for operations.
 **************************************************/
void scatterTriad(
  double *a, double *b, double *c,
  ssize_t *idx1,
  ssize_t streamArraySize, double scalar)
{
  FORK_JOIN_COMPUTE(
    args[i].scalar = scalar;
    args[i].a = a;
    args[i].b = b;
    args[i].c = c;
    args[i].idx1 = idx1;,
    scatterTriadThreadFunc);
}

/**************************************************
 * @brief Copies data using scatter-gather operation.
 *
 * @param streamArraySize Size of the stream array.
 **************************************************/
void sgCopy(
  double *a, double *b, double *c,
  ssize_t *idx1, ssize_t *idx2,
  ssize_t streamArraySize)
{
  FORK_JOIN_COMPUTE(
    args[i].a = a;
    args[i].c = c;
    args[i].idx1 = idx1;
    args[i].idx2 = idx2;,
    sgCopyThreadFunc);
}

/**************************************************
 * @brief Scales data using scatter-gather operation.
 *
 * @param streamArraySize Size of the stream array.
 * @param scalar Scalar value for operations.
 **************************************************/
void sgScale(
  double *a, double *b, double *c,
  ssize_t *idx1, ssize_t *idx2,
  ssize_t streamArraySize, double scalar)
{
  FORK_JOIN_COMPUTE(
    args[i].scalar = scalar;
    args[i].b = b;
    args[i].c = c;
    args[i].idx1 = idx1;
    args[i].idx2 = idx2;,
    sgScaleThreadFunc);
}

/**************************************************
 * @brief Adds data using scatter-gather operation.
 *
 * @param streamArraySize Size of the stream array.
 **************************************************/
void sgAdd(
  double *a, double *b, double *c,
  ssize_t *idx1, ssize_t *idx2, ssize_t *idx3,
  ssize_t streamArraySize)
{
  FORK_JOIN_COMPUTE(
    args[i].a = a;
    args[i].b = b;
    args[i].c = c;
    args[i].idx1 = idx1;
    args[i].idx2 = idx2;
    args[i].idx3 = idx3;,
    sgAddThreadFunc);
}

/**************************************************
 * @brief Performs triad operation using scatter-gather.
 *
 * @param streamArraySize Size of the stream array.
 * @param scalar Scalar value for operations.
 **************************************************/
void sgTriad(
  double *a, double *b, double *c,
  ssize_t *idx1, ssize_t *idx2, ssize_t *idx3,
  ssize_t streamArraySize, double scalar)
{
  FORK_JOIN_COMPUTE(
    args[i].scalar = scalar;
    args[i].a = a;
    args[i].b = b;
    args[i].c = c;
    args[i].idx1 = idx1;
    args[i].idx2 = idx2;
    args[i].idx3 = idx3;,
    sgTriadThreadFunc);
}

/**************************************************
 * @brief Copies data using a central location.
 *
 * @param streamArraySize Size of the stream array.
 **************************************************/
void centralCopy(
  double *a, double *b, double *c,
  ssize_t streamArraySize)
{
  FORK_JOIN_COMPUTE(
    args[i].a = a;
    args[i].c = c;,
    centralCopyThreadFunc);
}

/**************************************************
 * @brief Scales data using a central location.
 *
 * @param streamArraySize Size of the stream array.
 * @param scalar Scalar value for operations.
 **************************************************/
void centralScale(
  double *a,double *b, double *c,
  ssize_t streamArraySize, double scalar)
{
  FORK_JOIN_COMPUTE(
    args[i].scalar = scalar;
    args[i].b = b;
    args[i].c = c;,
    centralScaleThreadFunc);
}

/**************************************************
 * @brief Adds data using a central location.
 *
 * @param streamArraySize Size of the stream array.
 **************************************************/
void centralAdd(
  double *a, double *b, double *c,
  ssize_t streamArraySize)
{
  FORK_JOIN_COMPUTE(
    args[i].a = a;
    args[i].b = b;
    args[i].c = c;,
    centralAddThreadFunc);
}

/**************************************************
 * @brief Performs triad operation using a central location.
 *
 * @param streamArraySize Size of the stream array.
 * @param scalar Scalar value for operations.
 **************************************************/
void centralTriad(
  double *a, double *b, double *c,
  ssize_t streamArraySize, double scalar)
{
  FORK_JOIN_COMPUTE(
    args[i].scalar = scalar;
    args[i].a = a;
    args[i].b = b;
    args[i].c = c;,
    centralTriadThreadFunc);
}

/* EOF */

