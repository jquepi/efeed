/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2019 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_ll_dma.h"
#include "stm32f1xx_ll_i2c.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_system.h"
#include "stm32f1xx_ll_exti.h"
#include "stm32f1xx_ll_cortex.h"
#include "stm32f1xx_ll_utils.h"
#include "stm32f1xx_ll_pwr.h"
#include "stm32f1xx_ll_tim.h"
#include "stm32f1xx_ll_usart.h"
#include "stm32f1xx.h"
#include "stm32f1xx_ll_gpio.h"

#if defined(USE_FULL_ASSERT)
#include "stm32_assert.h"
#endif /* USE_FULL_ASSERT */

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <stdint.h>
#include "stddef.h"
#include "stdlib.h"
#include "string.h"

#include "fixedptc.h" 			// 8_24 format for spindle sync delay calculation
#include "fixedptc22_10.h" 	// steps per mm, min resolution is screw step / steps per rev / 2^10 = 1/400/1024=0,0000024mm
#include "fixedptc12_20.h" 	// mm, max 2048mm work field in this case, min resolution is about 0,000001mm

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
struct state_s;
typedef void (*state_func_t)( struct state_s* );
typedef void (*callback_func_t)(struct state_s*);

typedef struct {
	fixedptu Q824; //Q8.24 fixed math format
	uint8_t submenu;
	char Text[6];
	char Unit[6];
	uint8_t level;
	char infeed_mm[6];
	char infeed_inch[6];
	uint8_t infeed_strategy;
} THREAD_INFO;

typedef struct {
	fixedptu Q824; //Q8.24 fixed math format
	fixedptu pitch;
	uint8_t total_pass;
	uint8_t pass;

	fixedptu thread_depth;
//	fixedptu thread_angle; // tan(60 / 2 ) = 0,5774 >>>> fixedtp

	fixedptu infeed_mod; // =TAN(radians(B4/2-B5))
	fixedptu init_delta;
	fixedptu deltap_mm[20];
	fixedptu deltap_inch[20];
	uint8_t submenu;
	char Text[6];
	char Unit[6];
	uint8_t level;
	char infeed_mm[6];
	char infeed_inch[6];
	uint8_t infeed_strategy;
} S_WORK_SETUP;

typedef struct G_pipeline{
	int 
		X, // x axis
		Z, // z axis
		F, // feed
		P; // dwell

	int 
		I, // arc X axis delta
		K; // arc Z axis delta
//	bool sync; // wtf?
//	uint8_t code; // wtf?
} G_pipeline_t;

typedef struct G_task{
	int32_t dx, dz;
	int32_t 	x, z, x1, z1; // delta
	uint32_t steps_to_end;
	fixedptu F; //Q824
	callback_func_t callback_ref; //callback ref to iterate line or arc
	callback_func_t init_callback_ref;

	callback_func_t precalculate_init_callback_ref;
	callback_func_t precalculate_callback_ref;

	uint8_t z_direction, x_direction;
	bool stepper; // use this variable if this code use stepper motor
// arc
//	int rr, inc_dec;
	uint32_t a,b;
//	int64_t err, aa, bb;
} G_task_t;


typedef struct state_s
{
	bool init;
//	uint32_t steps_to_end;
//	uint32_t current_pos;
//	uint32_t end_pos;
//	uint8_t ramp_step;
	uint32_t Q824set; // feed rate
	uint32_t fract_part; // Q8.24 format fract part
	
	// arc variables for precalculated in task init callback for current task:	
	int64_t arc_aa, arc_bb, arc_dx, arc_dz; // error increment
	int64_t arc_err; // error of 1.step
	
	uint32_t prescaler; // used for calculating substep delay
	uint16_t rpm; // current sindle speed in rpm
	int substep_axis;
	
	G_task_t current_task;
	bool task_lock;
	bool precalculate_end;
	
	bool G94G95; // 0 - unit per min, 1 - unit per rev
//	uint32_t substep_mask;
  state_func_t function;
//  callback_func_t callback;
//	uint32_t async_z;
//	uint8_t z_period;
	bool f_encoder;
	bool f_tacho;
	bool spindle_dir;
	_Bool sync;
	_Bool main_feed_direction;
	TIM_TypeDef *syncbase;
  // other stateful data

	//	bresenham
	int err;
} state_t;


