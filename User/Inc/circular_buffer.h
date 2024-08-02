#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"

#define BUFFER_SIZE 256

typedef struct {
    QueueHandle_t queue; // FreeRTOS 队列句柄
} CircularBuffer;

void initBuffer(CircularBuffer* buffer);
int writeBuffer(CircularBuffer* buffer, int value);
int readBuffer(CircularBuffer* buffer, int* value);
int isBufferFull(CircularBuffer* buffer);
int isBufferEmpty(CircularBuffer* buffer);

#endif // CIRCULAR_BUFFER_H
