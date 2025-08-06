/**
  Generated Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This is the main file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  Description:
    This header file provides implementations for driver APIs for all modules
  selected in the GUI. Generation Information : Product Revision  :  PIC10 /
  PIC12 / PIC16 / PIC18 MCUs - 1.81.8 Device            :  PIC16F1936 Driver
  Version    :  2.00
 */

/*
    (c) 2018 Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms, you may use Microchip software
   and any derivatives exclusively with Microchip products. It is your
   responsibility to comply with third party license terms applicable to your
   use of third party software (including open source software) that may
   accompany Microchip software.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS
    FOR A PARTICULAR PURPOSE.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS
    SOFTWARE.
 */

// 202500805 TY1475_RX_LOW V01 CS:3DEF
// 參考 KD件動作表.xlsx
// 主要特點:
// 1.遠燈(olny),電磁閥測試
// 2.遠燈+晝行, 晝行(忽略遠燈信號)
// 3.遠燈+行車, 行車(忽略遠燈信號)
// 4.遠燈+近燈, 遠燈
// 搭配TY1014_TX V02 CS:2556使用
// 5.遠燈&近燈&晝行 同一光源 遠燈&近燈 PWM 100% 晝行 PWM 40%
// 6.電磁閥(開) 遠燈, (關) 近燈

// 20250505 TY1475_RX_LOW V02 CS:413C
// 修正行車信號 P2_ON 功能
// 修正LoBeam程式 增加程式可讀性


#include "mcc_generated_files/mcc.h"
#include "mcc_generated_files/pin_manager.h"
#include <pic.h>
#include <stdint.h> //uint8_t
#include <stdio.h>  //printf
typedef union {
  struct {
    _Bool b0 : 1;
    _Bool b1 : 1;
    _Bool RunLight : 1;
    _Bool DRL : 1;
    _Bool LoBeam : 1;
    _Bool HiBeam : 1;
    _Bool b6 : 1;
    _Bool b7 : 1;
  };
  uint8_t Byte;
} flag_D2;
typedef union {
  struct {
    _Bool b0 : 1;
    _Bool b1 : 1;
    _Bool b2 : 1;
    _Bool b3 : 1;
    _Bool b4 : 1;
    _Bool turnLight_L : 1;
    _Bool turnLight_R : 1;
    _Bool b7 : 1;
  };
  uint8_t Byte;
} flag_D3;
typedef union {
  struct {
    _Bool turnLight_R : 1;
    _Bool turnLight_L : 1;
    _Bool HiBeam : 1;
    _Bool LoBeam : 1;
    _Bool DRL : 1;
    _Bool RunLight : 1;
    _Bool b6 : 1;
    _Bool b7 : 1;
  };
  uint8_t Byte;
} flag_CS_index;

uint8_t rx_data = 0;
volatile bool data_1byte_OK = 0;
bool fFrame_OK = 0;
uint8_t data_buf[11];
uint8_t step = 0;

flag_D2 D2;
flag_D3 D3;
flag_CS_index csFlag;

volatile uint8_t T10MS_CNT = 0; // 單位:10ms
// bool fPWM1_ON = 0;
bool fException = 0;
bool fException2 = 0;

#define LOGIC_LEVEL (16)
#define HiBeam_ON() HB_EN_SetHigh()
#define HiBeam_OFF() HB_EN_SetLow()
#define LoBeam_ON() EPWM2_LoadDutyValue(PWM2_DUTY_100_PERCENT); // PWM 100%
#define LoBeam_OFF() EPWM2_LoadDutyValue(PWM2_DUTY_0_PERCENT);  // PWM 0%
#define DRL_ON() EPWM2_LoadDutyValue(PWM2_DUTY_40_PERCENT);     // PWM 40%
#define P2_ON() P2_EN_SetHigh()
#define P2_OFF() P2_EN_SetLow()

#define PWM2_DUTY_40_PERCENT (199)
#define PWM2_DUTY_100_PERCENT (499)
#define PWM2_DUTY_0_PERCENT (0)

