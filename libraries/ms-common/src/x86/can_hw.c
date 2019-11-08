#include "can_hw.h"

#include <fcntl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <poll.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "fifo.h"
#include "interrupt_def.h"
#include "log.h"
#include "x86_interrupt.h"

#define CAN_HW_DEV_INTERFACE "vcan0"
#define CAN_HW_MAX_FILTERS 14
#define CAN_HW_TX_FIFO_LEN 8
// Check for thread exit once every 10ms
#define CAN_HW_THREAD_EXIT_PERIOD_US 10000

typedef struct CanHwEventHandler {
  CanHwEventHandlerCb callback;
  void *context;
} CanHwEventHandler;

typedef struct CanHwSocketData {
  int can_fd;
  struct can_frame rx_frame;
  Fifo tx_fifo;
  struct can_frame tx_frames[CAN_HW_TX_FIFO_LEN];
  struct can_filter filters[CAN_HW_MAX_FILTERS];
  size_t num_filters;
  CanHwEventHandler handlers[NUM_CAN_HW_EVENTS];
  uint32_t delay_us;
} CanHwSocketData;

static pthread_t s_rx_pthread_id;
static pthread_t s_tx_pthread_id;
static pthread_barrier_t s_barrier;
// Producer/Consumer semaphore
static sem_t s_tx_sem;

// Locked if the TX/RX threads should be alive, unlocked on exit
static pthread_mutex_t s_keep_alive = PTHREAD_MUTEX_INITIALIZER;

static CanHwSocketData s_socket_data = { .can_fd = -1 };

static uint32_t prv_get_delay(CanHwBitrate bitrate) {
  const uint32_t delay_us[NUM_CAN_HW_BITRATES] = {
    1000,  // 125 kbps
    500,   // 250 kbps
    250,   // 500 kbps
    125,   // 1 mbps
  };

  return delay_us[bitrate];
}

static void *prv_rx_thread(void *arg) {
  x86_interrupt_pthread_init();
  LOG_DEBUG("CAN HW RX thread started\n");

  pthread_barrier_wait(&s_barrier);

  struct timeval timeout = { .tv_usec = CAN_HW_THREAD_EXIT_PERIOD_US };

  // Mutex is unlocked when the thread should exit
  while (pthread_mutex_trylock(&s_keep_alive) != 0) {
    // Select timeout is used to poll every now and then
    fd_set input_fds;
    FD_ZERO(&input_fds);
    FD_SET(s_socket_data.can_fd, &input_fds);

    select(s_socket_data.can_fd + 1, &input_fds, NULL, NULL, &timeout);

    if (FD_ISSET(s_socket_data.can_fd, &input_fds)) {
      int bytes =
          read(s_socket_data.can_fd, &s_socket_data.rx_frame, sizeof(s_socket_data.rx_frame));

      if (s_socket_data.handlers[CAN_HW_EVENT_MSG_RX].callback != NULL) {
        s_socket_data.handlers[CAN_HW_EVENT_MSG_RX].callback(
            s_socket_data.handlers[CAN_HW_EVENT_TX_READY].context);
      }

      // Limit how often we can receive messages to simulate bus speed
      usleep(s_socket_data.delay_us);
    }
  }

  pthread_mutex_unlock(&s_keep_alive);

  return NULL;
}

static void *prv_tx_thread(void *arg) {
  x86_interrupt_pthread_init();
  LOG_DEBUG("CAN HW TX thread started\n");
  struct can_frame frame = { 0 };

  pthread_barrier_wait(&s_barrier);

  // Mutex is unlocked when the thread should exit
  while (pthread_mutex_trylock(&s_keep_alive) != 0) {
    // Wait until the producer has created an item
    sem_wait(&s_tx_sem);
    fifo_pop(&s_socket_data.tx_fifo, &frame);
    int bytes = write(s_socket_data.can_fd, &frame, sizeof(frame));

    // Delay to simulate bus speed
    usleep(s_socket_data.delay_us);

    if (s_socket_data.handlers[CAN_HW_EVENT_TX_READY].callback != NULL) {
      s_socket_data.handlers[CAN_HW_EVENT_TX_READY].callback(
          s_socket_data.handlers[CAN_HW_EVENT_TX_READY].context);
    }
  }

  pthread_mutex_unlock(&s_keep_alive);

  return NULL;
}

