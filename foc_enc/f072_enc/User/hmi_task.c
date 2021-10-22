#include "hmi_task.h"
#include "uc1705.h"
#include "ntc_temperature_sensor.h"
#include <stdlib.h>

uint16_t hmi_task_cnt=0;

uint8_t enc_pos=0;

extern int8_t enc_value;
extern NTC_Handle_t *pTemperatureSensor[1];
//extern MCI_Handle_t Mci[0];
MCI_Handle_t * uMCI = &Mci[M1];
 
HMI_Handle_t hmi_handle;
HMI_Data_t hmi_data;

bool spindle_dir=0;
int16_t spindle_spped=135;
uint8_t spindle_load=0;
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
			
			if(HMI_Data(&hmi_data,&Mci[M1],(MotorPowMeas_Handle_t *)pMPM[M1]))
					HMI_NextState(&hmi_handle,GUI_REFRESH);
			break;
		case KEYPAD_EVENT:/*process keypad event*/
		
			if(MCI_GetSTMState(uMCI) == RUN)
			{
				MCI_StopMotor(uMCI);
				LCD_ShowString(5,44,"IDLE",6,12);
			}
			else if(MCI_GetSTMState(uMCI) == IDLE)
			{
				MCI_StartMotor(uMCI);	
				LCD_ShowString(5,44,"RUN  ",6,12);
			}
			HMI_NextState(&hmi_handle,GUI_REFRESH);
			break;
		case ENCOER_EVENT:/*process encoder event*/

			spindle_spped += enc_value*5;
			enc_value=0;	
		  if(labs(spindle_spped)>560)spindle_spped=560;
			LCD_ShowString(110,2,"ccw",6,8);
			if(0<spindle_spped)LCD_ShowString(110,2," cw",6,8);	
			MCI_ExecSpeedRamp(uMCI,spindle_spped,0);
			HMI_NextState(&hmi_handle,GUI_REFRESH);
			break;
		case GUI_REFRESH:/*lcd data refresh*/
			LCD_RefreshData();
			LCD_RefreshGram();
			HMI_NextState(&hmi_handle,HMI_IDLE);
			break;
		default:
			break;
	}

}
void HMI_Main_page(void)
{
	LCD_ShowChar(43,0,'S',12,24,1);
	LCD_ShowChar(43,21,'P',12,24,1);
	LCD_ShowChar(110,31,'W',6,12,1);
	LCD_ShowChar(122,56,'%',6,8,1);
	LCD_ShowString(43,46,"SET:",6,8);
	LCD_Show_Logo();
	LCD_ShowString(5,44,"IDLE",6,12);
	LCD_ShowString(110,46,"RPM",6,8);
	LCD_ShowString(110,10,"RPM",6,12);
	LCD_ShowString(110,2," cw",6,8);
}

void HMI_Init( HMI_Handle_t * pHandle )
{
	LCD_Init();
	HMI_Main_page();
	LCD_RefreshData();
  pHandle->bState = HMI_IDLE;
	LCD_RefreshGram();
}


/*speed power torque data*/
bool HMI_Data(HMI_Data_t *mdata,MCI_Handle_t * mHandle,MotorPowMeas_Handle_t * pHandle)
{
	bool change=0;
	if(mdata->current_speed !=MCI_GetAvrgMecSpeedUnit(mHandle)||
			mdata->current_power != MPM_GetAvrgElMotorPowerW(pHandle)||
			mdata->temp != NTC_GetAvTemp_C(pTemperatureSensor[M1])
	)
	{
		mdata->current_speed =labs( MCI_GetAvrgMecSpeedUnit(mHandle));
		mdata->current_power = MPM_GetAvrgElMotorPowerW(pHandle);
		spindle_load=(mdata->current_power)/4;
		mdata->temp = NTC_GetAvTemp_C(pTemperatureSensor[M1]);
		change = 1;
	}
	return change;
}

void LCD_RefreshData(void)
{
	LCD_ShowNum(60,0,(hmi_data.current_speed)*6,4,12,24);/*show speed*/
	LCD_ShowNum(60,21,(hmi_data.current_power),4,12,24);/*show power*/
	LCD_ShowNum(82,46,labs(spindle_spped*6),4,6,8);/*show set speed*/
	LCD_ShowNum(103,56,spindle_load,3,6,8);/*show spindle load*/
	LCD_ShowLoad(0,60,spindle_load);/*show spindle load */
	LCD_ShowNum(11,7,hmi_data.temp,2,6,8);/*show temp*/
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



void Delay_US(uint32_t nus)
{
	uint32_t ticks;
	uint32_t told,tnow,tcnt=0;
	uint32_t reload=SysTick->LOAD;
	ticks = nus*48;
	told= SysTick->VAL;
	while(1)
	{
		tnow= SysTick->VAL;
		if(tnow!=told)
		{
			if(tnow<told)tcnt+=told-tnow;
			else tcnt += reload-tnow+told;
			told=tnow;
			if(tcnt>=ticks)break;
		}
	}
}

void Delay_MS(uint16_t nms)
{
	for(uint16_t i=0;i<nms;i++)Delay_US(1000);
}

