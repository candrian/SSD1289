#include "SSD1289.h"

#define LCD_REG      (*((volatile unsigned short *) 0x60000000))
#define LCD_RAM      (*((volatile unsigned short *) 0x60020000))

#define MAX_POLY_CORNERS   200
#define POLY_Y(Z)          ((int32_t)((Points + Z)->X))
#define POLY_X(Z)          ((int32_t)((Points + Z)->Y))
#define ABS(X)  ((X) > 0 ? (X) : -(X))

/* Global variables to set the written text color */
__IO uint16_t TextColor = 0x0000;
__IO uint16_t BackColor = 0xFFFF;
__IO uint16_t asciisize = 16;
uint16_t TimerPeriod    = 0;
uint16_t Channel3Pulse  = 0;

TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
TIM_OCInitTypeDef  TIM_OCInitStructure;
//****************************************************************************//

void LCD_CtrlLinesConfig(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOG |
						   RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOF,
						   ENABLE);
    RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, ENABLE);
    
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FSMC);		// D2
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FSMC);		// D3
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource4, GPIO_AF_FSMC);		// NOE -> RD
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_FSMC);		// NWE -> WR
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource7, GPIO_AF_FSMC);		// NE1 -> CS
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FSMC);		// D13
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FSMC);		// D14
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FSMC);		// D15
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource11, GPIO_AF_FSMC);		// A16 -> RS
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FSMC);		// D0
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FSMC);		// D1
    
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource7, GPIO_AF_FSMC);		// D4
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource8, GPIO_AF_FSMC);		// D5
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_FSMC);		// D6
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource10, GPIO_AF_FSMC);		// D7
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource11, GPIO_AF_FSMC);		// D8
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource12, GPIO_AF_FSMC);		// D9
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource13, GPIO_AF_FSMC);		// D10
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_FSMC);		// D11
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource15, GPIO_AF_FSMC);		// D12
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 |
    GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 |
    GPIO_Pin_11 | GPIO_Pin_14 | GPIO_Pin_15;
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 |
    GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 |
    GPIO_Pin_15;
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    
}

void LCD_FSMCConfig(void) {
    
    FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
    FSMC_NORSRAMTimingInitTypeDef FSMC_NORSRAMTimingInitStructure;
    FSMC_NORSRAMTimingInitStructure.FSMC_AddressSetupTime = 0;  //0
    FSMC_NORSRAMTimingInitStructure.FSMC_AddressHoldTime = 0;   //0
    FSMC_NORSRAMTimingInitStructure.FSMC_DataSetupTime = 2;     //3
    FSMC_NORSRAMTimingInitStructure.FSMC_BusTurnAroundDuration = 0;
    FSMC_NORSRAMTimingInitStructure.FSMC_CLKDivision = 1;//1
    FSMC_NORSRAMTimingInitStructure.FSMC_DataLatency = 0;
    FSMC_NORSRAMTimingInitStructure.FSMC_AccessMode = FSMC_AccessMode_A;
    
    FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM1;
    FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
    FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;
    FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
    FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
    FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
    FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
    FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Enable;//disable
    FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &FSMC_NORSRAMTimingInitStructure;
    
    FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);
    FSMC_NORSRAMTimingInitStructure.FSMC_AddressSetupTime = 0;    //0
    FSMC_NORSRAMTimingInitStructure.FSMC_AddressHoldTime = 0;	//0
    FSMC_NORSRAMTimingInitStructure.FSMC_DataSetupTime = 4;	//3
    FSMC_NORSRAMTimingInitStructure.FSMC_BusTurnAroundDuration = 0;
    FSMC_NORSRAMTimingInitStructure.FSMC_CLKDivision = 1;//1
    FSMC_NORSRAMTimingInitStructure.FSMC_DataLatency = 0;
    FSMC_NORSRAMTimingInitStructure.FSMC_AccessMode = FSMC_AccessMode_A;
    FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &FSMC_NORSRAMTimingInitStructure;
    FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);
    
    FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);
}

void Init_SysTick(void) {
    
    RCC_ClocksTypeDef RCC_Clocks;
    
    RCC_GetClocksFreq(&RCC_Clocks);
    SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);
    
}

void TIM_Config(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA  , ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_TIM1);
    TimerPeriod = (SystemCoreClock / 17570 ) - 1;
    Channel3Pulse = (uint16_t) (((uint32_t) 99 * (TimerPeriod - 1)) / 100);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 , ENABLE);
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = TimerPeriod;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
    TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;
    TIM_OCInitStructure.TIM_Pulse = Channel3Pulse;
    TIM_OC3Init(TIM1, &TIM_OCInitStructure);
    TIM_Cmd(TIM1, ENABLE);
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
    
}

