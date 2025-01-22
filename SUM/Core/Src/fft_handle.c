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

#define FFT_LENGTH 4096										//���������
#define LOW_F_INDEX 80                                      //̫��Ƶ�ķ���Ӧʡ��
#define JUDGE_AVE 250
#define BIZHI_SIZE 5
#define FOCUS_NUM 10                                          //�������о۽�����
#define FOCUS_JUDGE  20                                      //20�������ڣ�20*f0Ƶ���ڣ�������ֱ�Ӽ��е���λ
#define SIN_JUDGE  890
#define FOCUS_JUDGE_3 40                                    //40�������ڣ�40*f0Ƶ���ڣ�������ֱ�Ӽ��е�������������
#define TYPE_JUDGE_NUM 6                                   //�������͵���ƽ����
#define OUT_REDUCE 10                                        // fft_outbuff ����ƽ��������ʱ����ֹ�����Ƚ�����С
#define BASIC_FRE_POS  160                                   //25.6Kʱ��1K��4096���160��
#define SMALLER_E  5                                        //��ֹ��������������ȶ�����������С
#define THD_RAM_SIZE 10                                   //THD����ȡ��ֵ���ȶ�����
#define FFT_ABS_LIMIT 14                                    //FFTֵС�������ֱ������0

extern float adcVolt[FFT_LENGTH];
extern float PWM_frequency;
extern uint32_t f_sa;										//��������Ƶ��Ϊ25K=84M/(410*12)	
float fft_inputbuf[FFT_LENGTH * 2];  			//fft����ǰ����(��ʵ���鲿)
float fft_outputbuf[FFT_LENGTH];  				//fft���������
float bizhi1[BIZHI_SIZE]; //�����������������������ı�ֵ
int p1=0; //bizhi1����
float buff3[400];													//�洢��ֵ����
float buff2[FFT_LENGTH*2]={0};						//��ֵ����
char temp_str;
uint16_t fft_index_max = 1;
uint16_t fft_index_max2 = 1;
uint16_t fft_index_min = 1;
uint16_t waveform_mode = 1;							//1���������2������ǲ���3������Ҳ�
extern float fft_frequency;
extern float fft_frequency2;
float fft_direct = 0;
char f_buff[30];
char V_buff[20];
char K_buff[20];
int cnt_calculate = 0;									//���ڽ���ˢ����
int flag_judge;											//�Ƿ�ʼ�в��ı�־
char SPR_STR[100];
int TYPEsumA=0; 
int TYPEsumB=0; 
int CLKsum=0;  //��¼�жϵĴ���

//----------THD RAM ��ֵ���� ------------------
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
            if (values[j] > pivot) { // �Ӵ�С����
                i++;
                // ����ֵ
                float tempVal = values[i];
                values[i] = values[j];
                values[j] = tempVal;

                // ��������
                int tempIndex = indices[i];
                indices[i] = indices[j];
                indices[j] = tempIndex;
            }
        }
        // ��󽻻�
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


void get_vpp(void)	   //��ȡ���ֵ
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

