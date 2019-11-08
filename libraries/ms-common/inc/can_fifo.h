#pragma once
// Specific instance of FIFO for CAN
#include "can_msg.h"
#include "fifo.h"

#define CAN_FIFO_SIZE 32

typedef struct CanFifo {
  Fifo fifo;
  CanMessage msg_nodes[CAN_FIFO_SIZE];
} CanFifo;

#define can_fifo_init(can_fifo) fifo_init(&(can_fifo)->fifo, (can_fifo)->msg_nodes)

#define can_fifo_push(can_fifo, source) fifo_push(&(can_fifo)->fifo, (source))

#define can_fifo_peek(can_fifo, dest) fifo_peek(&(can_fifo)->fifo, (dest))

#define can_fifo_pop(can_fifo, dest) fifo_pop(&(can_fifo)->fifo, (dest))

#define can_fifo_size(can_fifo) fifo_size(&(can_fifo)->fifo)