/*
 * Reset and Initialize Display.
*/
void Init_LCD(void) {
    
    LCD_CtrlLinesConfig();
    Delay(3000);
    LCD_FSMCConfig();
    Delay(3000);
    TIM_Config();
    LCD_BackLight(100);
    
    LCD_WriteReg(0x0007,0x0021);    Delay(50);
    LCD_WriteReg(0x0000,0x0001);    Delay(50);
    LCD_WriteReg(0x0007,0x0023);    Delay(50);
    LCD_WriteReg(0x0010,0x0000);    Delay(90);
    LCD_WriteReg(0x0007,0x0033);    Delay(50);
    LCD_WriteReg(0x0011,0x6830);    Delay(50);
    LCD_WriteReg(0x0002,0x0600);    Delay(50);
    LCD_WriteReg(0x0012,0x6CEB);    Delay(50);
    LCD_WriteReg(0x0003,0xA8A4);    Delay(50);
    LCD_WriteReg(0x000C,0x0000);    Delay(50);
    LCD_WriteReg(0x000D,0x080C);    Delay(50);
    LCD_WriteReg(0x000E,0x2B00);    Delay(50);
    LCD_WriteReg(0x001E,0x00B0);    Delay(50);
    LCD_WriteReg(0x0001,0x2b3F);    Delay(50);  //RGB
    LCD_WriteReg(0x0005,0x0000);    Delay(50);
    LCD_WriteReg(0x0006,0x0000);    Delay(50);
    LCD_WriteReg(0x0016,0xEF1C);    Delay(50);
    LCD_WriteReg(0x0017,0x0103);    Delay(50);
    LCD_WriteReg(0x000B,0x0000);    Delay(50);
    LCD_WriteReg(0x000F,0x0000);    Delay(50);
    LCD_WriteReg(0x0041,0x0000);    Delay(50);
    LCD_WriteReg(0x0042,0x0000);    Delay(50);
    LCD_WriteReg(0x0048,0x0000);    Delay(50);
    LCD_WriteReg(0x0049,0x013F);    Delay(50);
    LCD_WriteReg(0x004A,0x0000);    Delay(50);
    LCD_WriteReg(0x004B,0x0000);    Delay(50);
    LCD_WriteReg(0x0044,0xEF00);    Delay(50);
    LCD_WriteReg(0x0045,0x0000);    Delay(50);
    LCD_WriteReg(0x0046,0x013F);    Delay(50);
    LCD_WriteReg(0x0030,0x0707);    Delay(50);
    LCD_WriteReg(0x0031,0x0204);    Delay(50);
    LCD_WriteReg(0x0032,0x0204);    Delay(50);
    LCD_WriteReg(0x0033,0x0502);    Delay(50);
    LCD_WriteReg(0x0034,0x0507);    Delay(50);
    LCD_WriteReg(0x0035,0x0204);    Delay(50);
    LCD_WriteReg(0x0036,0x0204);    Delay(50);
    LCD_WriteReg(0x0037,0x0502);    Delay(50);
    LCD_WriteReg(0x003A,0x0302);    Delay(50);
    LCD_WriteReg(0x002F,0x12BE);    Delay(50);
    LCD_WriteReg(0x003B,0x0302);    Delay(50);
    LCD_WriteReg(0x0023,0x0000);    Delay(50);
    LCD_WriteReg(0x0024,0x0000);    Delay(50);
    LCD_WriteReg(0x0025,0x8000);    Delay(50);
    LCD_WriteReg(0x004f,0x0000);    Delay(50);
    LCD_WriteReg(0x004e,0x0000);    Delay(50);
    
}

void LCD_WriteRAM_Prepare(void) {
    LCD_REG = LCD_REG_34;
}

void LCD_WriteRAM(uint16_t RGB_Code) {
    LCD_RAM = RGB_Code;
}

void LCD_WriteReg(uint8_t LCD_Reg, uint16_t LCD_RegValue) {
    LCD_REG = LCD_Reg;
    LCD_RAM = LCD_RegValue;
    
}

void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos) {
    LCD_WriteReg(LCD_REG_78, Xpos);
    LCD_WriteReg(LCD_REG_79, Ypos);
    
}

void LCD_Clear(uint16_t color) {
    uint32_t index = 0;
    LCD_SetCursor(0x00, 0x00);
    LCD_WriteRAM_Prepare();
    for(index = 0; index <76800; index++){
        LCD_RAM = color;
    }
}

void LCD_BackLight(int procentai) {
    if (procentai>100)
    {procentai=100;}
    else if(procentai<0)
    {procentai=0;}
    Channel3Pulse =(uint16_t)(((uint32_t)procentai*(TimerPeriod-1))/100);
    TIM_TimeBaseStructure.TIM_Period = TimerPeriod;
    TIM_OCInitStructure.TIM_Pulse = Channel3Pulse;
    TIM_OC3Init(TIM1, &TIM_OCInitStructure);
}