static uint8_t CS[64] = {
    // 行車,晝行,近燈,遠燈,左方,右方
    0x4C, // 000000 關
    0x9F, // 000001 右方
    0xED, // 000010 左方
    0x62, // 000011 右方+左方(警示)

    0x9A, // 000100 遠燈
    0x00, // 000101 遠燈+右方
    0x00, // 000110 遠燈+左方
    0x00, // 000111 遠燈+右方+左方(警示)

    0x27, // 001000 近燈
    0x00, // 001001 近燈+右方
    0x00, // 001010 近燈+左方
    0x00, // 001011 近燈+右方+左方(警示)

    0x73, // 001100 遠燈+近燈(超車)
    0xA0, // 001101 遠燈+近燈(超車)+右方
    0x71, // 001110 遠燈+近燈(超車)+左方
    0x01, // 001111 遠燈+近燈(超車)+右方+左方(警示)

    0x8E, // 010000 晝行
    0x5D, // 010001 晝行+右方
    0x2F, // 010010 晝行+左方
    0xFC, // 010011 晝行+右方+左方(警示)

    0x67, // 010100 晝行+遠燈
    0x00, // 010101 晝行+遠燈+右方
    0x00, // 010110 晝行+遠燈+左方
    0x00, // 010111 晝行+遠燈+右方+左方(警示)

    0xDA, // 011000 晝行+近燈
    0x00, // 011001 晝行+近燈+右方
    0x00, // 011010 晝行+近燈+左方
    0x00, // 011011 晝行+近燈+右方+左方(警示)

    // 0x12, // 011100 畫行+遠燈+近燈(超車)
    0xB1, // 011100 畫行+遠燈+近燈(超車)
    0xA0, // 011101 畫行+遠燈+近燈(超車)+右方
    0x71, // 011110 畫行+遠燈+近燈(超車)+左方
    0x01, // 011111 晝行+遠燈+近燈(超車)+右方+左方(警示)

    0x2D, // 100000 行車
    0xFE, // 100001 行車+右方
    0x8C, // 100010 行車+左方
    0x5F, // 100011 行車+右方+左方(警示)

    0x2C, // 100100 行車+遠燈
    0x00, // 100101 行車+遠燈+右方
    0x00, // 100110 行車+遠燈+左方
    0x00, // 100111 行車+遠燈+右方+左方(警示)

    0x38, // 101000 行車+近燈
    0xEB, // 101001 行車+近燈+右方
    0x99, // 101010 行車+近燈+左方
    0x4A, // 101011 行車+近燈+右方+左方(警示)

    0x12, // 101100 行車+遠燈+近燈(超車)
    0xC1, // 101101 行車+遠燈+近燈(超車)+右方
    0xB3, // 101110 行車+遠燈+近燈(超車)+左方
    0x60, // 101111 行車+遠燈+近燈(超車)+右方+左方(警示)

    0xEF, // 110000 行車+晝行
    0x3C, // 110001 行車+晝行+右方
    0x4E, // 110010 行車+晝行+左方
    0x9D, // 110011 行車+晝行+右方+左方(警示)

    0xD1, // 110100 行車+晝行+遠燈
    0x00, // 110101 行車+晝行+遠燈+右方
    0x00, // 110110 行車+晝行+遠燈+左方
    0x00, // 110111 行車+晝行+遠燈+右方+左方(警示)

    0x6C, // 111000 行車+晝行+近燈
    0x00, // 111001 行車+晝行+近燈+右方
    0x00, // 111010 行車+晝行+近燈+左方
    0x00, // 111011 行車+晝行+近燈+右方+左方(警示)

    0xD0, // 111100 行車+晝行+遠燈+近燈
    0x03, // 111101 行車+晝行+遠燈+近燈+右方
    0x71, // 111110 行車+晝行+遠燈+近燈+左方
    0xA2, // 111111 行車+晝行+遠燈+近燈(超車)+右方+左方(警示)
};

// 特殊信號(unlock) 亮P2
void Exception_handling(void) {
  if ((data_buf[2] == 6) && (data_buf[10] == 0xD5)) {
    P2_EN_SetHigh();
    fException = 1;
    EPWM2_LoadDutyValue(PWM2_DUTY_0_PERCENT); // 0%
    HiBeam_OFF();                             // 遠燈 OFF
    T10MS_CNT = 0;                            // 重置2000ms計數器
  } else {
    fException = 0;
  }
}

