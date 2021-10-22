#include "hmi_task.h"
#include "mc_interface.h"
#include "mc_config.h"
#include "uc1705.h"


uint16_t hmi_task_cnt=0;

uint8_t enc_pos=0;

extern int8_t enc_value;

int16_t motor_speed=83;


MCI_Handle_t * uMCI = &Mci[M1];
 
HMI_Handle_t hmi_handle;

/*define encoder value*/
uint16_t pos_value[POS_CNT];

/*define task time interval*/
void HMI_Scheduler(void)
{
	if(hmi_task_cnt > 0u)
	{
		hmi_task_cnt--;
	}
	else
	{
		HMI_Task();
		hmi_task_cnt = HMI_TICKS;
	}
}
	
HMI_State_t HMI_GetState( HMI_Handle_t * pHandle )
{
  return ( pHandle->bState );
}	

void HMI_Task(void)
{
	HMI_State_t hmi_state;
	hmi_state = HMI_GetState(&hmi_handle);
	switch(hmi_state)
	{
		case HMI_IDLE:/*check data refresh*/
			
			
			HMI_NextState(&hmi_handle,GUI_REFRESH);
			break;
		case KEYPAD_EVENT:/*process keypad event*/
			HAL_Delay(250);
			if(LL_GPIO_IsInputPinSet(GPIOB,ENC_BTN)==0)
			{
				if(MCI_GetSTMState(uMCI) == RUN)MCI_StopMotor(uMCI);
				else if(MCI_GetSTMState(uMCI) == IDLE)MCI_StartMotor(uMCI);			
			}
			else 
			{
				enc_pos++;
				if(enc_pos>POS_CNT)enc_pos=0;
			}
			HMI_NextState(&hmi_handle,GUI_REFRESH);
			break;
		case ENCOER_EVENT:/*process encoder event*/
			pos_value[enc_pos] += enc_value;
			HMI_NextState(&hmi_handle,GUI_REFRESH);
			break;
		case GUI_REFRESH:/*lcd data refresh*/
			LCD_RefreshGram();
			HMI_NextState(&hmi_handle,HMI_IDLE);
			break;
		default:
			break;
	}

}


void HMI_Init( HMI_Handle_t * pHandle )
{
	LCD_Init();
  pHandle->bState = HMI_IDLE;
}


/*speed power torque data*/
void HMI_Data(HMI_Data_t *mdata,MCI_Handle_t * mHandle,MotorPowMeas_Handle_t * pHandle)
{
	mdata->current_speed = MCI_GetLastRampFinalSpeed(mHandle);
//	mdata->current_torque = MCI_GetLastRampFinalTorque(mHandle);
	mdata->current_power = MPM_GetAvrgElMotorPowerW(pHandle);

}



bool HMI_NextState( HMI_Handle_t * pHandle, HMI_State_t bState )
{
	bool bChangeState = false;
  HMI_State_t bCurrentState = pHandle->bState;
  HMI_State_t bNewState = bCurrentState;	
  switch ( bCurrentState )
  {
    case HMI_IDLE:
      if ( (bState ==KEYPAD_EVENT)||(bState ==ENCOER_EVENT)||(bState ==GUI_REFRESH) )
      {
        bNewState = bState;
        bChangeState = true;
      }
      break;
    case KEYPAD_EVENT:
      if ( bState == GUI_REFRESH )
      {
        bNewState = bState;
        bChangeState = true;
      }
      break;
		case ENCOER_EVENT:
      if ( bState == GUI_REFRESH )
      {
        bNewState = bState;
        bChangeState = true;
      }
      break;
		case GUI_REFRESH:
      if ( bState == HMI_IDLE )
      {
        bNewState = bState;
        bChangeState = true;
      }
      break;
		default:
			break;
		
		}
  if ( bChangeState )
  {
    pHandle->bState = bNewState;
  }
	 return ( bChangeState );
}


