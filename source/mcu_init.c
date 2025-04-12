#include "main.h"

volatile uint32_t counter = 0;
void SysTick_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void SysTick_Config(u_int64_t ticks) {
  SysTick->SR &= ~(1 << 0);  // clear State flag
  SysTick->CMP = ticks;
  SysTick->CNT = 0;
  SysTick->CTLR = 0xF;

  NVIC_SetPriority(SysTicK_IRQn, 15);
  NVIC_EnableIRQ(SysTicK_IRQn);
}

void USBHS_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USB_Config(void) {
  RCC_USBCLK48MConfig(RCC_USBCLK48MCLKSource_USBPHY);
  RCC_USBHSPLLCLKConfig(RCC_HSBHSPLLCLKSource_HSE);
  RCC_USBHSConfig(RCC_USBPLL_Div2);
  RCC_USBHSPLLCKREFCLKConfig(RCC_USBHSPLLCKREFCLK_4M);
  RCC_USBHSPHYPLLALIVEcmd(ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_USBHS, ENABLE);
}

uint8_t Txbuf[TXBS] = {0};
uint8_t Rxbuf[RXBS] = {0};
void UART2_Config(uint32_t baudrate) {
  DMA_InitTypeDef DMA_InitStructure = {0};
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

  DMA_DeInit(DMA1_Channel7);
  DMA_InitStructure.DMA_PeripheralBaseAddr =
      (u32)(&USART2->DATAR); /* USART2->DATAR:0x40004404 */
  DMA_InitStructure.DMA_MemoryBaseAddr = (u32)Txbuf;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStructure.DMA_BufferSize = TXBS;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel7, &DMA_InitStructure);

  DMA_DeInit(DMA1_Channel6);
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART2->DATAR);
  DMA_InitStructure.DMA_MemoryBaseAddr = (u32)Rxbuf;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = RXBS;
  DMA_Init(DMA1_Channel6, &DMA_InitStructure);

  GPIO_InitTypeDef GPIO_InitStructure = {0};
  USART_InitTypeDef USART_InitStructure = {0};

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  /* USART2 TX-->A.2   RX-->A.3 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  USART_InitStructure.USART_BaudRate = baudrate;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl =
      USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  USART_Init(USART2, &USART_InitStructure);
  DMA_Cmd(DMA1_Channel7, ENABLE); /* USART2 Tx */
  DMA_Cmd(DMA1_Channel6, ENABLE); /* USART2 Rx */
  USART_Cmd(USART2, ENABLE);
  USART_DMACmd(USART2, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE);
  NVIC_EnableIRQ(DMA1_Channel6_IRQn);
  DMA_ITConfig(DMA1_Channel6, DMA_IT_TC, ENABLE);
}

void DMA1_Channel6_IRQHandler(void)
    __attribute__((interrupt("WCH-Interrupt-fast")));

void KEY_Config(void) {
  GPIO_InitTypeDef GPIO_InitStructure = {0};
  EXTI_InitTypeDef EXTI_InitStructure = {0};
  NVIC_InitTypeDef NVIC_InitStructure = {0};

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource4);
  EXTI_InitStructure.EXTI_Line = EXTI_Line4;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

void EXTI4_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void mcu_initialize(void) {
  SystemCoreClockUpdate();
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  USART_Printf_Init(115200);
  printf("Build Time : %s %s\n", __DATE__, __TIME__);
  printf("Current System CLock Frequency : %d\n", SystemCoreClock);
  Delay_Init();
  USB_Config();
  Delay_Ms(10);
  UART2_Config(115200);
  KEY_Config();
  SysTick_Config((SystemCoreClock - 1) / 100);
}

void SysTick_Handler(void) {
  if (SysTick->SR == 1) {
    SysTick->SR = 0;  // clear State flag
    counter++;
  }
}

void USBHS_IRQHandler(void) { tusb_int_handler(BOARD_TUD_RHPORT, true); }

volatile bool is_parse_need = false;
void DMA1_Channel6_IRQHandler(void) {
  is_parse_need = true;
  DMA_ClearITPendingBit(DMA1_IT_TC6);
}

void EXTI4_IRQHandler(void) {
  memset((void*)&current_state, 0, RXBS);
  current_state.device = 2;
  EXTI_ClearITPendingBit(EXTI_Line4);
}