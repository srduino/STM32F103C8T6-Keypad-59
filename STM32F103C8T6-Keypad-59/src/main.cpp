#include <Arduino.h>
// 运行模式(0:开发模式;1:运行模式)
int operationMode = 1;

// 引入USB复合库
#include <USBComposite.h>

// USB总线
USBHID HID;

// 键盘
HIDKeyboard Keyboard(HID); //普通按键
HIDConsumer Consumer(HID); //多媒体按键

// 鼠标(多媒体下不支持鼠标)
HIDMouse Mouse(HID);

// 多媒体键盘USB描述报文
const uint8_t reportDescription[] = {
    // 产品报告描述符
    HID_CONSUMER_REPORT_DESCRIPTOR(),
    // 键盘报告描述符
    HID_KEYBOARD_REPORT_DESCRIPTOR(),
};

// 键盘行数
const int numRows = 8;
// 键盘列数
const int numCols = 16;
// 开关稳定的毫秒数；去抖延时
const int debounceTime = 50;

/*
  软件时钟
*/
int Clock0 = 100; // 大小写切换占用

/*
  定义LED
*/
#define LED PA15
// LED状态
bool ledState;
// 大小写切换
bool CapsLockState;

// 键盘模式(0:白字小写; 1:白字大写; 2:Fn蓝键-WINDOWS; 3:Fn+w蓝键-WINDOWS; 4:Fn+q蓝键-Android; 5:Fn+e蓝键-IOS)
int keyboardMode = 0;

// 键盘队列
char KeyGroup[16] = {0};
// 按键临时队列
char releaseKeyGroup[4] = {0};

/*
  键盘库引入的说明:
  https://www.bilibili.com/read/cv8101978/
  键盘对应代码参考:
  https://wenku.baidu.com/view/b951d9d6c1c708a1284a443d.html

*/

