#ifndef __GUI_TASK_H
#define __GUI_TASK_H

#include <stdbool.h>
#include "main.h"
#include "mc_interface.h"
#include "mc_config.h"

#define HMI_TICKS	20
#define POS_CNT		2

typedef enum
{
	HMI_IDLE	= 0,
  KEYPAD_EVENT = 1,  
	ENCOER_EVENT = 2,
  GUI_REFRESH = 3
} HMI_State_t;


typedef struct
{
  HMI_State_t   bState;          /*!< Variable containing state machine current state */
} HMI_Handle_t;

typedef struct
{
	uint16_t current_speed;
	uint16_t current_power;
	uint16_t current_torque;
	uint8_t  curennt_satus;
	uint8_t  temp;
//	uint8_t  spindle_load;

}HMI_Data_t;


void HMI_Task(void);
void HMI_Scheduler(void);
void LCD_RefreshData(void);

void Delay_US(uint32_t nus);
void Delay_MS(uint16_t nms);

void HMI_Init( HMI_Handle_t * pHandle );
bool HMI_NextState( HMI_Handle_t * pHandle, HMI_State_t bState );
bool HMI_Data(HMI_Data_t *mdata,MCI_Handle_t * mHandle,MotorPowMeas_Handle_t * pHandle);

#endif
