#include "data_structure.h"

void initFloatQueue(FloatQueue* q, int capacity) {
    q->capacity = capacity;
    q->front = 0;
    q->rear = -1;
    q->size = 0;
    q->items = (float32_t*)malloc(sizeof(float32_t) * capacity);
}


bool enqueueFloat(FloatQueue* q, float32_t item) {
    if (q->size == q->capacity) {
        return false; // 队列已满
    }
    q->rear = (q->rear + 1) % q->capacity;
    q->items[q->rear] = item;
    q->size++;
    return true;
}


bool dequeueFloat(FloatQueue* q, float32_t* item) {
    if (q->size == 0) {
        return false; // 队列为空
    }
    *item = q->items[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->size--;
    return true;
}


bool peekFloat(FloatQueue* q, float32_t* item) {
    if (q->size == 0) {
        return false; // 队列为空
    }
    *item = q->items[q->front];
    return true;
}


bool isFloatQueueEmpty(FloatQueue* q) {
    return q->size == 0;
}

bool isFloatQueueFull(FloatQueue* q) {
    return q->size == q->capacity;
}


void freeFloatQueue(FloatQueue* q) {
    free(q->items);
}
