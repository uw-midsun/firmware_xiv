#pragma once
#include "can_msg.h"
#include "pqueue_backed.h"

#define CAN_QUEUE_SIZE 10

typedef struct CanQueue {
  PQueueBacked pqueue;
  PQueueNode queue_nodes[CAN_QUEUE_SIZE + 1];
  CanMessage msg_nodes[CAN_QUEUE_SIZE];
} CanQueue;

StatusCode can_queue_init(CanQueue *can_queue);

StatusCode can_queue_push(CanQueue *can_queue, const CanMessage *msg);

StatusCode can_queue_pop(CanQueue *can_queue, CanMessage *msg);

StatusCode can_queue_peek(CanQueue *can_queue, CanMessage *msg);

size_t can_queue_size(CanQueue *can_queue);
