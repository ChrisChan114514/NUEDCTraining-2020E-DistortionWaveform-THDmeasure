#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"
#include "stdio.h"
#include "lcd.h"
#include "arm_math.h"
#include "arm_const_structs.h"
#include "LCD_Show.h"
#include "math.h"
#include "ad9833.h"

#define FFT_LENGTH 4096										//采样点个数
#define LOW_F_INDEX 80                                      //太低频的分量应省略
#define JUDGE_AVE 250
#define BIZHI_SIZE 5
#define FOCUS_NUM 10                                          //能量集中聚焦数量
#define FOCUS_JUDGE  20                                      //20个索引内（20*f0频率内），能量直接集中到先位
#define SIN_JUDGE  890
#define FOCUS_JUDGE_3 40                                    //40个索引内（40*f0频率内），能量直接集中到三倍基波分量
#define TYPE_JUDGE_NUM 6                                   //波形类型的削平参数
#define OUT_REDUCE 10                                        // fft_outbuff 进行平方和运算时，防止过大先进行缩小
#define BASIC_FRE_POS  160                                   //25.6K时，1K在4096点的160处
#define SMALLER_E  5                                        //防止能量集中溢出，先对能量进行缩小
#define THD_RAM_SIZE 10                                   //THD测量取均值，稳定数据
#define FFT_ABS_LIMIT 14                                    //FFT值小于这个，直接算作0

extern float adcVolt[FFT_LENGTH];
extern float PWM_frequency;
extern uint32_t f_sa;										//基础采样频率为25K=84M/(410*12)	
float fft_inputbuf[FFT_LENGTH * 2];  			//fft计算前数组(含实部虚部)
float fft_outputbuf[FFT_LENGTH];  				//fft计算后数组
float bizhi1[BIZHI_SIZE]; //最大基波分量与其三倍分量的比值
int p1=0; //bizhi1索引
float buff3[400];													//存储数值缓存
float buff2[FFT_LENGTH*2]={0};						//数值缓存
char temp_str;
uint16_t fft_index_max = 1;
uint16_t fft_index_max2 = 1;
uint16_t fft_index_min = 1;
uint16_t waveform_mode = 1;							//1输出方波，2输出三角波，3输出正弦波
extern float fft_frequency;
extern float fft_frequency2;
float fft_direct = 0;
char f_buff[30];
char V_buff[20];
char K_buff[20];
int cnt_calculate = 0;									//用于降低刷新率
int flag_judge;											//是否开始判波的标志
char SPR_STR[100];
int TYPEsumA=0; 
int TYPEsumB=0; 
int CLKsum=0;  //记录判断的次数

//----------THD RAM 均值缓冲 ------------------
int RAM_index=0;
float THD_RAM[THD_RAM_SIZE]={0};
//---------------------------------------------
struct FRE
{
	float FREC;
	int FRECint;
	int TYPE;
	int TYPEpro;
}FREa,FREb;


void quickSort(float values[], int indices[], int low, int high) {
    if (low < high) {
        float pivot = values[high];
        int i = low - 1;

        for (int j = low; j < high; j++) {
            if (values[j] > pivot) { // 从大到小排序
                i++;
                // 交换值
                float tempVal = values[i];
                values[i] = values[j];
                values[j] = tempVal;

                // 交换索引
                int tempIndex = indices[i];
                indices[i] = indices[j];
                indices[j] = tempIndex;
            }
        }
        // 最后交换
        float tempVal = values[i + 1];
        values[i + 1] = values[high];
        values[high] = tempVal;

        int tempIndex = indices[i + 1];
        indices[i + 1] = indices[high];
        indices[high] = tempIndex;

        quickSort(values, indices, low, i);
        quickSort(values, indices, i + 2, high);
    }
}


void get_vpp(void)	   //获取峰峰值
{
	float max_data=adcVolt[0];
	float min_data=adcVolt[0];
	u32 n=0;
	float Vpp=0;

	for(n = 1;n<FFT_LENGTH;n++)
	{
			if(adcVolt[n] > max_data)
			{
				max_data = adcVolt[n];
			}
			if(adcVolt[n] < min_data)
			{
				min_data = adcVolt[n];
			}	
	} 
		Vpp = (float)(max_data - min_data);

}

