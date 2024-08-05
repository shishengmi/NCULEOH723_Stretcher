#ifndef __QUEUE_H
#define __QUEUE_H


#include <stdint.h>
#include <stdbool.h>

// 队列数据类型和大小可以根据需要调整
typedef float QueueDataType;

typedef struct {
    QueueDataType *buffer;  // 动态数组指针
    int head;  // 头部索引
    int tail;  // 尾部索引
    int max_size;  // 最大容量
    int current_size;  // 当前元素个数
} Queue;

// 初始化队列
void Queue_Init(Queue *q, int max_size);

// 销毁队列
void Queue_Destroy(Queue *q);

// 入队
bool Queue_Enqueue(Queue *q, QueueDataType value);

// 出队
bool Queue_Dequeue(Queue *q, QueueDataType *value);

// 队列是否为空
bool Queue_IsEmpty(Queue *q);

// 队列是否已满
bool Queue_IsFull(Queue *q);

// 计算队列中数据的方差
float Queue_Variance(Queue *q);

// 遍历函数
QueueDataType* Queue_ToArray(Queue *q, int *size);

#endif // __QUEUE_H