void LED_output(void) {
  uint8_t crc8 = CS[csFlag.Byte];
  // CRC檢查
  if (data_buf[10] != crc8)
    return;

  //  if (data_buf[0] != 0xC8)
  //    return; // CRC檢查
  //  if (data_buf[1] != 0x8B)
  //    return; // CRC檢查

  T10MS_CNT = 0; // 重置2000ms計數器

  // 行車
  if (csFlag.RunLight == 1) {
    P2_ON();
  } else {
    P2_OFF();
  }

  if (csFlag.RunLight == 0 && csFlag.LoBeam == 0 && csFlag.HiBeam == 0 &&
      csFlag.DRL == 0) { // 0
    // 行車OFF + 晝行OFF + 近燈OFF + 遠燈OFF = 電磁閥測試(關)(KD件) (LED OFF)
    HiBeam_OFF(); // 遠燈 OFF
    LoBeam_OFF(); // 近燈 OFF
  } else if (csFlag.RunLight == 0 && csFlag.LoBeam == 0 && csFlag.HiBeam == 0 &&
             csFlag.DRL == 1) { // 1
    // DRL = DRL
    HiBeam_OFF(); // 遠燈 OFF
    DRL_ON();     // 晝行 ON
  } else if (csFlag.RunLight == 0 && csFlag.LoBeam == 0 && csFlag.HiBeam == 1 &&
             csFlag.DRL == 0) { // 2
    // 行車OFF + HiBeam = 電磁閥測試(開)(KD件) (LED OFF)
    HiBeam_ON();  // 遠燈 ON
    LoBeam_OFF(); // 近燈 OFF
  } else if (csFlag.RunLight == 0 && csFlag.LoBeam == 0 && csFlag.HiBeam == 1 &&
             csFlag.DRL == 1) { // 3
    // HiBeam + DRL = DRL(此時Hibeam信號無效-KD件)
    HiBeam_OFF(); // 遠燈 OFF
    DRL_ON();     // 晝行 ON
  } else if (csFlag.RunLight == 0 && csFlag.LoBeam == 1 && csFlag.HiBeam == 0 &&
             csFlag.DRL == 0) { // 4
    // LOBEAM = LOBEAM
    HiBeam_OFF(); // 遠燈 OFF
    LoBeam_ON();  // 近燈 ON
  } else if (csFlag.RunLight == 0 && csFlag.LoBeam == 1 && csFlag.HiBeam == 0 &&
             csFlag.DRL == 1) { // 5
    // LOBEAM + DRL = LOBEAM
    HiBeam_OFF(); // 遠燈 OFF
    LoBeam_ON();  // 近燈 ON
  } else if (csFlag.RunLight == 0 && csFlag.LoBeam == 1 && csFlag.HiBeam == 1 &&
             csFlag.DRL == 0) { // 6
    // LOBEAM + HIBEAM = HIBEAM + LOBEAM
    HiBeam_ON(); // 遠燈 ON
    LoBeam_ON(); // 近燈 ON
  } else if (csFlag.RunLight == 0 && csFlag.LoBeam == 1 && csFlag.HiBeam == 1 &&
             csFlag.DRL == 1) { // 7
    // LOBEAM + HIBEAM + DRL = HIBEAM
    HiBeam_ON(); // 晝行+超車
    LoBeam_ON(); // 近燈 ON
  } else if (csFlag.RunLight == 1 && csFlag.LoBeam == 0 && csFlag.HiBeam == 0 &&
             csFlag.DRL == 0) { // 8
    // 行車 電磁閥(關)
    HiBeam_OFF(); // 遠燈 OFF
    LoBeam_OFF()
  } else if (csFlag.RunLight == 1 && csFlag.LoBeam == 0 && csFlag.HiBeam == 0 &&
             csFlag.DRL == 1) { // 9
    // 行車 + DRL = 行車 + DRL
    HiBeam_OFF(); // 遠燈 OFF
    DRL_ON();     // 晝行 ON
  } else if (csFlag.RunLight == 1 && csFlag.LoBeam == 0 && csFlag.HiBeam == 1 &&
             csFlag.DRL == 0) { // 10
    // 行車 + HIBEAM = 行車(此時Hibeam信號無效-KD件)
    P2_EN_SetHigh();
    HiBeam_OFF(); // 遠燈 OFF
    LoBeam_OFF(); // 近燈 OFF
  } else if (csFlag.RunLight == 1 && csFlag.LoBeam == 0 && csFlag.HiBeam == 1 &&
             csFlag.DRL == 1) { // 11
    // 行車 + HIBEAM + DRL = 行車 + DRL(此時Hibeam信號無效-KD件)
    HiBeam_OFF(); // 遠燈 OFF
    DRL_ON();     // 晝行 ON
  } else if (csFlag.RunLight == 1 && csFlag.LoBeam == 1 && csFlag.HiBeam == 0 &&
             csFlag.DRL == 0) { // 12
    // 行車 + LOBEAM = 行車 + LOBEAM
    HiBeam_OFF(); // 遠燈 OFF
    LoBeam_ON();  // 近燈 ON
  } else if (csFlag.RunLight == 1 && csFlag.LoBeam == 1 && csFlag.HiBeam == 0 &&
             csFlag.DRL == 1) { // 13
    // 行車 + LOBEAM + DRL = 行車 + LOBEAM
    HiBeam_OFF(); // 遠燈 OFF
    LoBeam_ON();  // 近燈 ON
  } else if (csFlag.RunLight == 1 && csFlag.LoBeam == 1 && csFlag.HiBeam == 1 &&
             csFlag.DRL == 0) { // 14
    // 行車 + HIBEAM + LOBEAM = 行車 + HIBEAM + LOBEAM
    HiBeam_ON(); // 遠燈 ON
    LoBeam_ON(); // 近燈 ON
  } else if (csFlag.RunLight == 1 && csFlag.LoBeam == 1 && csFlag.HiBeam == 1 &&
             csFlag.DRL == 1) { // 15
    // 行車 + HIBEAM + LOBEAM + DRL = 行車 + HIBEAM + LOBEAM
    HiBeam_ON(); // 遠燈 ON
    LoBeam_ON(); // 近燈 ON
  }

  for (uint8_t i = 0; i < 11; i++) {
    data_buf[i] = 0; // 清除資料緩衝區
  }
  // // 左方向
  // if (csFlag.turnLight_L == 1) {
  //   // LED5_SetHigh();
  // } else {
  //   // LED5_SetLow();
  // }

  // 右方向
  // if (csFlag.turnLight_R == 1) {
  //   // LED6_SetHigh();
  // } else {
  //   // LED6_SetLow();
  // }
}

