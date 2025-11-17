#include "messaging.h"
#define MAX_MESSAGE_QUEUES (MAX_QUEUES_PER_TASK * MAX_TASKS)

#define QUEUE_TYPENAME message_data_queue
#define QUEUE_TYPE uint8_t
#define QUEUE_CAPACITY MESSAGE_QUEUE_SIZE
#include "queue.h"

typedef struct {
  message_data_queue data_queue;
  TASK_HANDLE handle;
  bool closed;
  bool is_awaiting_read;
  bool is_awaiting_write;
} message_queue_side;

typedef struct {
  message_queue_side side1;
  message_queue_side side2;
} message_queue;

#define HANDLED_LIST_TYPENAME message_queue_list
#define HANDLED_LIST_TYPE message_queue
#define HANDLED_LIST_CAPACITY MAX_MESSAGE_QUEUES
#define HANDLED_LIST_HANDLE_NAME MESSAGE_QUEUE_HANDLE
#include "handledlist.h"

message_queue_list message_queues;

#define QUEUE_TYPENAME queue_queue
#define QUEUE_TYPE MESSAGE_QUEUE_HANDLE
#define QUEUE_CAPACITY MAX_QUEUES_PER_TASK
#include "queue.h"

queue_queue queues_per_task[MAX_TASKS];

void messaging_init() {
  message_queue_list_init(&message_queues);
  for (int i = 0; i < MAX_TASKS; i++) {
    queue_queue_init(&queues_per_task[i]);
  }
}

static TASK_HANDLE looking_for_handle1;
static TASK_HANDLE looking_for_handle2;
bool look_for_handle(message_queue queue) {
  return (queue.side1.handle == looking_for_handle1 &&
          queue.side2.handle == looking_for_handle2) ||
         (queue.side1.handle == looking_for_handle2 &&
          queue.side1.handle == looking_for_handle1);
}

message_q_error messaging_queue_create(
    TASK_HANDLE handle, MESSAGE_QUEUE_HANDLE* q_handle
) {
  MESSAGE_QUEUE_HANDLE queue_handle;
  looking_for_handle1 = handle;
  looking_for_handle2 = get_current_task().task_handle;
  bool found =
      message_queue_list_find(&message_queues, look_for_handle, &queue_handle);
  if (found) {
    *q_handle = queue_handle;
  } else {
    message_queue queue = {
        .side1 =
            {
                    .closed = false,
                    .handle = get_current_task().task_handle,
                    },
        .side2 = {.closed = false, .handle = handle}
    };
    message_data_queue_init(&queue.side1.data_queue);
    message_data_queue_init(&queue.side2.data_queue);
    if (!message_queue_list_insert(&message_queues, queue, q_handle)) {
      return MESSAGE_QUEUE_TOO_MANY_QUEUES;
    }
    queue_queue_enqueue(
        &queues_per_task[get_current_task().task_handle],
        *q_handle
    );
    queue_queue_enqueue(&queues_per_task[handle], *q_handle);
  }
  return MESSAGE_QUEUE_OK;
}

message_q_error message_queue_write(
    MESSAGE_QUEUE_HANDLE q_handle, void* const data, size_t size
) {
  message_queue* queue;
  if (!message_queue_list_get_ref(&message_queues, q_handle, &queue)) {
    return MESSAGE_QUEUE_INVALID_HANDLE;
  }
  TASK_HANDLE current_task = get_current_task().task_handle;
  message_queue_side* this_side =
      current_task == queue->side1.handle ? &queue->side1 : &queue->side2;
  message_queue_side* receiving_side =
      current_task == queue->side1.handle ? &queue->side2 : &queue->side1;
  size_t sent = 0;
  while (sent < size) {
    if (receiving_side->closed) {
      return MESSAGE_QUEUE_CLOSED;
    }
    sent += message_data_queue_enqueue_all(&this_side->data_queue, size, data);
    if (receiving_side->is_awaiting_write) {
      receiving_side->is_awaiting_write = false;
      wake_task(receiving_side->handle);
    }
    if (sent < size) {
      if (receiving_side->is_awaiting_read) {
        return MESSAGE_QUEUE_DEADLOCK;
      }
      this_side->is_awaiting_read = true;
      sleep_task(current_task);
    }
  }
  return MESSAGE_QUEUE_OK;
}

message_q_error message_queue_read(
    MESSAGE_QUEUE_HANDLE q_handle, void* data, size_t buffer_size
) {
  message_queue* queue;
  if (!message_queue_list_get_ref(&message_queues, q_handle, &queue)) {
    return MESSAGE_QUEUE_INVALID_HANDLE;
  }

  TASK_HANDLE current_task = get_current_task().task_handle;
  message_queue_side* sending_side =
      current_task == queue->side1.handle ? &queue->side2 : &queue->side1;
  message_queue_side* this_side =
      current_task == queue->side1.handle ? &queue->side1 : &queue->side2;

  size_t recv = 0;
  while (recv < buffer_size) {
    recv += message_data_queue_dequeue_all(
        &sending_side->data_queue,
        buffer_size,
        data
    );
    if (sending_side->closed) {
      return MESSAGE_QUEUE_CLOSED;
    }
    if (sending_side->is_awaiting_read) {
      sending_side->is_awaiting_read = false;
      wake_task(sending_side->handle);
    }
    if (recv < buffer_size) {
      if (sending_side->is_awaiting_write) {
        return MESSAGE_QUEUE_DEADLOCK;
      }
      this_side->is_awaiting_write = true;
      sleep_task(current_task);
    }
  }
  return MESSAGE_QUEUE_OK;
}

message_q_error message_queue_data_available(
    MESSAGE_QUEUE_HANDLE q_handle, size_t* data
) {
  message_queue queue;
  if (!message_queue_list_get(&message_queues, q_handle, &queue)) {
    return MESSAGE_QUEUE_INVALID_HANDLE;
  }

  TASK_HANDLE current_task = get_current_task().task_handle;
  message_queue_side sending_side =
      current_task == queue.side1.handle ? queue.side2 : queue.side1;
  *data = message_data_queue_size(&sending_side.data_queue);
  if (sending_side.closed) {
    return MESSAGE_QUEUE_CLOSED;
  }
  return MESSAGE_QUEUE_OK;
}

message_q_error messaging_close_queues(TASK_HANDLE task) {
  if (!valid_handle(task)) {
    return MESSAGE_QUEUE_INVALID_TASK;
  }
  MESSAGE_QUEUE_HANDLE q_handle;
  while (queue_queue_dequeue(&queues_per_task[task], &q_handle)) {
    message_queue queue;
    if (!message_queue_list_get(&message_queues, q_handle, &queue)) {
      // Invalid handle got into the queue
      // TODO: Hardfault
    }
    message_queue_side this_side =
        task == queue.side1.handle ? queue.side1 : queue.side2;
    this_side.closed = true;
  }
  return true;
}