StatusCode can_hw_init(const CanHwSettings *settings) {
  if (s_socket_data.can_fd != -1) {
    LOG_DEBUG("Exiting CAN HW\n");

    // Request threads to exit
    close(s_socket_data.can_fd);

    pthread_mutex_unlock(&s_keep_alive);

    pthread_join(s_rx_pthread_id, NULL);

    // Allow the TX thread to continue
    sem_post(&s_tx_sem);

    sem_destroy(&s_tx_sem);
    pthread_join(s_tx_pthread_id, NULL);
  }

  pthread_mutex_init(&s_keep_alive, NULL);
  // Init semaphore to thread-shared and locked
  sem_init(&s_tx_sem, 0, 0);

  pthread_mutex_lock(&s_keep_alive);

  memset(&s_socket_data, 0, sizeof(s_socket_data));
  s_socket_data.delay_us = prv_get_delay(settings->bitrate);
  fifo_init(&s_socket_data.tx_fifo, s_socket_data.tx_frames);

  s_socket_data.can_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if (s_socket_data.can_fd == -1) {
    LOG_CRITICAL("CAN HW: Failed to open SocketCAN socket\n");
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "CAN HW: Failed to open socket");
  }

  // Loopback - expects to receive its own messages
  int loopback = settings->loopback;
  if (setsockopt(s_socket_data.can_fd, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &loopback,
                 sizeof(loopback)) < 0) {
    LOG_CRITICAL("CAN HW: Failed to set loopback mode on socket\n");
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "CAN HW: Failed to set loopback mode on socket");
  }

  struct ifreq ifr = { 0 };
  snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", CAN_HW_DEV_INTERFACE);
  if (ioctl(s_socket_data.can_fd, SIOCGIFINDEX, &ifr) < 0) {
    LOG_CRITICAL("CAN HW: Device %s not found\n", CAN_HW_DEV_INTERFACE);
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "CAN HW: Device not found");
  }

  // Set non-blocking
  fcntl(s_socket_data.can_fd, F_SETFL, O_NONBLOCK);

  struct sockaddr_can addr = {
    .can_family = AF_CAN,
    .can_ifindex = ifr.ifr_ifindex,
  };
  if (bind(s_socket_data.can_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    LOG_CRITICAL("CAN HW: Failed to bind socket\n");
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "CAN HW: Failed to bind socket");
  }

  LOG_DEBUG("CAN HW initialized on %s\n", CAN_HW_DEV_INTERFACE);

  // 3 threads total: main, TX, RX
  pthread_barrier_init(&s_barrier, NULL, 3);

  pthread_create(&s_rx_pthread_id, NULL, prv_rx_thread, NULL);
  pthread_create(&s_tx_pthread_id, NULL, prv_tx_thread, NULL);

  pthread_barrier_wait(&s_barrier);
  pthread_barrier_destroy(&s_barrier);

  return STATUS_CODE_OK;
}

// Registers a callback for the given event
StatusCode can_hw_register_callback(CanHwEvent event, CanHwEventHandlerCb callback, void *context) {
  if (event >= NUM_CAN_HW_EVENTS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  s_socket_data.handlers[event] = (CanHwEventHandler){
    .callback = callback,  //
    .context = context,    //
  };

  return STATUS_CODE_OK;
}

StatusCode can_hw_add_filter(uint32_t mask, uint32_t filter, bool extended) {
  if (s_socket_data.num_filters >= CAN_HW_MAX_FILTERS) {
    return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "CAN HW: Ran out of filters.");
  }

  uint32_t reg_mask = extended ? CAN_EFF_MASK : CAN_SFF_MASK;
  uint32_t ide = extended ? CAN_EFF_FLAG : 0;
  s_socket_data.filters[s_socket_data.num_filters].can_id = (filter & reg_mask) | ide;
  s_socket_data.filters[s_socket_data.num_filters].can_mask = (mask & reg_mask) | CAN_EFF_FLAG;
  s_socket_data.num_filters++;

  if (setsockopt(s_socket_data.can_fd, SOL_CAN_RAW, CAN_RAW_FILTER, s_socket_data.filters,
                 sizeof(s_socket_data.filters[0]) * s_socket_data.num_filters) < 0) {
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "CAN HW: Failed to set raw filters");
  }

  return STATUS_CODE_OK;
}

CanHwBusStatus can_hw_bus_status(void) {
  return CAN_HW_BUS_STATUS_OK;
}

StatusCode can_hw_transmit(uint32_t id, bool extended, const uint8_t *data, size_t len) {
  uint32_t mask = extended ? CAN_EFF_MASK : CAN_SFF_MASK;
  uint32_t extended_bit = extended ? CAN_EFF_FLAG : 0;
  struct can_frame frame = { .can_id = (id & mask) | extended_bit, .can_dlc = len };
  memcpy(&frame.data, data, len);

  StatusCode ret = fifo_push(&s_socket_data.tx_fifo, &frame);
  if (ret != STATUS_CODE_OK) {
    // Fifo is full
    return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "CAN HW TX failed");
  }
  // Unblock TX thread
  sem_post(&s_tx_sem);

  return STATUS_CODE_OK;
}

// Must be called within the RX handler, returns whether a message was processed
bool can_hw_receive(uint32_t *id, bool *extended, uint64_t *data, size_t *len) {
  if (s_socket_data.rx_frame.can_id == 0) {
    // Assumes that we'll never transmit something with a CAN ID of all 0s
    return false;
  }

  *extended = !!(s_socket_data.rx_frame.can_id & CAN_EFF_FLAG);
  uint32_t mask = *extended ? CAN_EFF_MASK : CAN_SFF_MASK;
  *id = s_socket_data.rx_frame.can_id & mask;
  memcpy(data, s_socket_data.rx_frame.data, sizeof(*data));
  *len = s_socket_data.rx_frame.can_dlc;

  memset(&s_socket_data.rx_frame, 0, sizeof(s_socket_data.rx_frame));
  return true;
}
