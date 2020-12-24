#include <Arduino.h>
// ����ģʽ(0:����ģʽ;1:����ģʽ)
int operationMode = 1;

// ����USB���Ͽ�
#include <USBComposite.h>

// USB����
USBHID HID;

// ����
HIDKeyboard Keyboard(HID); //��ͨ����
HIDConsumer Consumer(HID); //��ý�尴��

// ���(��ý���²�֧�����)
HIDMouse Mouse(HID);

// ��ý�����USB��������
const uint8_t reportDescription[] = {
    // ��Ʒ����������
    HID_CONSUMER_REPORT_DESCRIPTOR(),
    // ���̱���������
    HID_KEYBOARD_REPORT_DESCRIPTOR(),
};

// ��������
const int numRows = 8;
// ��������
const int numCols = 16;
// �����ȶ��ĺ�������ȥ����ʱ
const int debounceTime = 50;

/*
  ���ʱ��
*/
int Clock0 = 100; // ��Сд�л�ռ��

/*
  ����LED
*/
#define LED PA15
// LED״̬
bool ledState;
// ��Сд�л�
bool CapsLockState;

// ����ģʽ(0:����Сд; 1:���ִ�д; 2:Fn����-WINDOWS; 3:Fn+w����-WINDOWS; 4:Fn+q����-Android; 5:Fn+e����-IOS)
int keyboardMode = 0;

// ���̶���
char KeyGroup[16] = {0};
// ������ʱ����
char releaseKeyGroup[4] = {0};

/*
  ���̿������˵��:
  https://www.bilibili.com/read/cv8101978/
  ���̶�Ӧ����ο�:
  https://wenku.baidu.com/view/b951d9d6c1c708a1284a443d.html

*/

//���̵�ͼ0(�׼�Сд)
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
//���̵�ͼ1(�׼���д)
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
//���̵�ͼ2(Fn����-WINDOWS)
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
//���̵�ͼ3(Fn+q����-Android))
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
//���̵�ͼ4(Fn+w����-Windows)
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
//���̵�ͼ5(Fn+e����-IOS)
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

//�������ȷ�������к��е�����

const int rowPins[numRows] = {PA1, PA2, PA3, PA4, PA5, PA6, PA7, PB0};                                                 //0-7��
const int colPins[numCols] = {PB9, PB8, PB7, PB6, PB5, PB1, PB10, PB11, PA8, PB15, PB14, PB13, PB12, PC14, PC15, PC13}; //0-15��

/*
  ���ݼ�����·������ϵİ���
   Ϊ����������ݶఴ��,������·�������
   ��ϸ���ݸ���PRIDE-K1311 VER:B (59��)����������鴦��
*/