float fft_frequency_Vpp_check(uint16_t fft_index_max)				//fft测频率及峰峰值优化函数，用于补偿栅栏效应 
																	//此函数就是将下标值转化为频率值
{
	float sum_A = 0;
//	float sum_Vpp = 0;
//	float Vpp = 0;
	float frequency_average = 0;
	
	//这里只取了5个点，如果不同主要频率点下标相差比较大，可以取更多；反之更少
	int left_dot  = fft_index_max-2;				//设置栅栏效应的左邻谐波边界
	int right_dot = fft_index_max+3;				//设置栅栏效应的右邻谐波边界

	//边界处理
	if(left_dot <= 0)
	{
		left_dot = 1;
	}
	if(right_dot >= FFT_LENGTH/2)
	{
		right_dot = FFT_LENGTH/2-1;
	}
	
	//求Vpp,这里对fft_outputbuf[fft_index_max]附近的点,我们用先求平方和后开方的方法来聚集能量，减小误差		@光 24.7.12
	for (int i = left_dot;i <= right_dot;i++)
	{
		sum_A += fft_outputbuf[i];																					//为下面计算frequency_average做准备，所以不能删
//		sum_Vpp += fft_outputbuf[i]*fft_outputbuf[i];
	}
//	Vpp = sqrt(sum_Vpp)*2;
//	sprintf(V_buff,"Vpp:%0.3f",Vpp);
//	lcd_show_string(10,420,80,16,16,V_buff,WHITE);
	
	//求frequency_average,这里对fft_outputbuf[fft_index_max]附近的点进行加权处理		@光 24.7.12
	for (int i = left_dot;i <= right_dot;i++)
	{
		frequency_average += (i*f_sa/FFT_LENGTH) * (fft_outputbuf[i]/sum_A);			//F测 =（F下标*f_sa采样频率/采样点）*（权重占比）
	}
//	lcd_show_string(405,40,80,16,16,"fft_F(Hz):",YELLOW);
//	sprintf((char*)f_buff,"%0.3fHz",fft_frequency);
//	lcd_show_string(405,60,130,16,16,f_buff,WHITE);
//	sprintf((char*)f_buff,"%0.3fHz",fft_frequency2);
//	lcd_show_string(405,120,80,16,16,"fft_F2(Hz):",YELLOW);
//	lcd_show_string(405,140,80,16,16,f_buff,WHITE);
	return frequency_average;
}

 

void fft_THD()  //测量THD
{
	//先全部打印，观察频谱图
	int i;
	int flag=0;
	float BAS_E[5]; //存放聚焦过的基频倍频能量
	float THD_NUM=0;
	float THD_RE=0; //THD均值结果
	
	
//	for(i=0;i<FFT_LENGTH;i++)
//	{
//		printf("%d,%f\n",i,fft_outputbuf[i]);
//	}
	//对一到五倍频率进行能量集中
	for (i=BASIC_FRE_POS;i<=BASIC_FRE_POS*5;i+=BASIC_FRE_POS)
	{
		for (int j=i-FOCUS_NUM;j<=i+FOCUS_NUM;j++)
		{
			if (fft_outputbuf[j]>FFT_ABS_LIMIT)
			{
				BAS_E[flag]+=(fft_outputbuf[j]/SMALLER_E)*(fft_outputbuf[j]/SMALLER_E);
			}
			
		}
		flag++;
	}
	//集中过的能量进行开方
	for (i=0;i<5;i++)
	{
		BAS_E[i]=sqrt(BAS_E[i]);
	}
	//THD运算
	THD_NUM=sqrt(BAS_E[4]*BAS_E[4]+BAS_E[1]*BAS_E[1]+BAS_E[2]*BAS_E[2]+BAS_E[3]*BAS_E[3])/BAS_E[0];
	THD_RAM[RAM_index]=(THD_NUM>100)?100:THD_NUM;
	//求THD_RAM均值 
	for (i=0;i<THD_RAM_SIZE;i++)
	{
		THD_RE+=(THD_RAM[i]>100)?100:THD_RAM[i];
	}
	if (RAM_index==THD_RAM_SIZE-1)
	{
		RAM_index=0;
	}
	else
	{
		RAM_index+=1;
	}
	THD_RE=THD_RE/THD_RAM_SIZE;
	printf("%f\n",THD_RE);
	lcd_show_string(100,10,80,16,24,"THD",YELLOW);
	sprintf(SPR_STR,"%f%%",THD_RE*100);
	lcd_show_string(100,35,80,16,16,SPR_STR,YELLOW);
//	sprintf(SPR_STR,"%f",THD_NUM*100);
//	lcd_show_string(100,90,80,16,16,SPR_STR,YELLOW);
	
}


void fftCalculate(void)// FFT 计算函数
{
    
		if (cnt_calculate == 0)
		{
			for (int i = 0; i < FFT_LENGTH; i++)								//均值滤波来提高采样精度,每两个点取平均，这样就可以达到2次均值滤波
		{
				fft_inputbuf[i * 2] = adcVolt[i];//实部赋值
				fft_inputbuf[i * 2 + 1] = 0;//虚部赋值，固定为0.
		}

		
		arm_cfft_f32(&arm_cfft_sR_f32_len4096, fft_inputbuf, 0, 1);			//这边len后参数要联动FFT_LENGTH修改
		arm_cmplx_mag_f32(fft_inputbuf, fft_outputbuf, FFT_LENGTH);
		
		fft_index_max = 1;
		fft_index_min = 1;
		
		fft_direct = fft_outputbuf[0]/FFT_LENGTH; 						
		//处理直流分量（低频）
		for (int i =0;i<LOW_F_INDEX;i++)
		{
			fft_outputbuf[i] = 0;  //存入fft_direct直流分量并让fft_outputbuf[0]置0，保证不影响波形判断
		}
//		//FFT运算，阈值限制
//		for (int i=0;i<FFT_LENGTH;i++)
//		{
//			if (fft_outputbuf[i]<FFT_ABS_LIMIT)
//			{
//				fft_outputbuf[i]=0;
//			}
//		}
													
		
		//fft_waveform_check_H();
		fft_THD();

			cnt_calculate = 25;
		}
		else
		{
			cnt_calculate -= 1;
		}
		

}


