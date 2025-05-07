/* USER CODE BEGIN Header */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "AS608.h"
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define FINGER_CLEAR  "clear"
#define PIN_OPEN "1111"

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
SysPara AS608Para;							//ָ指纹模块AS608参数
uint16_t ValidN;								//模块内有效模板个数

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* Size of Reception buffer */

uint8_t aRxBuffer[RXBUFFERSIZE];				//接收缓冲
uint8_t RX_len=0;													//接收字节计数
void UsartReceive_IDLE(UART_HandleTypeDef *huart)//空闲中断回调函数
{
  __HAL_UART_CLEAR_IDLEFLAG(&huart1); //清除中断

  RX_len = RXBUFFERSIZE - huart1.RxXferCount; //计算接收数据长度
  HAL_UART_AbortReceive_IT(huart); //终止接收

  HAL_UART_Receive_IT(&huart1, (uint8_t*)aRxBuffer, RXBUFFERSIZE); //接收完数据后再次打开中断接收函数
}



//录指纹
void Add_FR(void)
{
	uint8_t i=0,ensure ,processnum=0;
	uint16_t ID;
	while(1)
	{
		printf("go\r\n");
		switch (processnum)
		{
			case 0:
				i++;
				ensure=GZ_GetImage();
				if(ensure==0x00) 
				{
					ensure=GZ_GenChar(CharBuffer1);//生成特征
					if(ensure==0x00)
					{
						i=0;
						processnum=1;//跳到第二步							
					}			
				}						
				break;
			
			case 1:
				i++;
				ensure=GZ_GetImage();
				if(ensure==0x00) 
				{
					ensure=GZ_GenChar(CharBuffer2);//生成特征		
					if(ensure==0x00)
					{
						i=0;
						processnum=2;//跳到第三步
					}
				}	
				break;

			case 2:
				ensure=GZ_Match();
				if(ensure==0x00) 
				{
					processnum=3;//跳到第四步
				}
				else 
				{
					i=0;
					processnum=0;//跳回第一步			
				}
				HAL_Delay(1000);
				break;

			case 3:

				ensure=GZ_RegModel();
				if(ensure==0x00) 
				{
					processnum=4;//跳到第五步
				}else {processnum=0;}
				HAL_Delay(1000);
				break;
				
			case 4:	

		  	ID=0;
				ensure=GZ_StoreChar(CharBuffer2,ID);//储存模板
				if(ensure==0x00) 
				{			
					GZ_ValidTempleteNum(&ValidN);//读库指纹个数
					printf("%d\r\n",ValidN);
					HAL_Delay(1500);//延时后清除显示	
					return ;
				}else {processnum=0;}					
				break;				
		}
		HAL_Delay(800);
		if(i==10)				//超过10次没有按手指则退出
		{
			break;	
		}				
	}
}

//刷指纹
void press_FR(void)
{
	//printf("press_fr\r\n");
	SearchResult seach;
	uint8_t ensure;
	char *str;
	ensure=GZ_GetImage();
	if(ensure==0x00)						//获取图像成功 
	{	
		ensure=GZ_GenChar(CharBuffer1);
		if(ensure==0x00) 					//生成特征成功
		{		
			ensure=GZ_HighSpeedSearch(CharBuffer1,0,300,&seach);
			if(ensure==0x00)				//搜索成功
			{	

					HAL_UART_Transmit(&huart3,(uint8_t*)"clock in",8,0xffff);
					printf("success 1\r\n");
//          if(seach.mathscore>100)
//					{
//						printf("success\r\n");
//						HAL_Delay(20000);
//					}else
//					{
//					}

			}
			else
			{
					//HAL_UART_Transmit(&huart3,(uint8_t*)"error finger",12,0xffff);
					printf("error finger\r\n");
			}

	  }
	}
		
}

//删除指纹
void Del_FR(void)
{
	uint8_t  ensure;
	uint16_t num;
	GZ_Empty();						//清空指纹库
	//GZ_DeletChar(0,1);			//删除单个指纹
}

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
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */


	if (HAL_UART_Receive_IT(&huart1, (uint8_t *)aRxBuffer, RXBUFFERSIZE) != HAL_OK)//接收完数据后再次打开中断接收函数
  {
   
  }
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE); //使能空闲中断
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		while(GZ_HandShake(&AS608Addr))//与AS608模块握手
		{
				
		}
		GZ_ValidTempleteNum(&ValidN);  //读库指纹个数
    GZ_ReadSysPara(&AS608Para);  //读AS608模块参数 
		
		if(ValidN>0)   //指纹里面有模版
		{
			while(1)
			{
				press_FR();//刷指纹
				if(rx3_end_flag)
				{
					rx3_end_flag=false;
					
					//删除指纹
					if(strstr((const char*)FINGER_CLEAR,(const char*)uart3_rx)!=NULL)
					{
							Del_FR();
					}
					
					if(strstr((const char*)PIN_OPEN,(const char*)uart3_rx)!=NULL)
					{
							 HAL_GPIO_WritePin(LAY_GPIO_Port, LAY_Pin, GPIO_PIN_RESET);

					}
					
					memset(uart3_rx,0,100);
					rx3_count=0;
					HAL_UART_Receive_DMA(&huart3,uart3_rx,100);   //��Ҫ���´�DMA����
				}
			}	
		}
		else
		{
			HAL_UART_Transmit(&huart3,(uint8_t*)"no finger",9,0xffff);
			Add_FR();//录入指纹	
			HAL_Delay(1000);
		}
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

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