void getKey()
{
  // 0��ʾû�а�������
  char key = 0;

  //ͬʱ�ֶ��ٸ���������
  int keyCount = 0;

  // ��ʼ������
  for (int i = 0; i < 16; i++)
  {
    KeyGroup[i] = 0;
  }

  // ɨ����
  for (int col = 0; col < numCols; col++)
  {
    // ���ǰ��
    digitalWrite(colPins[col], LOW);

    // ɨ����
    for (int row = 0; row < numRows; row++)
    {
      // ����д���
      if (digitalRead(rowPins[row]) == LOW)
      {
        // �жϼ�������
        switch (keyboardMode)
        {
        case 0:
          // ����Сд����
          key = keymap0[row][col];
          break;
        case 1:
          // ���ִ�д����
          key = keymap1[row][col];
          break;
        case 2:
          // Fn����-Windows
          key = keymap2[row][col];
          break;
        case 3:
          // Fn+w����-WINDOWS
          key = keymap3[row][col];
          break;
        case 4:
          // Fn+q����-Android
          key = keymap4[row][col];
          break;
        case 5:
          // Fn+e����-IOS
          key = keymap4[row][col];
          break;
        default:
          key = keymap0[row][col];
          break;
        }

        // �ۼ�ͬʱ���µİ���
        keyCount++;

        // ��ֵ��������ֵ
        KeyGroup[keyCount] = key;
        // ȥ��
        delay(debounceTime); //ȥ��

        // ������Ϣ
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
    //��ʼ����
    digitalWrite(colPins[col], HIGH);
  }
}

void setup()
{
  //��ʼ������
  Serial.begin(115200);

  // ��ʼ����Сд״̬
  CapsLockState = false;

  /**��ʼ���ܽ�״̬*/
  // ��ʼ��LED
  pinMode(LED, OUTPUT);
  ledState = true;

  // �������þ�����
  for (int row = 0; row < numRows; row++)
  {
    pinMode(rowPins[row], INPUT_PULLUP); //����������Ϊ���룬����Ҫע��ģʽѡ���ڲ�����
    digitalWrite(rowPins[row], HIGH);    //����������
  }

  // �������þ�����
  for (int column = 0; column < numCols; column++)
  {
    pinMode(colPins[column], OUTPUT); //����������Ϊ���
    digitalWrite(colPins[column], HIGH); //ʹ�����в���Ծ
  }

  // ��˸LED��ʾ�������
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

  // ��ʱ1��
  delay(1000);

  // ����USBΪ����
  // HID.begin(HID_KEYBOARD);
  // ����USBΪ���̺���겢��ʼ��
  // HID.begin(HID_KEYBOARD_MOUSE);
  // ��ʼ����ý�����
  HID.begin(reportDescription, sizeof(reportDescription));
  // ��ʼ������(�������ʼ��,�ڶ����ģʽ�»ᵼ�´�Сд���Ƴ��ֳ�ͻ)
  Keyboard.begin();
}

void loop()
{

  // ����ʱ��
  Clock0++;
  if (Clock0 == 200)
  {
    Clock0 = 100;
  }

  // ��ʼɨ�����
  getKey();

  // ������Ϣ
  if (operationMode == 0)
  {
    uint8_t modifiers;

    // ��ӡ���԰���������Ϣ
    Serial.print(",keyboardMode:");
    Serial.print(keyboardMode);
    Serial.print(",Key1:");
    Serial.print(KeyGroup[1]);
    Serial.print(",Key2");
    Serial.print(KeyGroup[2]);
    Serial.print(",Key3");
    Serial.println(KeyGroup[3]);
  }

  // �л���Сд״̬(����ɨ��̫��,��һ������ʱ��������)
  if (KeyGroup[1] == 0xC1 && Clock0 != 0)
  {
    Clock0 = 0;
  }
  if (Clock0 == 1)
  {
    CapsLockState = !CapsLockState;
  }
  // �жϴ�Сд״̬
  if (CapsLockState == true)
  {
    // ����LED
    ledState = false;
    // �л�����ģʽ
    keyboardMode = 1;
  }
  else
  {
    // Ϩ��LED
    ledState = true;
    keyboardMode = 0;
  }

  // Fn��������
  if (KeyGroup[1] == 0xF0)
  {
    // �л�����ģʽ
    keyboardMode = 1;
  }

  // Fn��������
  if (KeyGroup[1] == 0xF0)
  {
    // �л�����ģʽ(Fn����)
    keyboardMode = 2;
  }

  // Fn + q��������
  if (KeyGroup[1] == 0xF0 && KeyGroup[2] == 0XF1)
  {
    // �л�����ģʽ(Fn+q����-Android)
    keyboardMode = 3;
  }
  // Fn + w��������
  if (KeyGroup[1] == 0xF0 && KeyGroup[2] == 0XF2)
  {
    // �л�����ģʽ(Fn+w����-WINDOWS)
    keyboardMode = 4;
  }

  // Fn + w + F11��������
  if (keyboardMode == 4 && KeyGroup[3] == 0XF0)
  {
    // ������
    Consumer.press(HIDConsumer::VOLUME_UP);
  }else{
    Consumer.release();
  }

  // Fn + w + F10��������
  if (keyboardMode == 4 && KeyGroup[3] == 0XF1)
  {
    // ������
    Consumer.press(HIDConsumer::VOLUME_DOWN);
  }else{
    Consumer.release();
  }

  // Fn + w + F12��������
  if (keyboardMode == 4 && KeyGroup[3] == 0XF2)
  {
    // ����
    Consumer.press(HIDConsumer::MUTE);
  }else{
    Consumer.release();
  }

  // Fn + e ��������
  if (KeyGroup[1] == 0xF0 && KeyGroup[2] == 0XF3)
  {
    // �л�����ģʽ(Fn+e����-IOS)
    keyboardMode = 5;
  }

  // ���ݶ����������
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

  // ���ж�����
  if (KeyGroup[2])
  {
    // ��ʱ����ڶ���������,�����ڵڶ������ɿ�ʱ,��Ҫ������,Ҫ����������
    releaseKeyGroup[2] = KeyGroup[2];
    Keyboard.press(KeyGroup[2]);
  }
  else
  {
    Keyboard.release(releaseKeyGroup[2]);
    releaseKeyGroup[2] = 0;
  }

  // ����������
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

  // �����İ���
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

  // �ͷ����а���
  if (!KeyGroup[1] && !KeyGroup[2] && !KeyGroup[3])
  {
    // ��հ�����ʱ����
    for (int i = 0; i < 3; i++)
    {
      releaseKeyGroup[i] = 0;
    }
    // �ͷ����а���
    Keyboard.releaseAll();
    // �ͷŶ�ý�尴��
    Consumer.release();
  }

  // ˢ��LED״̬
  digitalWrite(LED, ledState);

}