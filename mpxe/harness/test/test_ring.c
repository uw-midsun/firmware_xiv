#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "ring.h"
#include "test_helpers.h"

// test general prod/con ability

#define NUM_THREADS 10
static unsigned int rand_seed = NUM_THREADS;

static Ring *s_ring;
static int s_output[NUM_THREADS];

static int prv_produce(int id) {
  int r = rand_r(&rand_seed);
  LOG_DEBUG("producer %d produced %d\n", id, r);
  return r;
}

static void prv_consume(int id, int num) {
  s_output[id] = num;
  LOG_DEBUG("consumer %d consumed %d\n", id, num);
}

static void *prv_producer(void *arg) {
  int *id = arg;
  for (int i = 0; i < NUM_THREADS; i++) {
    int num = prv_produce(*id);
    ring_wait_write(s_ring, &num);
  }
  free(arg);
  pthread_exit(NULL);
}

static void *prv_consumer(void *arg) {
  int *id = arg;
  for (int i = 0; i < NUM_THREADS; i++) {
    int num;
    ring_wait_read(s_ring, &num);
    prv_consume(*id, num);
  }
  free(id);
  pthread_exit(NULL);
}

void setup_test(void) {
  ring_create(sizeof(s_output[0]), NUM_THREADS, &s_ring);
  memset(&s_output[0], 0, sizeof(s_output));
}

void teardown_test(void) {
  ring_destroy(s_ring);
}

void test_prod_con(void) {
  pthread_t threads[NUM_THREADS * 2];

  for (int i = 0; i < NUM_THREADS; i++) {
    int *id = malloc(sizeof(int));
    *id = i;
    pthread_create(&threads[i], NULL, prv_producer, id);
  }
  for (int i = 0; i < NUM_THREADS; i++) {
    int *id = malloc(sizeof(int));
    *id = i;
    pthread_create(&threads[i], NULL, prv_consumer, id);
  }
  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  for (int i = 0; i < NUM_THREADS; i++) {
    TEST_ASSERT_NOT_EQUAL(0, s_output[i]);
  }

  LOG_DEBUG("output:\n");
  for (int i = 0; i < NUM_THREADS; i++) {
    printf("%i: %i\n", i, s_output[i]);
  }
  printf("\n");
}