//键盘地图0(白键小写)
const char keymap0[numRows][numCols] = {
    //  0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15
    {0XFF, 0XFF, 0XFF, 0x80, 0x60, 0XFF, 0XFF, '5', '6', '=', 0XFF, '-', 0XFF, 0XFF, 0XFF, 0XFF},   // 0
    {0XFF, 0XFF, 0XFF, 0XFF, '1', '2', '3', '4', '7', '8', '9', '0', 0XFF, 0XFF, 0XFF, 0XFF},       // 1
    {0XFF, 0XFF, 0XFF, 0XFF, 'q', 'w', 'e', 'r', 'o', 'i', 'u', 'p', 0XFF, 0XFF, 0XFF, 0XFF},       // 2
    {0XFF, 0x82, 0x81, 0XFF, 0x09, 0xC1, 0XFF, 't', 'y', 0x5C, 0XFF, 0X2F, 0XFF, 0xB2, 0XFF, 0XFF}, // 3
    {0XF0, 0XFF, 0XFF, 0XFF, 'a', 's', 'd', 'f', 'j', 'k', 'l', 0x0A, 0xB0, 0XFF, 0XFF, 0XFF},      // 4
    {0XFF, 0XFF, 0XFF, 0XFF, 'z', 'x', 'c', 0XFF, 'n', 'm', 0X2C, 0X3B, 0XFF, 0x85, 0XFF, 0XFF},    // 5
    {0x83, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 'g', 'h', 0XFF, 0XFF, 0XFF, 0xDA, 0XFF, 0x20, 0XFF}, // 6
    {0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 'v', 'b', 0XFF, 0XFF, 0X2E, 0xD8, 0XFF, 0xD9, 0xD7}, // 7
};
//键盘地图1(白键大写)
const char keymap1[numRows][numCols] = {
    //  0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15
    {0XFF, 0XFF, 0XFF, 0x80, 0x60, 0XFF, 0XFF, '5', '6', '=', 0XFF, '-', 0XFF, 0XFF, 0XFF, 0XFF},   // 0
    {0XFF, 0XFF, 0XFF, 0XFF, '1', '2', '3', '4', '7', '8', '9', '0', 0XFF, 0XFF, 0XFF, 0XFF},       // 1
    {0XFF, 0XFF, 0XFF, 0XFF, 'Q', 'W', 'E', 'R', 'O', 'I', 'U', 'P', 0XFF, 0XFF, 0XFF, 0XFF},       // 2
    {0XFF, 0x82, 0x81, 0XFF, 0x09, 0xC1, 0XFF, 'T', 'Y', 0x5C, 0XFF, 0X2F, 0XFF, 0xB2, 0XFF, 0XFF}, // 3
    {0XF0, 0XFF, 0XFF, 0XFF, 'A', 'S', 'D', 'F', 'J', 'K', 'L', 0x0A, 0xB0, 0XFF, 0XFF, 0XFF},      // 4
    {0XFF, 0XFF, 0XFF, 0XFF, 'Z', 'X', 'C', 0XFF, 'N', 'M', 0X2C, 0X3B, 0XFF, 0x85, 0XFF, 0XFF},    // 5
    {0x83, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 'G', 'H', 0XFF, 0XFF, 0XFF, 0xDA, 0XFF, 0x20, 0XFF}, // 6
    {0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 'V', 'B', 0XFF, 0XFF, 0X2E, 0xD8, 0XFF, 0xD9, 0xD7}, // 7
};
//键盘地图2(Fn蓝键-WINDOWS)
const char keymap2[numRows][numCols] = {
    //  0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15
    {0XFF, 0XFF, 0XFF, 0x80, 0xB1, 0XFF, 0XFF, 0xC6, 0xC7, 0xCD, 0XFF, 0xCC, 0XFF, 0XFF, 0XFF, 0XFF}, // 0
    {0XFF, 0XFF, 0XFF, 0XFF, 0xC2, 0xC3, 0xC4, 0xC5, 0xC8, 0xC9, 0xCA, 0xCB, 0XFF, 0XFF, 0XFF, 0XFF}, // 1
    {0XFF, 0XFF, 0XFF, 0XFF, 0XF1, 0XF2, 0XF3, 'r', 39, 'i', 'u', 34, 0XFF, 0XFF, 0XFF, 0XFF},        // 2
    {0XFF, 0x82, 0x81, 0XFF, 0x09, 0xC1, 0XFF, 't', 'y', 0x5C, 0XFF, 0X2F, 0XFF, 0xD4, 0XFF, 0XFF},   // 3
    {0XF0, 0XFF, 0XFF, 0XFF, 'a', 's', 'd', 'f', 'j', 'k', 91, 0x0A, 0xB0, 0XFF, 0XFF, 0XFF},         // 4
    {0XFF, 0XFF, 0XFF, 0XFF, 'z', 'x', 'c', 0XFF, 'n', 'm', 123, 93, 0XFF, 0x85, 0XFF, 0XFF},         // 5
    {0x83, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 'g', 'h', 0XFF, 0XFF, 0XFF, 0xD3, 0XFF, 0x20, 0XFF},   // 6
    {0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 'v', 'b', 0XFF, 0XFF, 125, 0xD2, 0XFF, 0xD6, 0xD5},    // 7
};
//键盘地图3(Fn+q蓝键-Android))
const char keymap3[numRows][numCols] = {
    //  0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15
    {0XFF, 0XFF, 0XFF, 0x80, 0xB1, 0XFF, 0XFF, 0xC6, 0xC7, 0xCD, 0XFF, 0xCC, 0XFF, 0XFF, 0XFF, 0XFF}, // 0
    {0XFF, 0XFF, 0XFF, 0XFF, 0xC2, 0xC3, 0xC4, 0xC5, 0xC8, 0xC9, 0xCA, 0xCB, 0XFF, 0XFF, 0XFF, 0XFF}, // 1
    {0XFF, 0XFF, 0XFF, 0XFF, 0XF1, 0XF2, 0XF3, 'r', 39, 'i', 'u', 34, 0XFF, 0XFF, 0XFF, 0XFF},        // 2
    {0XFF, 0x82, 0x81, 0XFF, 0x09, 0xC1, 0XFF, 't', 'y', 0x5C, 0XFF, 0X2F, 0XFF, 0xD4, 0XFF, 0XFF},   // 3
    {0XF0, 0XFF, 0XFF, 0XFF, 'a', 's', 'd', 'f', 'j', 'k', 91, 0x0A, 0xB0, 0XFF, 0XFF, 0XFF},         // 4
    {0XFF, 0XFF, 0XFF, 0XFF, 'z', 'x', 'c', 0XFF, 'n', 'm', 123, 93, 0XFF, 0x85, 0XFF, 0XFF},         // 5
    {0x83, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 'g', 'h', 0XFF, 0XFF, 0XFF, 0xD3, 0XFF, 0x20, 0XFF},   // 6
    {0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 'v', 'b', 0XFF, 0XFF, 125, 0xD2, 0XFF, 0xD6, 0xD5},    // 7
};
//键盘地图4(Fn+w蓝键-Windows)
const char keymap4[numRows][numCols] = {
    //  0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15
    {0XFF, 0XFF, 0XFF, 0x80, 0xB1, 0XFF, 0XFF, 0xC6, 0xC7, 0XF2, 0XFF, 0XF0, 0XFF, 0XFF, 0XFF, 0XFF}, // 0
    {0XFF, 0XFF, 0XFF, 0XFF, 0xC2, 0xC3, 0xC4, 0xC5, 0xC8, 0xC9, 0xCA, 0xF1, 0XFF, 0XFF, 0XFF, 0XFF}, // 1
    {0XFF, 0XFF, 0XFF, 0XFF, 0XF1, 0XF2, 0XF3, 'r', 39, 'i', 'u', 34, 0XFF, 0XFF, 0XFF, 0XFF},        // 2
    {0XFF, 0x82, 0x81, 0XFF, 0x09, 0xC1, 0XFF, 't', 'y', 0x5C, 0XFF, 0X2F, 0XFF, 0xD4, 0XFF, 0XFF},   // 3
    {0XF0, 0XFF, 0XFF, 0XFF, 'a', 's', 'd', 'f', 'j', 'k', 91, 0x0A, 0xB0, 0XFF, 0XFF, 0XFF},         // 4
    {0XFF, 0XFF, 0XFF, 0XFF, 'z', 'x', 'c', 0XFF, 'n', 'm', 123, 93, 0XFF, 0x85, 0XFF, 0XFF},         // 5
    {0x83, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 'g', 'h', 0XFF, 0XFF, 0XFF, 0xD3, 0XFF, 0x20, 0XFF},   // 6
    {0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 'v', 'b', 0XFF, 0XFF, 125, 0xD2, 0XFF, 0xD6, 0xD5},    // 7
};
//键盘地图5(Fn+e蓝键-IOS)
const char keymap5[numRows][numCols] = {
    //  0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15
    {0XFF, 0XFF, 0XFF, 0x80, 0xB1, 0XFF, 0XFF, 0xC6, 0xC7, 0xCD, 0XFF, 0xCC, 0XFF, 0XFF, 0XFF, 0XFF}, // 0
    {0XFF, 0XFF, 0XFF, 0XFF, 0xC2, 0xC3, 0xC4, 0xC5, 0xC8, 0xC9, 0xCA, 0xCB, 0XFF, 0XFF, 0XFF, 0XFF}, // 1
    {0XFF, 0XFF, 0XFF, 0XFF, 0XF1, 0XF2, 0XF3, 'r', 39, 'i', 'u', 34, 0XFF, 0XFF, 0XFF, 0XFF},        // 2
    {0XFF, 0x82, 0x81, 0XFF, 0x09, 0xC1, 0XFF, 't', 'y', 0x5C, 0XFF, 0X2F, 0XFF, 0xD4, 0XFF, 0XFF},   // 3
    {0XF0, 0XFF, 0XFF, 0XFF, 'a', 's', 'd', 'f', 'j', 'k', 91, 0x0A, 0xB0, 0XFF, 0XFF, 0XFF},         // 4
    {0XFF, 0XFF, 0XFF, 0XFF, 'z', 'x', 'c', 0XFF, 'n', 'm', 123, 93, 0XFF, 0x85, 0XFF, 0XFF},         // 5
    {0x83, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 'g', 'h', 0XFF, 0XFF, 0XFF, 0xD3, 0XFF, 0x20, 0XFF},   // 6
    {0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 'v', 'b', 0XFF, 0XFF, 125, 0xD2, 0XFF, 0xD6, 0xD5},    // 7
};

