#include "fifo.h"
#include <string.h>
#include "critical_section.h"

StatusCode fifo_init_impl(Fifo *fifo, void *buffer, size_t elem_size, size_t num_elems) {
  memset(fifo, 0, sizeof(*fifo));
  memset(buffer, 0, num_elems * elem_size);

  fifo->buffer = buffer;
  fifo->end = (uint8_t *)buffer + num_elems * elem_size;
  fifo->head = buffer;
  fifo->next = buffer;
  fifo->max_elems = num_elems;
  fifo->elem_size = elem_size;

  return STATUS_CODE_OK;
}

size_t fifo_size(Fifo *fifo) {
  bool disabled = critical_section_start();
  size_t fifo_size = fifo->num_elems;
  critical_section_end(disabled);

  return fifo_size;
}

StatusCode fifo_push_impl(Fifo *fifo, void *source_elem, size_t elem_size) {
  if (fifo->num_elems == fifo->max_elems) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  } else if (fifo->elem_size != elem_size) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  bool disabled = critical_section_start();
  memcpy(fifo->next, source_elem, fifo->elem_size);

  *(uint8_t **)&fifo->next += elem_size;
  if (fifo->next >= fifo->end) {
    fifo->next = fifo->buffer;
  }

  fifo->num_elems++;
  critical_section_end(disabled);

  return STATUS_CODE_OK;
}

StatusCode fifo_peek_impl(Fifo *fifo, void *dest_elem, size_t elem_size) {
  if (fifo->num_elems == 0) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  } else if (fifo->elem_size != elem_size && dest_elem != NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  bool disabled = critical_section_start();
  if (dest_elem != NULL) {
    memcpy(dest_elem, fifo->head, fifo->elem_size);
  }
  critical_section_end(disabled);

  return STATUS_CODE_OK;
}

StatusCode fifo_pop_impl(Fifo *fifo, void *dest_elem, size_t elem_size) {
  if (fifo->num_elems == 0) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  } else if (fifo->elem_size != elem_size && dest_elem != NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  bool disabled = critical_section_start();
  if (dest_elem != NULL) {
    memcpy(dest_elem, fifo->head, fifo->elem_size);
  }

  memset(fifo->head, 0, fifo->elem_size);

  *(uint8_t **)&fifo->head += fifo->elem_size;
  if (fifo->head >= fifo->end) {
    fifo->head = fifo->buffer;
  }

  fifo->num_elems--;
  critical_section_end(disabled);

  return STATUS_CODE_OK;
}

StatusCode fifo_push_arr_impl(Fifo *fifo, void *source_arr, size_t elem_size, size_t num_elems) {
  if (fifo->num_elems + num_elems > fifo->max_elems) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  } else if (fifo->elem_size != elem_size) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  bool disabled = critical_section_start();
  uint8_t *new_next = (uint8_t *)fifo->next + fifo->elem_size * num_elems;

  size_t wrap_bytes = 0;
  if ((uint8_t *)fifo->end < new_next) {
    wrap_bytes = (size_t)(new_next - (uint8_t *)fifo->end);
  }

  size_t nonwrap_bytes = fifo->elem_size * num_elems - wrap_bytes;
  memcpy(fifo->next, source_arr, nonwrap_bytes);
  *(uint8_t **)&fifo->next += nonwrap_bytes;
  if (fifo->next >= fifo->end) {
    fifo->next = fifo->buffer;
  }

  if (wrap_bytes > 0) {
    memcpy(fifo->next, (uint8_t *)source_arr + nonwrap_bytes, wrap_bytes);
    *(uint8_t **)&fifo->next += wrap_bytes;
  }

  fifo->num_elems += num_elems;
  critical_section_end(disabled);

  return STATUS_CODE_OK;
}

StatusCode fifo_pop_arr_impl(Fifo *fifo, void *dest_arr, size_t elem_size, size_t num_elems) {
  if (fifo->num_elems < num_elems) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  } else if (fifo->elem_size != elem_size && dest_arr != NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  bool disabled = critical_section_start();
  uint8_t *new_head = (uint8_t *)fifo->head + fifo->elem_size * num_elems;

  size_t wrap_bytes = 0;
  if ((uint8_t *)fifo->end < new_head) {
    wrap_bytes = (size_t)(new_head - (uint8_t *)fifo->end);
  }

  size_t nonwrap_bytes = fifo->elem_size * num_elems - wrap_bytes;

  if (dest_arr != NULL) {
    memcpy(dest_arr, fifo->head, nonwrap_bytes);
  }
  memset(fifo->head, 0, nonwrap_bytes);

  *(uint8_t **)&fifo->head += nonwrap_bytes;
  if (fifo->head >= fifo->end) {
    fifo->head = fifo->buffer;
  }

  if (wrap_bytes > 0) {
    if (dest_arr != NULL) {
      memcpy((uint8_t *)dest_arr + nonwrap_bytes, fifo->head, wrap_bytes);
    }
    memset(fifo->head, 0, wrap_bytes);
    *(uint8_t **)&fifo->head += wrap_bytes;
  }

  fifo->num_elems -= num_elems;
  critical_section_end(disabled);

  return STATUS_CODE_OK;
}
