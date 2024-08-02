#include "circular_buffer.h"

// 初始化缓冲区
void initBuffer(CircularBuffer* buffer) {
  buffer->queue = xQueueCreate(BUFFER_SIZE, sizeof(int));
}

// 写入数据到缓冲区
int writeBuffer(CircularBuffer* buffer, int value) {
  if (xQueueSend(buffer->queue, &value, 0) == pdPASS) {
    return 0; // 写入成功
  } else {
    // 如果队列已满，先读取并丢弃一个元素再写入新数据
    int discarded;
    xQueueReceive(buffer->queue, &discarded, 0);
    xQueueSend(buffer->queue, &value, 0);
    return 0; // 写入成功
  }
}

// 读取数据从缓冲区中移除
int readBuffer(CircularBuffer* buffer, int* value) {
  if (xQueueReceive(buffer->queue, value, portMAX_DELAY) == pdPASS) {
    return 0; // 读取成功
  } else {
    return -1; // 读取失败
  }
}

// 检查缓冲区是否已满
int isBufferFull(CircularBuffer* buffer) {
  return uxQueueSpacesAvailable(buffer->queue) == 0;
}

// 检查缓冲区是否为空
int isBufferEmpty(CircularBuffer* buffer) {
  return uxQueueMessagesWaiting(buffer->queue) == 0;
}