void check_input(void) {
  D2.Byte = data_buf[2];
  D3.Byte = data_buf[3];
  csFlag.Byte = 0;

  // 行車
  if (D2.RunLight == 1)
    csFlag.RunLight = 1;
  else
    csFlag.RunLight = 0;

  // 晝行
  if (D2.DRL == 1)
    csFlag.DRL = 1;
  else
    csFlag.DRL = 0;

  // 近燈
  if (D2.LoBeam == 1)
    csFlag.LoBeam = 1;
  else
    csFlag.LoBeam = 0;

  // 遠燈
  if (D2.HiBeam == 1)
    csFlag.HiBeam = 1;
  else
    csFlag.HiBeam = 0;

  // 左方向
  if (D3.turnLight_L == 1)
    csFlag.turnLight_L = 1;
  else
    csFlag.turnLight_L = 0;

  // 右方向
  if (D3.turnLight_R == 1)
    csFlag.turnLight_R = 1;
  else
    csFlag.turnLight_R = 0;
}

void getFrameData(void) {
  if (data_1byte_OK == 1) {
    data_1byte_OK = 0;
    // printf("%0
    // printf("\n
    switch (step) {
    case 0:
      if (rx_data == 0xC8) {
        step = 1;
        data_buf[0] = rx_data;
      } else {
        step = 0;
      }
      break;
    case 1:
      if (rx_data == 0x8B) {
        data_buf[1] = rx_data;
        step = 2;
      } else {
        step = 0;
      }
      break;
    case 2:
      data_buf[2] = rx_data;
      step = 3;
      break;
    case 3:
      data_buf[3] = rx_data;
      step = 4;
      break;
    case 4:
      data_buf[4] = rx_data;
      step = 5;
      break;
    case 5:
      data_buf[5] = rx_data;
      step = 6;
      break;
    case 6:
      data_buf[6] = rx_data;
      step = 7;
      break;
    case 7:
      data_buf[7] = rx_data;
      step = 8;
      break;
    case 8:
      data_buf[8] = rx_data;
      step = 9;
      break;
    case 9:
      data_buf[9] = rx_data;
      step = 10;
      break;
    case 10:
      data_buf[10] = rx_data;
      fFrame_OK = 1;
      step = 0;
      break;
    default:
      step = 0;
      break;
    }
  }
}

