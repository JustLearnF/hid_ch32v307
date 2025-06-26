#pragma once
#include "debug.h"
#include "tusb.h"
#include "usb_descriptors.h"

extern volatile uint32_t counter;
extern volatile bool is_parse_need;

enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

#define TXBS 8
#define RXBS 16
extern uint8_t Txbuf[TXBS];
extern uint8_t Rxbuf[RXBS];

struct k_report
{
  uint8_t modifier;
  uint8_t keycode[6];
};

struct m_report
{
  uint8_t buttons;
  int8_t x;
  int8_t y;
  int8_t vertical;
  int8_t horizon;
};


struct input_state
{
  uint8_t device;   // 选择的设备 0:键盘 1:鼠标 2:BOTH
  uint8_t kmode;   // 键盘选择的模式 0:单次 1:持久 2:循环
  uint8_t mmode;   // 鼠标选择的模式 0:单次 1:持久 2:循环
  struct k_report kreport;   // 键盘hid报文
  struct m_report mreport;   // 鼠标hid报文
  uint8_t delay;   // 单次模式为延时,循环模式为间隔，单位为10ms
};

extern struct input_state current_state;

void mcu_initialize(void);
void usb_task(void);