extern uint8_t Menu_Step;																					// выборка из массива по умолчанию (1.5mm)
extern bool menu_changed;


extern state_t state_precalc;
extern state_t state_hw;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
extern const THREAD_INFO Thread_Info[];

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */


/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define min_pulse 145*5
#define LED_Pin LL_GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define MOTOR_X_DIR_Pin LL_GPIO_PIN_7
#define MOTOR_X_DIR_GPIO_Port GPIOA
#define MOTOR_X_STEP_Pin LL_GPIO_PIN_0
#define MOTOR_X_STEP_GPIO_Port GPIOB
#define MOTOR_X_ENABLE_Pin LL_GPIO_PIN_1
#define MOTOR_X_ENABLE_GPIO_Port GPIOB
#define BUTTON_1_Pin LL_GPIO_PIN_8
#define BUTTON_1_GPIO_Port GPIOA
#define BUTTON_2_Pin LL_GPIO_PIN_9
#define BUTTON_2_GPIO_Port GPIOA
#define MOTOR_Z_ENABLE_Pin LL_GPIO_PIN_15
#define MOTOR_Z_ENABLE_GPIO_Port GPIOA
#define MOTOR_Z_DIR_Pin LL_GPIO_PIN_3
#define MOTOR_Z_DIR_GPIO_Port GPIOB
#define MOTOR_Z_STEP_Pin LL_GPIO_PIN_4
#define MOTOR_Z_STEP_GPIO_Port GPIOB
#define ENC_A_Pin LL_GPIO_PIN_6
#define ENC_A_GPIO_Port GPIOB
#define ENC_B_Pin LL_GPIO_PIN_7
#define ENC_B_GPIO_Port GPIOB
#define ENC_ZERO_Pin LL_GPIO_PIN_8
#define ENC_ZERO_GPIO_Port GPIOB
#ifndef NVIC_PRIORITYGROUP_0
#define NVIC_PRIORITYGROUP_0         ((uint32_t)0x00000007) /*!< 0 bit  for pre-emption priority,
                                                                 4 bits for subpriority */
#define NVIC_PRIORITYGROUP_1         ((uint32_t)0x00000006) /*!< 1 bit  for pre-emption priority,
                                                                 3 bits for subpriority */
#define NVIC_PRIORITYGROUP_2         ((uint32_t)0x00000005) /*!< 2 bits for pre-emption priority,
                                                                 2 bits for subpriority */
#define NVIC_PRIORITYGROUP_3         ((uint32_t)0x00000004) /*!< 3 bits for pre-emption priority,
                                                                 1 bit  for subpriority */
#define NVIC_PRIORITYGROUP_4         ((uint32_t)0x00000003) /*!< 4 bits for pre-emption priority,
                                                                 0 bit  for subpriority */
#endif
/* USER CODE BEGIN Private defines */
// extract GPIO pin nuber by passing LL_GPIO_PIN_x value to it. 
// Use this value for compute bit-banding address of pin at compiling time by preprocessor
#if 	MOTOR_X_DIR_Pin == 	LL_GPIO_PIN_0	
	#define MOTOR_X_DIR_Pin_num 0
#elif	MOTOR_X_DIR_Pin == 	LL_GPIO_PIN_1
	#define MOTOR_X_DIR_Pin_num 1
#elif	MOTOR_X_DIR_Pin == 	LL_GPIO_PIN_2
	#define MOTOR_X_DIR_Pin_num 2
#elif	MOTOR_X_DIR_Pin == 	LL_GPIO_PIN_3
	#define MOTOR_X_DIR_Pin_num 3
#elif	MOTOR_X_DIR_Pin == 	LL_GPIO_PIN_4
	#define MOTOR_X_DIR_Pin_num 4
#elif	MOTOR_X_DIR_Pin == 	LL_GPIO_PIN_5
	#define MOTOR_X_DIR_Pin_num 5
#elif	MOTOR_X_DIR_Pin == 	LL_GPIO_PIN_6
	#define MOTOR_X_DIR_Pin_num 6
#elif	MOTOR_X_DIR_Pin == 	LL_GPIO_PIN_7
	#define MOTOR_X_DIR_Pin_num 7
#elif	MOTOR_X_DIR_Pin == 	LL_GPIO_PIN_8
	#define MOTOR_X_DIR_Pin_num 8
