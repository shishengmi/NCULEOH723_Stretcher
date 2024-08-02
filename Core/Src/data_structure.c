#include <stdio.h>
#include <stdlib.h>
#include "ADS1292.h"

#define BUFFER_SIZE 256  // 定义缓冲区大小

typedef struct {
    float32_t data[BUFFER_SIZE];  // 缓冲区数据存储
    int head;               // 指向缓冲区中第一个有效数据
    int tail;               // 指向下一个要写入的位置
    int size;               // 当前缓冲区中有效数据的数量
} CircularBuffer;

// 初始化缓冲区
void initBuffer(CircularBuffer* buffer) {
  buffer->head = 0;
  buffer->tail = 0;
  buffer->size = 0;
}

// 写入数据到缓冲区
void writeBuffer(CircularBuffer* buffer, float32_t value) {
  if (buffer->size < BUFFER_SIZE) {
    buffer->data[buffer->tail] = value;
    buffer->tail = (buffer->tail + 1) % BUFFER_SIZE;
    buffer->size++;
  } else {
    printf("Buffer is full, cannot write new data.\n");
  }
}

// 读取数据并从缓冲区中移除
float32_t readBuffer(CircularBuffer* buffer) {
  if (buffer->size > 0) {
    int value = buffer->data[buffer->head];
    buffer->head = (buffer->head + 1) % BUFFER_SIZE;
    buffer->size--;
    return value;
  } else {
    printf("Buffer is empty, cannot read data.\n");
    return 0;  // 返回一个错误值
  }
}



