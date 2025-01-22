#include "ad9833.h"

#include "spi.h"


//#define CS_9833_0() HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_RESET) //Ƭѡ�������ͺ���
//#define CS_9833_1() HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_SET)
////AD9833 ����
///******************************************************************************
//*  ��������AD9833_Init
//*  ����˵������ʼ��AD9833���GPIO
//*  ��  �Σ���
//*  ����ֵ����
//*******************************************************************************
//*/
//void AD9833_GPIOinit(void)
//{
//	CS_9833_1();
//}
///******************************************************************************
//*  ��������AD9833_Write
//*  ����˵������AD9833����һ��16bit��
//*  ��  �Σ�TxData Ҫ���͵���
//*  ����ֵ����
//*******************************************************************************
//*/
//void AD9833_Write(unsigned short TxData)  //д��Ĵ���
//{
//	unsigned char data[2];
//	data[0]=(unsigned char)((TxData>>8)&0xff);
//	data[1]=(unsigned char)(TxData&0xff);
//	CS_9833_0();
//	HAL_SPI_Transmit(&hspi2,data,2,0x02);
//	CS_9833_1();
//	
//}
///******************************************************************************
//*  ��������AD9833_CtrlSet
//*  ����˵��������AD9833�����ʽ
//*  ��  �Σ�Reset:0Ϊ�������1Ϊ�����
//           SleepMode:3�ر��ڲ�ʱ�Ӻ�DAC,0���ر�
//           optionbit|modebit:
//                     00���Ҳ���01���ǲ���10������11����
//*  ����ֵ����
//*******************************************************************************
//*/
//void AD9833_CtrlSet(unsigned char Reset,unsigned char SleepMode,unsigned char optionbit,unsigned char modebit)
//{
//	unsigned short regtmep=0;
//	regtmep=regtmep|(((unsigned short)Reset&0x01)<<8); //�Ĵ���λ������
//	regtmep=regtmep|((SleepMode&0x03)<<6);
//	regtmep=regtmep|((optionbit&0x01)<<5);
//	regtmep=regtmep|((modebit&0x01)<<1);
//	
//	AD9833_Write(regtmep);
//}
///******************************************************************************
//*  ��������AD9833_FreqSet
//*  ����˵��������AD9833���Ƶ��
//*  ��  �Σ�Freq: Ƶ��ֵ��0.1HZ~12.5MHZ��,Ϊ����ֵ��һ��
//*  ����ֵ����
//*******************************************************************************
//*/
//void AD9833_Freq(double Freq)
//{
//	int frequence_LSB,frequence_MSB;
//	double frequence_mid,frequence_DATA;
//	long int frequence_hex;
//	//Ƶ�ʻ���
//	frequence_mid=268435456/25;
//	frequence_DATA=Freq;
//	frequence_DATA=frequence_DATA/1000000;
//	frequence_DATA=frequence_DATA*frequence_mid;
//	frequence_hex=frequence_DATA;
//	frequence_LSB=frequence_hex; //Ƶ�ʵ�λ
//	frequence_LSB=frequence_DATA; //32bit
//	frequence_LSB=frequence_LSB&0x3fff; //14bit
//	frequence_MSB=frequence_LSB>>14;    //Ƶ�ʸ�λ
//	frequence_MSB=frequence_MSB&0x3fff; //14bit
//	
//	//ʹ��Ƶ�ʼĴ���
//	frequence_LSB=frequence_LSB|0x4000;
//	frequence_MSB=frequence_MSB|0x4000;
//	
//	AD9833_Write(0x2100);
//	AD9833_Write(frequence_LSB);
//	AD9833_Write(frequence_MSB);
//}  

void AD9833_GPIOinit(void)

{

	CS_9833_1(); //��������Ƭѡ����ʹ�ܣ���ֹ�����������

}



/*

*******************************************************

��AD9833����һ��16bit������

*******************************************************

*/

void AD9833_Write(unsigned short TxData) //TxData��2�ֽ�

