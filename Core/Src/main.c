/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "oled.h"
#include "mpu6050.h"
#include "dht11.h"
#include "queue.h"
#include "alarm.h"
#include "fault_monitor.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct
{
    uint8_t temperature;
    uint8_t humidity;
    uint8_t valid;
} DHT_Message_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//#define VIBRATION_THRESHOLD 3000
//#define ALARM_COUNT_LIMIT   3
//#define TEMP_HIGH_THRESHOLD    27
//#define HUM_HIGH_THRESHOLD     85

#define MPU_CALIBRATION_SAMPLE_COUNT    50
#define MPU_CALIBRATION_MAX_ATTEMPTS    100
#define MPU_CALIBRATION_INTERVAL_MS     20


#define DHT_FAULT_COUNT_LIMIT       3
#define DHT_RECOVER_COUNT_LIMIT     2

#define MPU_FAULT_COUNT_LIMIT       3
#define MPU_RECOVER_COUNT_LIMIT     2

#define KEY_GPIO_PORT        GPIOA
#define KEY_GPIO_PIN         GPIO_PIN_0
#define KEY_PRESSED_LEVEL    GPIO_PIN_RESET
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;

osThreadId defaultTaskHandle;
osThreadId ledtaskHandle;
/* USER CODE BEGIN PV */
volatile uint32_t led_count = 0;

static QueueHandle_t g_dht_queue = NULL;

static QueueHandle_t g_key_queue = NULL;
//volatile uint8_t g_dht_temperature = 0;
//volatile uint8_t g_dht_humidity = 0;
//volatile uint8_t g_dht_valid = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_I2C1_Init(void);
void StartDefaultTask(void const * argument);
void StartLedTask(void const * argument);

/* USER CODE BEGIN PFP */
void LedTask(void *argument);
void PrintTask(void *argument);
void I2CScanTask(void *argument);
void OledTask(void *argument);
void MpuTask(void *argument);
static int32_t Abs32(int32_t x);
static void Buzzer_On(void);
static void Buzzer_Off(void);
void DhtTask(void *argument);

void KeyTask(void *argument);

static uint8_t MPU6050_Calibrate(int32_t *base_ax,
                                 int32_t *base_ay,
                                 int32_t *base_az);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
	BaseType_t ret_led;
	BaseType_t ret_print;
	BaseType_t ret_mpu;
	BaseType_t ret_dht;
	BaseType_t ret_key;
	
	g_dht_queue = xQueueCreate(1, sizeof(DHT_Message_t));

	if (g_dht_queue == NULL)
	{
			printf("Create DHT queue failed!\r\n");
			Error_Handler();
	}
	
	g_key_queue = xQueueCreate(1, sizeof(uint8_t));

	if (g_key_queue == NULL)
	{
			printf("Create key queue failed!\r\n");
			Error_Handler();
	}

	printf("Before create tasks\r\n");

	ret_led=xTaskCreate(LedTask,
            "LedTask",
            128,
            NULL,
            tskIDLE_PRIORITY + 1,
            NULL);

//	ret_print=xTaskCreate(PrintTask,
//            "PrintTask",
//            256,
//            NULL,
//            tskIDLE_PRIORITY + 1,
//            NULL);

//xTaskCreate(I2CScanTask,
//            "I2CScanTask",
//            256,
//            NULL,
//            tskIDLE_PRIORITY + 1,
//            NULL);

//xTaskCreate(OledTask,
//            "OledTask",
//            256,
//            NULL,
//            tskIDLE_PRIORITY + 1,
//            NULL);

	ret_mpu=xTaskCreate(MpuTask,
            "MpuTask",
            512,
            NULL,
            tskIDLE_PRIORITY + 1,
            NULL);

	ret_dht = xTaskCreate(DhtTask,
                      "DhtTask",
                      256,
                      NULL,
                      tskIDLE_PRIORITY + 1,
                      NULL);
									
	ret_key = xTaskCreate(KeyTask,
                      "KeyTask",
                      128,
                      NULL,
                      tskIDLE_PRIORITY + 1,
                      NULL);

	printf("create ret: led=%d mpu=%d dht=%d key=%d\r\n",
       ret_led,
       ret_mpu,
       ret_dht,
       ret_key);

