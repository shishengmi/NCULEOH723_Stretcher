/**
 * ************************************************************************
 *
 * @file blood.h
 * @author zxr
 * @brief
 *
 * ************************************************************************
 * @copyright Copyright (c) 2024 zxr
 * ************************************************************************
 */
#ifndef _BLOOD_H
#define _BLOOD_H

#include "main.h"
#include "MAX30102.h"
#include "algorithm.h"
#include "math.h"
#include "stdio.h"



void blood_data_translate(void);
void blood_data_update(void);
float blood_Loop(void);

#endif



