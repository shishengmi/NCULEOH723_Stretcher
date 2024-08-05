#include "queue.h"
#include <stdlib.h>

// 初始化队列
void Queue_Init(Queue *q, int max_size) {
    q->buffer = (QueueDataType *)malloc(sizeof(QueueDataType) * max_size);
    q->max_size = max_size;
    q->head = 0;
    q->tail = 0;
    q->current_size = 0;
}

// 销毁队列
void Queue_Destroy(Queue *q) {
    free(q->buffer);
    q->buffer = NULL;
    q->max_size = 0;
    q->head = 0;
    q->tail = 0;
    q->current_size = 0;
}

// 入队
bool Queue_Enqueue(Queue *q, QueueDataType value) {
    if (Queue_IsFull(q)) {
        return false;
    }

    q->buffer[q->tail] = value;
    q->tail = (q->tail + 1) % q->max_size;
    q->current_size++;
    return true;
}

// 出队
bool Queue_Dequeue(Queue *q, QueueDataType *value) {
    if (Queue_IsEmpty(q)) {
        return false;
    }

    *value = q->buffer[q->head];
    q->head = (q->head + 1) % q->max_size;
    q->current_size--;
    return true;
}

// 队列是否为空
bool Queue_IsEmpty(Queue *q) {
    return q->current_size == 0;
}

// 队列是否已满
bool Queue_IsFull(Queue *q) {
    return q->current_size == q->max_size;
}

// 计算队列中数据的方差
float Queue_Variance(Queue *q) {
    if (Queue_IsEmpty(q)) {
        return 0.0f;
    }

    float mean = 0.0f;
    float variance = 0.0f;
    int index = q->head;

    // 计算平均值
    for (int i = 0; i < q->current_size; i++) {
        mean += q->buffer[index];
        index = (index + 1) % q->max_size;
    }
    mean /= q->current_size;

    // 计算方差
    index = q->head;
    for (int i = 0; i < q->current_size; i++) {
        float diff = q->buffer[index] - mean;
        variance += diff * diff;
        index = (index + 1) % q->max_size;
    }
    variance /= q->current_size;

    return variance;
}

// 将队列中的元素返回到数组中
QueueDataType* Queue_ToArray(Queue *q, int *size) {
    if (Queue_IsEmpty(q)) {
        *size = 0;
        return NULL;
    }

    QueueDataType *array = (QueueDataType *)malloc(sizeof(QueueDataType) * q->current_size);
    int index = q->head;

    for (int i = 0; i < q->current_size; i++) {
        array[i] = q->buffer[index];
        index = (index + 1) % q->max_size;
    }

    *size = q->current_size;
    return array;
}