void ECCP3_CallBack(uint16_t capturedValue) {
  static uint16_t pluse_width_LO = 0;
  static uint16_t pluse_width_HI = 0;
  static uint8_t HI_us = 0;
  static uint8_t LO_us = 0;
  static uint8_t diff_us = 0;
  static uint16_t falling_edge_time = 0;
  static uint16_t rising_edge_time = 0;
  static bool rise_edge_flag = 0;
  static bool fHead = 0;
  static uint8_t data_cnt = 0;

  // falling edge tringgered
  if (CCP3CON == 0x04) {
    falling_edge_time = capturedValue;
    CCP3CON = 0x05; // set to rising edge trigger
    if (rise_edge_flag == 1) {
      rise_edge_flag = 0;

      if (falling_edge_time >= rising_edge_time) {
        pluse_width_HI = falling_edge_time - rising_edge_time;
      } else {
        pluse_width_HI = (0xffff - rising_edge_time) + falling_edge_time;
      }
      LO_us = (pluse_width_LO >> 3) & 0xff;
      HI_us = (pluse_width_HI >> 3) & 0xff;
      if (HI_us > LO_us)
        diff_us = HI_us - LO_us;
      else
        diff_us = LO_us - HI_us;

      if (fHead == 0) {
        UART_TX_SetHigh();
        // init
        if (diff_us < LOGIC_LEVEL) {
          // logical '0'
          fHead = 1;
          data_cnt = 0;
          UART_TX_SetLow();
        }
      } else {
        // data
        if (diff_us < LOGIC_LEVEL) {
          // logical '0'
          UART_TX_SetLow();
          rx_data &= ~(1 << data_cnt);
        } else {
          // logical '1'
          UART_TX_SetHigh();
          rx_data |= (1 << data_cnt);
        }

        data_cnt++;
        if (data_cnt > 7) {
          fHead = 0;
          data_1byte_OK = 1;
        }
      }
    }

  } // rising edge tringgered
  else if (CCP3CON == 0x05) {
    rising_edge_time = capturedValue;
    CCP3CON = 0x04; // set to falling edge trigger
    // 計算低電位脈衝寬度
    if (rising_edge_time >= falling_edge_time) {
      pluse_width_LO = rising_edge_time - falling_edge_time;
    } else {
      pluse_width_LO = (0xffff - falling_edge_time) + rising_edge_time;
    }
    rise_edge_flag = 1;
  }
}

void TMR0_EvenHandler(void) {
  T10MS_CNT++;
  if (T10MS_CNT >= 200) { // 2000ms
    fException2 = 1;      // 設定異常狀態
    T10MS_CNT = 0;
  }
}

/*
                         Main application
 */
void main(void) {
  // initialize the device
  SYSTEM_Initialize();
  TMR0_SetInterruptHandler(TMR0_EvenHandler);

  // When using interrupts, you need to set the Global and Peripheral
  // Interrupt Enable bits Use the following macros to:

  // Enable the Global Interrupts
  INTERRUPT_GlobalInterruptEnable();

  // Enable the Peripheral Interrupts
  INTERRUPT_PeripheralInterruptEnable();

  // Disable the Global Interrupts
  // INTERRUPT_GlobalInterruptDisable();

  // Disable the Peripheral Interrupts
  // INTERRUPT_PeripheralInterruptDisable();
  EPWM2_LoadDutyValue(0);
  HiBeam_OFF(); // 遠燈 OFF

  while (1) {
    getFrameData();
    check_input();
    // LED_output();
    Exception_handling();
    if (fException2 == 0) {
      if (fException == 1) {
        // 異常處理
        fException = 0; // 重置異常狀態
      } else
        LED_output();
    } else {
      // 2秒沒收到正確的資料開啟近燈+P2
      fException2 = 0;
      // fPWM1_ON = 0;
      EPWM2_LoadDutyValue(PWM2_DUTY_100_PERCENT); // PWM 100%
      HiBeam_OFF();                               // 遠燈 OFF
      P2_EN_SetHigh();                            // P2亮
    }
  }
}

/**
 End of File
 */