{

	unsigned char data[2] ; //һ��charһ���ֽڣ�����Ϊ2���ֽ�

	data[0] = (unsigned char)((TxData>>8) &0xff); //data[0]�洢��λ

	data[1] = (unsigned char)(TxData&0xff); //data[1]�洢��λ

	CS_9833_0(); //����Ƭѡ��׼��д��

	HAL_SPI_Transmit (&hspi2 , data, 2, 0x02) ; //��HAL���SPI���ͺ�����������

	CS_9833_1(); //������ϣ�����Ƭѡ

}

  

/*

*******************************************************

Reset: 0Ϊ�������1Ϊû�������λֻ�����������������λ�Ĵ���

SleeppMode: 3Ϊ�ر��ڲ�DAC��ʱ�ӣ�0Ϊ���ر�

optionbit|modebit: 00����01����10����11����

*******************************************************

*/

void AD9833_CtrlSet(unsigned char Reset,unsigned char SleeppMode,unsigned char optionbit,unsigned char modebit)

{

	unsigned short regtemp = 0; //�����ģʽ��һЩѡ��

	regtemp = regtemp|(((unsigned short)Reset&0x01)<<8); //���¾��ǰ�ÿ��λ��Ӧ����Ӧ�ļĴ����ϣ������������˸�DIV2,DIV2Ĭ��Ϊ0�ˣ�

	regtemp = regtemp|((SleeppMode&0x03)<<6);

	regtemp = regtemp|((optionbit&0x01)<<5);

	regtemp = regtemp|((modebit&0x01)<<1);

	AD9833_Write(regtemp); //д�����ݣ���������Ҫ��������Ҫд����

}

  

/*

*******************************************************

����Ƶ�ʣ���������ݲ��������CtrlSet�����������

Ƶ��ֵ��0.1Hz-12.5MHz�����ֵΪ25M����ʱ�ӵ�һ�룩

��λ��Hz;���磬���1M��������1000000

*******************************************************

*/

void AD9833_FreqSet(double Freq) //Freq���û��������HzΪ��λ��Ƶ��

{

	int frequence_LSB,frequence_MSB; //LSB��MSB�ֱ��ӦƵ�ʼĴ������LSB��MSB

	double frequence_mid,frequence_DATA; //mid��һ���м�ֵ�����ڽ�����Ƶ��ת��ΪAD9833���Խ��ܵĸ�ʽ��DATAΪ�����ȥ��Ƶ�ʵ�ʮ����

	long int frequence_hex; //hexΪ�����ȥ��Ƶ�ʵ�16����

	  

	frequence_mid = 268435456/25; //f_{out}=dds���Ƶ�ʣ�F_{cw}=f_{DATA}��f_{ref}=25M��2^28=268435456��1000000=1M

	frequence_DATA = Freq; //��ʽ��f_{out}=(F_{cw})*(f_{ref})/(2^28)=(f_{DATA})*25M/(2^28)

	frequence_DATA = frequence_DATA/1000000; //�����f_{DATA}(������MΪ��λ)=(Freq/1000000)*[(2^28)/(25)]��;f_{DATA}��Ϊf_{cw}�������ʽ���պõõ�f_{out}=Freq������HzΪ��λ

	frequence_DATA = frequence_DATA*frequence_mid;

	frequence_hex = frequence_DATA;

	frequence_LSB = frequence_hex;

	frequence_LSB = frequence_LSB&0x3fff; //���¾���һЩ����������������Ĵ���

	frequence_MSB = frequence_hex>>14;

	frequence_MSB = frequence_MSB&0x3fff;

	  

	frequence_LSB = frequence_LSB|0x4000;

	frequence_MSB = frequence_MSB|0x4000; //4000==0100 0000 0000 0000,��ǰ��λ���01������freq0�Ĵ���

	  

	AD9833_Write(0x2100); //2100==0010 0001 0000 0000;bit8Ϊ1->��reset;0010->����дLSB��MSB��־

	AD9833_Write(frequence_LSB);

	AD9833_Write(frequence_MSB);

  

}