#elif	MOTOR_X_DIR_Pin == 	LL_GPIO_PIN_9
	#define MOTOR_X_DIR_Pin_num 9
#elif	MOTOR_X_DIR_Pin == 	LL_GPIO_PIN_10
	#define MOTOR_X_DIR_Pin_num 10
#elif	MOTOR_X_DIR_Pin == 	LL_GPIO_PIN_11
	#define MOTOR_X_DIR_Pin_num 11
#elif	MOTOR_X_DIR_Pin == 	LL_GPIO_PIN_12
	#define MOTOR_X_DIR_Pin_num 12
#elif	MOTOR_X_DIR_Pin == 	LL_GPIO_PIN_13
	#define MOTOR_X_DIR_Pin_num 13
#elif	MOTOR_X_DIR_Pin == 	LL_GPIO_PIN_14
	#define MOTOR_X_DIR_Pin_num 14
#elif	MOTOR_X_DIR_Pin == 	LL_GPIO_PIN_15
	#define MOTOR_X_DIR_Pin_num 15
#endif 	

#if 	MOTOR_Z_DIR_Pin == 	LL_GPIO_PIN_0	
	#define MOTOR_Z_DIR_Pin_num 0
#elif	MOTOR_Z_DIR_Pin == 	LL_GPIO_PIN_1
	#define MOTOR_Z_DIR_Pin_num 1
#elif	MOTOR_Z_DIR_Pin == 	LL_GPIO_PIN_2
	#define MOTOR_Z_DIR_Pin_num 2
#elif	MOTOR_Z_DIR_Pin == 	LL_GPIO_PIN_3
	#define MOTOR_Z_DIR_Pin_num 3
#elif	MOTOR_Z_DIR_Pin == 	LL_GPIO_PIN_4
	#define MOTOR_Z_DIR_Pin_num 4
#elif	MOTOR_Z_DIR_Pin == 	LL_GPIO_PIN_5
	#define MOTOR_Z_DIR_Pin_num 5
#elif	MOTOR_Z_DIR_Pin == 	LL_GPIO_PIN_6
	#define MOTOR_Z_DIR_Pin_num 6
#elif	MOTOR_Z_DIR_Pin == 	LL_GPIO_PIN_7
	#define MOTOR_Z_DIR_Pin_num 7
#elif	MOTOR_Z_DIR_Pin == 	LL_GPIO_PIN_8
	#define MOTOR_Z_DIR_Pin_num 8
#elif	MOTOR_Z_DIR_Pin == 	LL_GPIO_PIN_9
	#define MOTOR_Z_DIR_Pin_num 9
#elif	MOTOR_Z_DIR_Pin == 	LL_GPIO_PIN_10
	#define MOTOR_Z_DIR_Pin_num 10
#elif	MOTOR_Z_DIR_Pin == 	LL_GPIO_PIN_11
	#define MOTOR_Z_DIR_Pin_num 11
#elif	MOTOR_Z_DIR_Pin == 	LL_GPIO_PIN_12
	#define MOTOR_Z_DIR_Pin_num 12
#elif	MOTOR_Z_DIR_Pin == 	LL_GPIO_PIN_13
	#define MOTOR_Z_DIR_Pin_num 13
#elif	MOTOR_Z_DIR_Pin == 	LL_GPIO_PIN_14
	#define MOTOR_Z_DIR_Pin_num 14
#elif	MOTOR_Z_DIR_Pin == 	LL_GPIO_PIN_15
	#define MOTOR_Z_DIR_Pin_num 15
#endif 	
#define zdir_forward	1
#define zdir_backward	0
#define xdir_forward	1
#define xdir_backward	0



// define calculated at compiling preprocessor time bit band values for GPIOx-ODR register directly to corresponding bit
// so it can be used for example as XDIR = zdir_forward;
#define XDIR	*((volatile uint32_t *) ((PERIPH_BB_BASE + (uint32_t)(  (uint8_t *)MOTOR_X_DIR_GPIO_Port+0xC 	- PERIPH_BASE)*32 + ( MOTOR_X_DIR_Pin_num*4 ))))
#define ZDIR	*((volatile uint32_t *) ((PERIPH_BB_BASE + (uint32_t)(  (uint8_t *)MOTOR_Z_DIR_GPIO_Port+0xC 	- PERIPH_BASE)*32 + ( MOTOR_Z_DIR_Pin_num*4 ))))