float fft_frequency_Vpp_check(uint16_t fft_index_max)				//fft��Ƶ�ʼ����ֵ�Ż����������ڲ���դ��ЧӦ 
																	//�˺������ǽ��±�ֵת��ΪƵ��ֵ
{
	float sum_A = 0;
//	float sum_Vpp = 0;
//	float Vpp = 0;
	float frequency_average = 0;
	
	//����ֻȡ��5���㣬�����ͬ��ҪƵ�ʵ��±����Ƚϴ󣬿���ȡ���ࣻ��֮����
	int left_dot  = fft_index_max-2;				//����դ��ЧӦ������г���߽�
	int right_dot = fft_index_max+3;				//����դ��ЧӦ������г���߽�

	//�߽紦��
	if(left_dot <= 0)
	{
		left_dot = 1;
	}
	if(right_dot >= FFT_LENGTH/2)
	{
		right_dot = FFT_LENGTH/2-1;
	}
	
	//��Vpp,�����fft_outputbuf[fft_index_max]�����ĵ�,����������ƽ���ͺ󿪷��ķ������ۼ���������С���		@�� 24.7.12
	for (int i = left_dot;i <= right_dot;i++)
	{
		sum_A += fft_outputbuf[i];																					//Ϊ�������frequency_average��׼�������Բ���ɾ
//		sum_Vpp += fft_outputbuf[i]*fft_outputbuf[i];
	}
//	Vpp = sqrt(sum_Vpp)*2;
//	sprintf(V_buff,"Vpp:%0.3f",Vpp);
//	lcd_show_string(10,420,80,16,16,V_buff,WHITE);
	
	//��frequency_average,�����fft_outputbuf[fft_index_max]�����ĵ���м�Ȩ����		@�� 24.7.12
	for (int i = left_dot;i <= right_dot;i++)
	{
		frequency_average += (i*f_sa/FFT_LENGTH) * (fft_outputbuf[i]/sum_A);			//F�� =��F�±�*f_sa����Ƶ��/�����㣩*��Ȩ��ռ�ȣ�
	}
//	lcd_show_string(405,40,80,16,16,"fft_F(Hz):",YELLOW);
//	sprintf((char*)f_buff,"%0.3fHz",fft_frequency);
//	lcd_show_string(405,60,130,16,16,f_buff,WHITE);
//	sprintf((char*)f_buff,"%0.3fHz",fft_frequency2);
//	lcd_show_string(405,120,80,16,16,"fft_F2(Hz):",YELLOW);
//	lcd_show_string(405,140,80,16,16,f_buff,WHITE);
	return frequency_average;
}

 

void fft_THD()  //����THD
{
	//��ȫ����ӡ���۲�Ƶ��ͼ
	int i;
	int flag=0;
	float BAS_E[5]; //��ž۽����Ļ�Ƶ��Ƶ����
	float THD_NUM=0;
	float THD_RE=0; //THD��ֵ���
	
	
//	for(i=0;i<FFT_LENGTH;i++)
//	{
//		printf("%d,%f\n",i,fft_outputbuf[i]);
//	}
	//��һ���屶Ƶ�ʽ�����������
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
	//���й����������п���
	for (i=0;i<5;i++)
	{
		BAS_E[i]=sqrt(BAS_E[i]);
	}
	//THD����
	THD_NUM=sqrt(BAS_E[4]*BAS_E[4]+BAS_E[1]*BAS_E[1]+BAS_E[2]*BAS_E[2]+BAS_E[3]*BAS_E[3])/BAS_E[0];
	THD_RAM[RAM_index]=(THD_NUM>100)?100:THD_NUM;
	//��THD_RAM��ֵ 
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


void fftCalculate(void)// FFT ���㺯��
{
    
		if (cnt_calculate == 0)
		{
			for (int i = 0; i < FFT_LENGTH; i++)								//��ֵ�˲�����߲�������,ÿ������ȡƽ���������Ϳ��Դﵽ2�ξ�ֵ�˲�
		{
				fft_inputbuf[i * 2] = adcVolt[i];//ʵ����ֵ
				fft_inputbuf[i * 2 + 1] = 0;//�鲿��ֵ���̶�Ϊ0.
		}

		
		arm_cfft_f32(&arm_cfft_sR_f32_len4096, fft_inputbuf, 0, 1);			//���len�����Ҫ����FFT_LENGTH�޸�
		arm_cmplx_mag_f32(fft_inputbuf, fft_outputbuf, FFT_LENGTH);
		
		fft_index_max = 1;
		fft_index_min = 1;
		
		fft_direct = fft_outputbuf[0]/FFT_LENGTH; 						
		//����ֱ����������Ƶ��
		for (int i =0;i<LOW_F_INDEX;i++)
		{
			fft_outputbuf[i] = 0;  //����fft_directֱ����������fft_outputbuf[0]��0����֤��Ӱ�첨���ж�
		}
//		//FFT���㣬��ֵ����
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


