#ifndef FILTERING_H__
#define FILTERING_H__
#endif


#include "ADS1292.h"


#define Samples_Number  1
#define Block_Size      1
#define NumTaps        	129
#define BUFFER_SIZE     256



void Filtering(float32_t * Input_data,float32_t * Output_data);
void Filtering_Init(void);


