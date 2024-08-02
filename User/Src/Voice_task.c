#include "Voice_task.h"

/*!
 * 强烈注意，这个文件的格式必须是GB2312
 * @param pvParameters
 */


void Voice_Task(void *pvParameters) {

  SetVolume(10);
//  SetReader(Reader_XiaoPing);
  char buff[3] = "ABC";
  for (;;) {
    GetChipStatus();
//    speech_text("检测到患者体温过低",GB2312);
    vTaskDelay(pdMS_TO_TICKS(1000)); // 1秒延时，可以根据实际情况调整
  }
}
