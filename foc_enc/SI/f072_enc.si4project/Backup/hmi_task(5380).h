#ifndef __GUI_TASK_H
#define __GUI_TASK_H

#include <stdbool.h>
#include "main.h"

#define HMI_TICKS	100
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



void HMI_Task(void);
void HMI_Scheduler(void);
void HMI_Init( HMI_Handle_t * pHandle );
bool HMI_NextState( HMI_Handle_t * pHandle, HMI_State_t bState );

#endif
