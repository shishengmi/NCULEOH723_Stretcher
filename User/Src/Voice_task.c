#include "Voice_task.h"

/*!
 * ǿ��ע�⣬����ļ��ĸ�ʽ������GB2312
 * @param pvParameters
 */


void Voice_Task(void *pvParameters) {

  SetVolume(10);
//  SetReader(Reader_XiaoPing);
  char buff[3] = "ABC";
  for (;;) {
    GetChipStatus();
//    speech_text("��⵽�������¹���",GB2312);
    vTaskDelay(pdMS_TO_TICKS(1000)); // 1����ʱ�����Ը���ʵ���������
  }
}