//这个数组确定用于行和列的引脚

const int rowPins[numRows] = {PA1, PA2, PA3, PA4, PA5, PA6, PA7, PB0};                                                 //0-7行
const int colPins[numCols] = {PB9, PB8, PB7, PB6, PB5, PB1, PB10, PB11, PA8, PB15, PB14, PB13, PB12, PC14, PC15, PC13}; //0-15列

/*
  根据键盘线路分配组合的按键
   为了最大程序兼容多按键,根据线路布局设计
   详细内容根据PRIDE-K1311 VER:B (59键)矩阵分配酌情处理
*/

void getKey()
{
  // 0表示没有按键按下
  char key = 0;

  //同时又多少个按键计数
  int keyCount = 0;

  // 初始化队列
  for (int i = 0; i < 16; i++)
  {
    KeyGroup[i] = 0;
  }

  // 扫描列
  for (int col = 0; col < numCols; col++)
  {
    // 激活当前列
    digitalWrite(colPins[col], LOW);

    // 扫描行
    for (int row = 0; row < numRows; row++)
    {
      // 检测行触发
      if (digitalRead(rowPins[row]) == LOW)
      {
        // 判断键盘类型
        switch (keyboardMode)
        {
        case 0:
          // 白字小写键盘
          key = keymap0[row][col];
          break;
        case 1:
          // 白字大写键盘
          key = keymap1[row][col];
          break;
        case 2:
          // Fn蓝键-Windows
          key = keymap2[row][col];
          break;
        case 3:
          // Fn+w蓝键-WINDOWS
          key = keymap3[row][col];
          break;
        case 4:
          // Fn+q蓝键-Android
          key = keymap4[row][col];
          break;
        case 5:
          // Fn+e蓝键-IOS
          key = keymap4[row][col];
          break;
        default:
          key = keymap0[row][col];
          break;
        }

        // 累加同时按下的按键
        keyCount++;

        // 赋值按键队列值
        KeyGroup[keyCount] = key;
        // 去抖
        delay(debounceTime); //去抖

        // 调试信息
        if (operationMode == 0)
        {
          Serial.print("keyCount:");
          Serial.print(keyCount);
          Serial.print(",row:");
          Serial.print(row);
          Serial.print(",col:");
          Serial.print(col);
          Serial.print(",key:");
          Serial.print(key);
        }
      }
    }
    //初始化列
    digitalWrite(colPins[col], HIGH);
  }
}

