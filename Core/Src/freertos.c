/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main_app.h"
#include "app_bsp.h"
#include "motor_control.h"
#include "motor_mix.h"
#include "motor_output.h"
#define MPU_TASK_PERIOD_MS			10

sm_remote_ctrl_t remote_data = {0};
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* ---- Flight task ---- */
#define FLIGHT_PERIOD_MS        2U          /* 500 Hz, configTICK_RATE_HZ = 1000 */
#define FLIGHT_DT               0.002f      /* motor_control chi nhan 1.5..2.5 ms */

#define FLIGHT_MAX_ANGLE_DEG    30.0f       /* stick 100 -> 30 do */
#define FLIGHT_MAX_YAW_DPS      150.0f
#define FLIGHT_THR_ACTIVE       5U          /* ga < 5% -> idle, khong chay PID */
#define FLIGHT_THR_IDLE         0.05f       /* ga idle khi da arm (0..1) */
#define FLIGHT_IMU_ERR_MAX      10U         /* loi IMU lien tiep -> disarm (20 ms) */

/* TEST: 1 = bo qua stick roll/pitch, setpoint goc = 0 (drone tu giu thang bang).
 * Ga va yaw van lay tu remote. Dat ve 0 khi muon dieu khien roll/pitch. */
#define FLIGHT_TEST_LEVEL_HOLD  1

/* Dau stick -> quy uoc cua motor_mix (MPU truc X huong mui, Z huong len):
 * roll+ = nghieng phai, pitch+ = cui dau, yaw+ = quay trai (CCW).
 * Doi dau o day neu stick bi nguoc. */
#define RC_ROLL_SIGN            ( 1.0f)
#define RC_PITCH_SIGN           ( 1.0f)
#define RC_YAW_SIGN             (-1.0f)     /* stick phai = quay phai = gz am */

/* Cau hinh ESC/PWM (tan so, us, thu tu kenh) nam trong main_app.c: g_motor_out_cfg */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for remoteTask */
osThreadId_t remoteTaskHandle;
const osThreadAttr_t remoteTask_attributes = {
  .name = "remoteTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for FlightTask */
osThreadId_t FlightTaskHandle;
const osThreadAttr_t FlightTask_attributes = {
  .name = "FlightTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityRealtime,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartRemoteTask(void *argument);
void StartFlightTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of remoteTask */
  remoteTaskHandle = osThreadNew(StartRemoteTask, NULL, &remoteTask_attributes);

  /* creation of FlightTask */
  FlightTaskHandle = osThreadNew(StartFlightTask, NULL, &FlightTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(100);
//	  main_app_t* app = main_app;
//	  sm_remote_read(app->m_remote, &remote_data);
//	  sm_hal_pwm_set_duty(app->m_drive.m_pwm_channel2,PWM_CHANNEL_2,remote_data.throttle*0.01f);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartRemoteTask */
/**
* @brief Function implementing the remoteTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartRemoteTask */
void StartRemoteTask(void *argument)
{
  /* USER CODE BEGIN StartRemoteTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
    main_app_t* app = main_app;
    sm_remote_read(app->m_remote, &remote_data);
  }
  /* USER CODE END StartRemoteTask */
}

/* USER CODE BEGIN Header_StartFlightTask */
/**
* @brief Function implementing the FlightTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartFlightTask */
void StartFlightTask(void *argument)
{
  /* USER CODE BEGIN StartFlightTask */
	main_app_t *app = main_app;
	sm_motor_control_t *ctrl = sm_motor_control_init();
	sm_motor_output_t *out = app->m_drive.m_motor_out;
	sm_attitude_angle_t att = {0};
	sm_remote_ctrl_t rc;
	sm_motor_control_sp_t sp;
	sm_motor_control_output_t ref;
	sm_mixer_input_t mix_in;
	sm_mixer_output_t mix_out;
	float m[MOTOR_COUNT] = {0};
	uint32_t imu_err = 0;
	uint8_t was_armed = 0;

	sm_motor_output_stop(out);			/* ESC arm o 1000 us */

	uint32_t next_tick = osKernelGetTickCount();
  /* Infinite loop */
  for(;;)
  {
	next_tick += FLIGHT_PERIOD_MS;

	/* 1. IMU: doc MPU + cap nhat quaternion (I2C 400 kHz ~0.4 ms).
	 *    Loi le 1-2 lan thi dung lai goc cu, loi lien tiep moi disarm. */
	if (sm_attitude_est_update(app->m_attitude) == 0
			&& sm_attitude_est_get_angle(app->m_attitude, &att) == 0)
		imu_err = 0;
	else if (imu_err < FLIGHT_IMU_ERR_MAX)
		imu_err++;
	uint8_t imu_ok = (imu_err < FLIGHT_IMU_ERR_MAX);

	/* 2. Remote: tra -1 va xoa ve 0 khi mat link/timeout */
	uint8_t rc_ok = (sm_remote_read(app->m_remote, &rc) == 0);

	/* 3. Arm/disarm an toan */
	if (!rc_ok || !imu_ok || !ctrl)
		app->m_motor_run = 0;			/* mat link: bat buoc an nut arm lai */
	if (app->m_motor_run && !was_armed && rc.throttle >= FLIGHT_THR_ACTIVE)
		app->m_motor_run = 0;			/* chi cho arm khi ga dang o thap */
	uint8_t armed = app->m_motor_run;
	was_armed = armed;

	if (!armed) {
		sm_motor_control_reset(ctrl);
		sm_motor_output_stop(out);
	}
	else if (rc.throttle < FLIGHT_THR_ACTIVE) {
		/* Dang nam tren dat: chi idle, khong de I tich luy */
		sm_motor_control_reset(ctrl);
		for (uint32_t i = 0; i < MOTOR_COUNT; i++) m[i] = FLIGHT_THR_IDLE;
		sm_motor_output_write(out, m);
	}
	else {
		/* 4. Setpoint tu stick */
#if FLIGHT_TEST_LEVEL_HOLD
		sp.roll_set     = 0.0f;
		sp.pitch_set    = 0.0f;
#else
		sp.roll_set     = RC_ROLL_SIGN  * rc.roll  * (FLIGHT_MAX_ANGLE_DEG / 100.0f);
		sp.pitch_set    = RC_PITCH_SIGN * rc.pitch * (FLIGHT_MAX_ANGLE_DEG / 100.0f);
#endif
		sp.yaw_rate_set = RC_YAW_SIGN   * rc.yaw   * (FLIGHT_MAX_YAW_DPS   / 100.0f);

		/* 5. Vong ngoai (goc, 100 Hz) + vong trong (rate, 500 Hz) */
		mix_in.throttle = rc.throttle * 0.01f;
		if (sm_motor_control_update(ctrl, &att, &sp, FLIGHT_DT, &ref) == 0) {
			mix_in.roll  = ref.roll;
			mix_in.pitch = ref.pitch;
			mix_in.yaw   = ref.yaw;
		} else {
			/* Timing/du lieu loi: giu throttle, bo hieu chinh chu ky nay */
			mix_in.roll = mix_in.pitch = mix_in.yaw = 0.0f;
		}

		/* 6. Mixer -> output */
		sm_motor_mixer_update(&mix_in, &mix_out);
		for (uint32_t i = 0; i < MOTOR_COUNT; i++)
			m[i] = (mix_out.motor[i] < FLIGHT_THR_IDLE) ? FLIGHT_THR_IDLE : mix_out.motor[i];
		sm_motor_output_write(out, m);
	}

	osDelayUntil(next_tick);
  }
  /* USER CODE END StartFlightTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

