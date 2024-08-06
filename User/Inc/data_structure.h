#ifndef __DATA_STRUCTURE_H
#define __DATA_STRUCTURE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// 定义一个浮点数类型，用于队列中的数据项
typedef float float32_t;

// 定义浮点数队列的结构体
typedef struct {
    float32_t* items;   // 存储队列元素的数组
    int capacity;       // 队列的最大容量
    int front;          // 队列头部的索引
    int rear;           // 队列尾部的索引
    int size;           // 队列当前的元素数量
} FloatQueue;

// 初始化队列的函数
void initFloatQueue(FloatQueue* q, int capacity);

// 将元素加入队列的函数
bool enqueueFloat(FloatQueue* q, float32_t item);

// 从队列中取出元素的函数
bool dequeueFloat(FloatQueue* q, float32_t* item);

// 获取队列首元素的函数，但不移除该元素
bool peekFloat(FloatQueue* q, float32_t* item);

// 检查队列是否为空的函数
bool isFloatQueueEmpty(FloatQueue* q);

// 检查队列是否已满的函数
bool isFloatQueueFull(FloatQueue* q);

// 获取队列当前元素数量的函数
int getFloatQueueSize(FloatQueue* q);

// 释放队列相关资源的函数
void freeFloatQueue(FloatQueue* q);

#endif // __DATA_STRUCTURE_H