void setup()
{
  //初始化串口
  Serial.begin(115200);

  // 初始化大小写状态
  CapsLockState = false;

  /**初始化管脚状态*/
  // 初始化LED
  pinMode(LED, OUTPUT);
  ledState = true;

  // 批量设置矩阵行
  for (int row = 0; row < numRows; row++)
  {
    pinMode(rowPins[row], INPUT_PULLUP); //设置行引脚为输入，这里要注意模式选用内部上拉
    digitalWrite(rowPins[row], HIGH);    //打开上拉电阻
  }

  // 批量设置矩阵列
  for (int column = 0; column < numCols; column++)
  {
    pinMode(colPins[column], OUTPUT); //设置列引脚为输出
    digitalWrite(colPins[column], HIGH); //使所有列不活跃
  }

  // 闪烁LED表示启动完成
  digitalWrite(LED, LOW);
  delay(100);
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  delay(100);
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  delay(100);
  digitalWrite(LED, HIGH);

  // 延时1秒
  delay(1000);

  // 定义USB为键盘
  // HID.begin(HID_KEYBOARD);
  // 定义USB为键盘和鼠标并初始化
  // HID.begin(HID_KEYBOARD_MOUSE);
  // 初始化多媒体键盘
  HID.begin(reportDescription, sizeof(reportDescription));
  // 初始化键盘(如果不初始化,在多键盘模式下会导致大小写控制出现冲突)
  Keyboard.begin();
}