#define t3cr1			((uint32_t *)((PERIPH_BB_BASE  + ((TIM3_BASE)-PERIPH_BASE)*32)))
#define t4cr1			((uint32_t *)((PERIPH_BB_BASE  + ((TIM4_BASE)-PERIPH_BASE)*32)))
#define t4sr			((uint32_t *)((PERIPH_BB_BASE  + ((TIM4_BASE+0x10)-PERIPH_BASE)*32))) // 0x10 is shift value of SR register
#define t4dier		((uint32_t *)((PERIPH_BB_BASE  + ((TIM4_BASE+0x0C)-PERIPH_BASE)*32))) // 0x0C is shift of DIER register

#define disable_encoder_ticks() t4dier[TIM_DIER_UIE_Pos] = 0    
#define enable_encoder_ticks()  t4dier[TIM_DIER_UIE_Pos] = 1    
#define auto_mode_delay_ms 4000
#define step_divider 2 //stepper driver divider microstep

#define BIT_BAND_SRAM(RAM,BIT) (*(volatile uint32_t*)(SRAM_BB_BASE+32*((uint32_t)((void*)(RAM))-SRAM_BASE)+4*((uint32_t)(BIT))))
#define BB_PERI(c,d) 	*((volatile uint32_t *) ((PERIPH_BB_BASE + (uint32_t)( &( c ) 			- PERIPH_BASE)*32 + ( d*4 ))))



#define MOTOR_Z_SetPulse()           t3cr1[TIM_CR1_CEN_Pos] = 1 //bitbang version, or with LL: LL_TIM_EnableCounter(TIM3) 
#define MOTOR_Z_RemovePulse()        // dummy macro, pulse disabled by hardware


#define MOTOR_Z_Enable()             MOTOR_Z_ENABLE_GPIO_Port->BSRR = MOTOR_Z_ENABLE_Pin
#define MOTOR_Z_Disable()            MOTOR_Z_ENABLE_GPIO_Port->BRR  = MOTOR_Z_ENABLE_Pin

#define LED_OFF()		LL_GPIO_SetOutputPin(LED_GPIO_Port, LED_Pin)
#define LED_ON()		LL_GPIO_ResetOutputPin(LED_GPIO_Port, LED_Pin)

#define MOTOR_X_CHANNEL         		LL_TIM_CHANNEL_CH3
#define MOTOR_Z_CHANNEL         		LL_TIM_CHANNEL_CH1
#define MOTOR_Z_OnlyPulse()         TIM3->CCER = MOTOR_Z_CHANNEL
#define MOTOR_Z_AllowPulse()         t3ccer[TIM_CCER_CC1E_Pos] = 1
#define MOTOR_Z_BlockPulse()         t3ccer[TIM_CCER_CC1E_Pos] = 0

#define MOTOR_X_OnlyPulse()         TIM3->CCER = MOTOR_X_CHANNEL
#define MOTOR_X_AllowPulse()         t3ccer[TIM_CCER_CC3E_Pos] = 1
#define MOTOR_X_BlockPulse()         t3ccer[TIM_CCER_CC3E_Pos] = 0

#define z_to_x_factor2210	1537 //1024*200*61/16/1,27/400	todo move to some central point to modify

#define SUBSTEP_AXIS_Z 0
#define SUBSTEP_AXIS_X 1

#define MOTOR_X_SetPulse()           t3cr1[TIM_CR1_CEN_Pos] = 1 //LL_TIM_EnableCounter(TIM3) //__HAL_TIM_ENABLE(&htim3)
#define MOTOR_X_RemovePulse()        // dummy macro, pulse disabled by hardware

#define MOTOR_X_Enable()             MOTOR_X_ENABLE_GPIO_Port->BSRR = MOTOR_X_ENABLE_Pin
#define MOTOR_X_Disable()            MOTOR_X_ENABLE_GPIO_Port->BRR  = MOTOR_X_ENABLE_Pin

#define Spindle_Direction_CW         0
#define Spindle_Direction_CCW        1
#define feed_direction_left         0 // from right to left
#define feed_direction_right        1 // from left to right

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