//	printf("create ret: led=%d print=%d mpu=%d\r\n",ret_led, ret_print, ret_mpu);

	vTaskStartScheduler();
	
	printf("Scheduler start failed!\r\n");

  /* USER CODE END 2 */

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
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of ledtask */
  osThreadDef(ledtask, StartLedTask, osPriorityNormal, 0, 128);
  ledtaskHandle = osThreadCreate(osThread(ledtask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11|GPIO_PIN_12, GPIO_PIN_SET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB11 PB12 */
  GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

void LedTask(void *argument)
{
    while (1)
    {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        led_count++;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void PrintTask(void *argument)
{
    while (1)
    {
        printf("FreeRTOS running, led_count = %lu\r\n", led_count);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void I2CScanTask(void *argument)
{
    HAL_StatusTypeDef result;
    uint8_t found;

    while (1)
    {
        found = 0;
        printf("\r\n========== I2C Scan Start ==========\r\n");

        for (uint8_t addr = 1; addr < 127; addr++)
        {
            result = HAL_I2C_IsDeviceReady(&hi2c1, addr << 1, 2, 10);

            if (result == HAL_OK)
            {
                printf("I2C device found: 7bit addr = 0x%02X, HAL addr = 0x%02X\r\n",
                       addr, addr << 1);
                found++;
            }

            vTaskDelay(pdMS_TO_TICKS(1));
        }

        if (found == 0)
        {
            printf("No I2C device found!\r\n");
        }

        printf("========== I2C Scan End, found = %d ==========\r\n", found);

        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void OledTask(void *argument)
{
    int count = 0;

    OLED_Init();
    OLED_Clear();

    OLED_ShowString(0, 0, "OLED OK");
    OLED_ShowString(2, 0, "CNT:");

    printf("OLED show string ok\r\n");

    while (1)
    {
        OLED_ShowString(2, 30, "      ");
        OLED_ShowNum(2, 30, count);

        count++;

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static int32_t Abs32(int32_t x)
{
    if (x < 0)
    {
        return -x;
    }
    else
    {
        return x;
    }
}

void MpuTask(void *argument)
{
    /*
     * MPU6050 当前一次读取的数据。
     * 初始化为0，避免读取失败时出现未初始化数据。
     */
    MPU6050_Data_t mpu_data = {0};

    uint8_t id;
    uint8_t ret;

    /*
     * 按键消音相关变量。
     */
    uint8_t key_event = 0;
    uint8_t alarm_mute_flag = 0;

    /*
     * 报警管理器。
     * 振动、温度、湿度和系统总报警状态
     * 全部保存在这个结构体中。
     */
    AlarmManager_t alarm_manager;

    FaultMonitor_t dht_fault_monitor;
		FaultMonitor_t mpu_fault_monitor;

    /*
     * 本轮MPU6050是否读取成功。
     *
     * 1：本轮读取成功
     * 0：本轮读取失败
     */
    uint8_t mpu_read_ok = 0;

    

    /*
     * MPU6050静止时的三轴基准值。
     */
    int32_t base_ax = 0;
    int32_t base_ay = 0;
    int32_t base_az = 0;
		
		uint8_t calibration_ok = 0;
		uint8_t mpu_effective_fault = 0;

    /*
     * DHT11队列消息。
     *
     * dht_message：
     * 保存当前正在使用的最新温湿度消息。
     *
     * new_message：
     * 临时接收队列中新到达的消息。
     */
    DHT_Message_t dht_message = {0};
    DHT_Message_t new_message;

    /*
     * 当前循环使用的温湿度快照。
     */
    uint8_t temperature = 0;
    uint8_t humidity = 0;
    uint8_t dht_valid = 0;

    /*
     * 当前振动强度。
     */
    int32_t vibration = 0;

    /*
     * 初始化报警管理器内部的计数器和报警状态。
     */
    Alarm_Init(&alarm_manager);
		
		FaultMonitor_Init(&dht_fault_monitor);
		FaultMonitor_Init(&mpu_fault_monitor);

    printf("MpuTask start\r\n");

    /*
     * 初始化OLED。
     */
    OLED_Init();
    OLED_Clear();

    OLED_ShowString(0, 0, "MPU6050 TEST");

    /*
     * 初始化MPU6050，并读取设备ID。
     */
    ret = MPU6050_Init();
    id = MPU6050_ReadID();

    OLED_ShowString(2, 0, "ID:");
    OLED_ShowNum(2, 24, id);

    if (ret == 0)
    {
        OLED_ShowString(4, 0, "MPU OK");
        printf("MPU6050 init ok, id = 0x%02X\r\n", id);
    }
    else
    {
        OLED_ShowString(4, 0, "MPU ERR");
        printf("MPU6050 init error, id = 0x%02X\r\n", id);
    }

    vTaskDelay(pdMS_TO_TICKS(1000));

    /*
     * MPU6050静态基准校准。
     */
    OLED_Clear();
		OLED_ShowString(0, 0, "CAL...");
		printf("MPU6050 calibrating...\r\n");

		/*
		 * 尝试完成首次校准。
		 */
		if (MPU6050_Calibrate(&base_ax,
                       &base_ay,
                       &base_az) == 0)
		{
				calibration_ok = 1;
				vibration = 0;
 
				/*
				 * 已经连续成功取得50个校准样本，
				 * 说明MPU通信已经恢复。
				 * 重新初始化故障监视器。
				 */
				FaultMonitor_Init(&mpu_fault_monitor);

				printf("MPU recalibration success, "
							 "BASE: AX=%ld, AY=%ld, AZ=%ld\r\n",
							 (long)base_ax,
							 (long)base_ay,
							 (long)base_az);
		}
		else
		{
				calibration_ok = 0;

				OLED_Clear();
				OLED_ShowString(0, 0, "CAL ERR");

				printf("MPU calibration error\r\n");

				vTaskDelay(pdMS_TO_TICKS(1000));
		}

    /*
     * OLED固定标题。
     */
    OLED_Clear();

    OLED_ShowString(0, 0, "T:");
    OLED_ShowString(2, 0, "H:");
    OLED_ShowString(4, 0, "S:");

    while (1)
    {
        /*
         * ==================================================
         * 第1部分：接收DHT11任务发送的新消息
         * ==================================================
         */
        if (xQueueReceive(g_dht_queue,
                          &new_message,
                          0) == pdPASS)
        {
            /*
             * 保存最新消息。
             */
            dht_message = new_message;

            FaultMonitor_Update(&dht_fault_monitor,
                        new_message.valid,
                        DHT_FAULT_COUNT_LIMIT,
                        DHT_RECOVER_COUNT_LIMIT);
               
        }

        /*
         * 把当前DHT消息复制到本轮使用的局部变量。
         */
        temperature = dht_message.temperature;
        humidity = dht_message.humidity;
        dht_valid = dht_message.valid;

        /*
         * ==================================================
         * 第2部分：读取MPU6050并更新故障状态
         * ==================================================
         */

        /*
         * 每轮开始时，先假设本轮读取失败。
         */
        mpu_read_ok = 0;

				/*
				 * 校准尚未成功时，周期性重新尝试校准。
				 */
				if (calibration_ok == 0)
				{
						OLED_ShowString(4, 0, "CAL...");

						if (MPU6050_Calibrate(&base_ax,
																	 &base_ay,
																	 &base_az) == 0)
						{
								calibration_ok = 1;

								/*
								 * 校准成功后清除旧振动值。
								 */
								vibration = 0;

								printf("MPU recalibration success, "
											 "BASE: AX=%ld, AY=%ld, AZ=%ld\r\n",
											 (long)base_ax,
											 (long)base_ay,
											 (long)base_az);
						}
				}
				else
				{
						/*
						 * 校准成功后，才进行正常的MPU数据读取。
						 */
						if (MPU6050_ReadAccel(&mpu_data) == 0)
						{
								mpu_read_ok = 1;

								vibration =
										Abs32((int32_t)mpu_data.ax - base_ax)
									+ Abs32((int32_t)mpu_data.ay - base_ay)
									+ Abs32((int32_t)mpu_data.az - base_az);
						}

						/*
						 * 每次实际读取MPU后，更新通信故障状态。
						 */
						FaultMonitor_Update(&mpu_fault_monitor,
																mpu_read_ok,
																MPU_FAULT_COUNT_LIMIT,
																MPU_RECOVER_COUNT_LIMIT);
						
						
						/*
						 * MPU已经确认通信故障时，
						 * 原来的静止基准不再认为可靠。
						 */
						if (mpu_fault_monitor.fault_flag == 1)
						{
								calibration_ok = 0;
								vibration = 0;
						}
				}
				

        /*
         * ==================================================
         * 第3部分：统一更新所有报警状态
         * ==================================================
         *
         * 到这里时，下面的数据已经全部准备完成：
         *
         * vibration
         * temperature
         * humidity
         * dht_valid
         * dht_fault_monitor.fault_flag
         * mpu_fault_monitor.fault_flag
         *
         * Alarm_Update()内部负责：
         *
         * 1. 振动连续计数和滞回
         * 2. 温度滞回
         * 3. 湿度滞回
         * 4. 传感器故障参与系统报警
         * 5. 生成系统总报警
         */
				 
				if ((calibration_ok == 0) ||
							(mpu_fault_monitor.fault_flag == 1))
				{
						mpu_effective_fault = 1;
				}
				else
				{
						mpu_effective_fault = 0;
				}
				
				
        Alarm_Update(&alarm_manager,
             vibration,
             temperature,
             humidity,
             dht_valid,
             mpu_read_ok,
             dht_fault_monitor.fault_flag,
             mpu_effective_fault);

        /*
         * ==================================================
         * 第4部分：处理按键消音事件
         * ==================================================
         */
        if (xQueueReceive(g_key_queue,
                          &key_event,
                          0) == pdPASS)
        {
            /*
             * 只有系统正在报警时，
             * 按键才设置消音标志。
             */
            if (alarm_manager.state.system_alarm == 1)
            {
                alarm_mute_flag = 1;
            }
        }

        /*
         * 所有异常恢复后，自动清除消音状态。
         * 保证下一次新报警仍然能够响铃。
         */
        if (alarm_manager.state.system_alarm == 0)
        {
            alarm_mute_flag = 0;
        }

        /*
         * ==================================================
         * 第5部分：控制蜂鸣器
         * ==================================================
         *
         * 蜂鸣器响必须同时满足：
         *
         * 1. 系统存在报警
         * 2. 当前没有被人工消音
         */
        if ((alarm_manager.state.system_alarm == 1) &&
            (alarm_mute_flag == 0))
        {
            Buzzer_On();
        }
        else
        {
            Buzzer_Off();
        }

        /*
         * ==================================================
         * 第6部分：刷新OLED
         * ==================================================
         */

        /*
         * 清除上一次显示的数值。
         */
        OLED_ShowString(0, 12, "        ");
        OLED_ShowString(2, 12, "        ");
        OLED_ShowString(4, 12, "        ");
        OLED_ShowString(6, 0,  "          ");

        /*
         * DHT11当前数据有效，并且没有确认故障时，
         * 才显示温湿度。
         */
        if ((dht_valid == 1) &&
            (dht_fault_monitor.fault_flag == 0))
        {
            OLED_ShowNum(0, 12, temperature);
            OLED_ShowString(0, 30, "C");

            OLED_ShowNum(2, 12, humidity);
        }
        else
        {
            OLED_ShowString(0, 12, "--");
            OLED_ShowString(2, 12, "--");
        }

        /*
         * 本轮MPU6050读取成功，并且没有确认故障时，
         * 才显示振动强度。
         */
        if ((calibration_ok == 1) &&
						(mpu_read_ok == 1) &&
						(mpu_fault_monitor.fault_flag == 0))
				{
						OLED_ShowNum(4, 12, vibration);
				}
				else
				{
						OLED_ShowString(4, 12, "--");
				}

        /*
         * OLED显示的是系统真实报警状态。
         * 即使按键已经消音，仍然显示ALARM。
         */
        if (alarm_manager.state.system_alarm == 1)
        {
            OLED_ShowString(6, 0, "ALARM");
        }
        else
        {
            OLED_ShowString(6, 0, "OK");
        }

        /*
         * ==================================================
         * 第7部分：串口输出完整状态
         * ==================================================
         */
        printf("T=%d C, H=%d %%, DHT_V=%d, "
							 "AX=%d, AY=%d, AZ=%d, S=%ld, "
							 "CAL=%d, MPU_OK=%d, "
							 "V_ALM=%d, T_ALM=%d, H_ALM=%d, "
							 "DHT_F=%d, MPU_F=%d, SYS=%d, MUTE=%d\r\n",
							 temperature,
							 humidity,
							 dht_valid,
							 mpu_data.ax,
							 mpu_data.ay,
							 mpu_data.az,
							 (long)vibration,
							 calibration_ok,
							 mpu_read_ok,
							 alarm_manager.state.vibration_alarm,
							 alarm_manager.state.temperature_alarm,
							 alarm_manager.state.humidity_alarm,
							 dht_fault_monitor.fault_flag,
							 mpu_effective_fault,
							 alarm_manager.state.system_alarm,
							 alarm_mute_flag);

        /*
         * MpuTask每300ms运行一轮。
         */
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}

static void Buzzer_On(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
}

static void Buzzer_Off(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
}

void DhtTask(void *argument)
{
    DHT11_Data_t dht_data;
		DHT_Message_t message;
    uint8_t ret;

    DHT11_Init();
	
		message.temperature = 0;
    message.humidity = 0;
    message.valid = 0;

    vTaskDelay(pdMS_TO_TICKS(2000));

    while (1)
    {
        ret = DHT11_Read(&dht_data);


        if (ret == DHT11_OK)
        {
            message.temperature = dht_data.temperature;
            message.humidity = dht_data.humidity;
            message.valid = 1;
        }
        else
        {
            message.valid = 0;
        }

        xQueueOverwrite(g_dht_queue, &message);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void KeyTask(void *argument)
{
    uint8_t key_event = 1;

    while (1)
    {
        if (HAL_GPIO_ReadPin(KEY_GPIO_PORT,
                             KEY_GPIO_PIN) ==
            KEY_PRESSED_LEVEL)
        {
            /*
             * 延时20ms消除机械按键抖动。
             */
            vTaskDelay(pdMS_TO_TICKS(20));

            if (HAL_GPIO_ReadPin(KEY_GPIO_PORT,
                                 KEY_GPIO_PIN) ==
                KEY_PRESSED_LEVEL)
            {
                /*
                 * 确认按键按下，发送按键事件。
                 * 队列长度为1，始终保留最新事件。
                 */
                xQueueOverwrite(g_key_queue, &key_event);

                /*
                 * 等待按键释放，避免一次长按
                 * 被识别成多次按下。
                 */
                while (HAL_GPIO_ReadPin(KEY_GPIO_PORT,
                                        KEY_GPIO_PIN) ==
                       KEY_PRESSED_LEVEL)
                {
                    vTaskDelay(pdMS_TO_TICKS(10));
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static uint8_t MPU6050_Calibrate(int32_t *base_ax,
                                 int32_t *base_ay,
                                 int32_t *base_az)
{
    MPU6050_Data_t data;

    int32_t sum_ax = 0;
    int32_t sum_ay = 0;
    int32_t sum_az = 0;

    uint16_t success_count = 0;
    uint16_t attempt_count = 0;

    /*
     * 必须获得规定数量的成功样本，
     * 同时限制最大尝试次数，避免永久卡住。
     */
    while ((success_count < MPU_CALIBRATION_SAMPLE_COUNT) &&
           (attempt_count < MPU_CALIBRATION_MAX_ATTEMPTS))
    {
        attempt_count++;

        if (MPU6050_ReadAccel(&data) == 0)
        {
            sum_ax += data.ax;
            sum_ay += data.ay;
            sum_az += data.az;

            success_count++;
        }

        vTaskDelay(
            pdMS_TO_TICKS(MPU_CALIBRATION_INTERVAL_MS));
    }

    /*
     * 没能收集到足够的成功数据，校准失败。
     */
    if (success_count < MPU_CALIBRATION_SAMPLE_COUNT)
    {
        printf("MPU calibration failed, success=%u, attempts=%u\r\n",
               success_count,
               attempt_count);

        return 1;
    }

    /*
     * 使用实际成功样本数量计算平均值。
     */
    *base_ax = sum_ax / (int32_t)success_count;
    *base_ay = sum_ay / (int32_t)success_count;
    *base_az = sum_az / (int32_t)success_count;

    printf("MPU calibration success, samples=%u\r\n",
           success_count);

    return 0;
}


/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartLedTask */
/**
* @brief Function implementing the ledtask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLedTask */
void StartLedTask(void const * argument)
{
  /* USER CODE BEGIN StartLedTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartLedTask */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM4 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM4) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