void loop()
{

  // 启动时钟
  Clock0++;
  if (Clock0 == 200)
  {
    Clock0 = 100;
  }

  // 开始扫描键盘
  getKey();

  // 调试信息
  if (operationMode == 0)
  {
    uint8_t modifiers;

    // 打印调试按键队列信息
    Serial.print(",keyboardMode:");
    Serial.print(keyboardMode);
    Serial.print(",Key1:");
    Serial.print(KeyGroup[1]);
    Serial.print(",Key2");
    Serial.print(KeyGroup[2]);
    Serial.print(",Key3");
    Serial.println(KeyGroup[3]);
  }

  // 切换大小写状态(由于扫描太快,加一个计数时钟来处理)
  if (KeyGroup[1] == 0xC1 && Clock0 != 0)
  {
    Clock0 = 0;
  }
  if (Clock0 == 1)
  {
    CapsLockState = !CapsLockState;
  }
  // 判断大小写状态
  if (CapsLockState == true)
  {
    // 点亮LED
    ledState = false;
    // 切换键盘模式
    keyboardMode = 1;
  }
  else
  {
    // 熄灭LED
    ledState = true;
    keyboardMode = 0;
  }

  // Fn按键处理
  if (KeyGroup[1] == 0xF0)
  {
    // 切换键盘模式
    keyboardMode = 1;
  }

  // Fn按键处理
  if (KeyGroup[1] == 0xF0)
  {
    // 切换键盘模式(Fn蓝键)
    keyboardMode = 2;
  }

  // Fn + q按键处理
  if (KeyGroup[1] == 0xF0 && KeyGroup[2] == 0XF1)
  {
    // 切换键盘模式(Fn+q蓝键-Android)
    keyboardMode = 3;
  }
  // Fn + w按键处理
  if (KeyGroup[1] == 0xF0 && KeyGroup[2] == 0XF2)
  {
    // 切换键盘模式(Fn+w蓝键-WINDOWS)
    keyboardMode = 4;
  }

  // Fn + w + F11按键处理
  if (keyboardMode == 4 && KeyGroup[3] == 0XF0)
  {
    // 音量加
    Consumer.press(HIDConsumer::VOLUME_UP);
  }else{
    Consumer.release();
  }

  // Fn + w + F10按键处理
  if (keyboardMode == 4 && KeyGroup[3] == 0XF1)
  {
    // 音量减
    Consumer.press(HIDConsumer::VOLUME_DOWN);
  }else{
    Consumer.release();
  }

  // Fn + w + F12按键处理
  if (keyboardMode == 4 && KeyGroup[3] == 0XF2)
  {
    // 静音
    Consumer.press(HIDConsumer::MUTE);
  }else{
    Consumer.release();
  }

  // Fn + e 按键处理
  if (KeyGroup[1] == 0xF0 && KeyGroup[2] == 0XF3)
  {
    // 切换键盘模式(Fn+e蓝键-IOS)
    keyboardMode = 5;
  }

  // 根据队列输出按键
  if (KeyGroup[1])
  {
    releaseKeyGroup[1] = KeyGroup[1];
    Keyboard.press(KeyGroup[1]);
  }
  else
  {
    Keyboard.release(releaseKeyGroup[1]);
    releaseKeyGroup[1] = 0;
  }

  // 队列二按键
  if (KeyGroup[2])
  {
    // 临时缓存第二按键队列,这是在第二按键松开时,需要的条件,要不导致问题
    releaseKeyGroup[2] = KeyGroup[2];
    Keyboard.press(KeyGroup[2]);
  }
  else
  {
    Keyboard.release(releaseKeyGroup[2]);
    releaseKeyGroup[2] = 0;
  }

  // 队列三按键
  if (KeyGroup[3])
  {
    releaseKeyGroup[3] = KeyGroup[3];
    Keyboard.press(KeyGroup[3]);
  }
  else
  {
    Keyboard.release(releaseKeyGroup[3]);
    releaseKeyGroup[3] = 0;
  }

  // 队列四按键
  if (KeyGroup[4])
  {
    releaseKeyGroup[4] = KeyGroup[4];
    Keyboard.press(KeyGroup[4]);
  }
  else
  {
    Keyboard.release(releaseKeyGroup[4]);
    releaseKeyGroup[4] = 0;
  }

  // 释放所有按键
  if (!KeyGroup[1] && !KeyGroup[2] && !KeyGroup[3])
  {
    // 清空按键临时队列
    for (int i = 0; i < 3; i++)
    {
      releaseKeyGroup[i] = 0;
    }
    // 释放所有按键
    Keyboard.releaseAll();
    // 释放多媒体按键
    Consumer.release();
  }

  // 刷新LED状态
  digitalWrite(LED, ledState);

}