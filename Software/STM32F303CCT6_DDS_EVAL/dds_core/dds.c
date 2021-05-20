#include "dds.h"
#include "arm_math.h"
#include "dac.h"
#include "tim.h"
#include "math.h"

DDS_TypeDef     dds;
volatile uint16_t        dds_lut[LUT_MAX_LENGTH];

void ddsStart(void)
{
	/*
	DAC7811_SCLK_H();
	DAC7811_SDIN_H();
	DAC7811_SYNC_H();
	*/
	
	dds.freq = 1000;
	dds.amp = 4.5;
	dds.waveType = SINE_WAVE;
	dds.lutLen = 800;
	
	//	Default configuration
	getNewWaveLUT(800, TRIANGLE_WAVE);
	
	HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_2, (uint32_t*)dds_lut, 400, DAC_ALIGN_12B_R);
	HAL_TIM_Base_Start(&htim8);
}

void ddsStop(void)
{
	HAL_TIM_Base_Stop(&htim8);
	HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_2);
}

void DDS_setWaveParams(uint32_t freq, float amptitude, uint8_t type)
{
	
	//	Stop DAC DMA Transfer, and leave tim8 running, that's fine
	
	HAL_TIM_Base_Stop(&htim8);
	HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_2);

	//	Select wave type and update it
	switch(type)
	{
		case SINE_WAVE:
			dds.waveType = SINE_WAVE;
			break;
		case SQUARE_WAVE:
			dds.waveType = SQUARE_WAVE;
			break;
		case TRIANGLE_WAVE:
			dds.waveType = TRIANGLE_WAVE;
			break;
		case SINC_WAVE:
			dds.waveType = SINC_WAVE;
			break;
		case EXP_WAVE:
			dds.waveType = EXP_WAVE;
			break;
		default:
			break;
	}
	
	dds.freq = freq;
	//	Select frequency range and register timer's parameters
	if (freq == 1)
	{
		//	for very low frequencies, FMCLK = 2kHz, 72M / 720 / 50 = 2kHz
		TIM8 -> PSC = 720-1;
		TIM8 -> ARR = 50-1;
		dds.lutLen = 4000;
		getNewWaveLUT(dds.lutLen, dds.waveType);
	}
	else if (freq == 10)
	{
		//	FMCLK = 20kHz, 72M / 72 / 50 = 20kHz
		TIM8 -> PSC = 72-1;
		TIM8 -> ARR = 50-1;
		dds.lutLen = 4000;
		getNewWaveLUT(dds.lutLen, dds.waveType);
	}
	else if (freq > 10 && freq <= 100)
	{
		//	FMCLK = 200kHz, 72M / 72 / 50 = 20kHz
		TIM8 -> PSC = 72-1;
		TIM8 -> ARR = 50-1;
		dds.lutLen = (uint32_t)(40000 / freq);
		getNewWaveLUT(dds.lutLen, dds.waveType);
	}
	else if (freq > 100 && freq <= 1000)
	{
		//	FMCLK = 200kHz, 72M / 360 = 200kHz
		TIM8 -> PSC = 36-1;
		TIM8 -> ARR = 10-1;
		dds.lutLen = (uint32_t)(400000 / freq);
		getNewWaveLUT(dds.lutLen, dds.waveType);
	}
	else if (freq > 1000 && freq<= 10000)
	{
		//	FMCLK = 2MHz, 72M / 36 = 2MHz
		TIM8 -> PSC = 4-1;
		TIM8 -> ARR = 9-1;
		dds.lutLen = (uint32_t)(4000000 / freq);
		getNewWaveLUT(dds.lutLen, dds.waveType);
	}
	else if (freq > 10000 && freq <= 400000)
	{
		//	Maximum Speed of DMA Dataflow
		//	FMCLK = 4MHz, 72M / 18 = 4MHz
		TIM8 -> PSC = 0;
		TIM8 -> ARR = 18-1;
		dds.lutLen = (uint32_t)(8000000 / freq);
		getNewWaveLUT(dds.lutLen, dds.waveType);
	}

	
	dds.amp = amptitude;
	//	Resize the wave LUT by Amptitude factor
	for (uint16_t i = 0; i < dds.lutLen; i++)
	{
		dds_lut[i] = dds_lut[i] * (amptitude / DDS_MAX_AMP);
	}
	
	//	Restart DAC DMA Transfer
	HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_2, (uint32_t*)dds_lut, dds.lutLen/2, DAC_ALIGN_12B_R);
	HAL_TIM_Base_Start(&htim8);

}


void getNewWaveLUT(uint32_t length, uint8_t type)
{
	if (type == SINE_WAVE)
	{
		float sin_step = 2.0f * 3.14159f / (float)(length-1);
		for (uint16_t i = 0; i < length; i++)
		{
			dds_lut[i] = (uint16_t)(DAC_AMP+(DAC_AMP*sinf(sin_step*(float)i)));
		}
	}
	
	else if (type == SQUARE_WAVE)
	{
		for(uint16_t i = 0; i < length / 2; i++)
		{
			dds_lut[i] = DAC_AMP*2;
			dds_lut[i + (length / 2)] = 0;
		}
	}
	
	else if (type == TRIANGLE_WAVE)
	{
		uint16_t tri_step = DAC_AMP*2 / (length/2);
		
		for(uint16_t i = 0; i < length / 2; i++)
		{
			dds_lut[i] = DAC_AMP*2 - tri_step*i;
			dds_lut[length - i - 1] = dds_lut[i];
		}
	}
	else  if (type == SINC_WAVE)
	{
		float sin_step = 2.0f * 3.14159f / (float)(length-1);
		for (uint16_t i = (length/2); i < length; i++)
		{
			dds_lut[i] = (uint16_t)(DAC_AMP + DAC_AMP * sinf(sin_step * (float)i) / (sin_step * (float)i));
			dds_lut[length - i - 2] = dds_lut[i];
		}
		dds_lut[(length/2)-1] = 2*DAC_AMP;
	}
	else if (type == EXP_WAVE)
	{
		float exp_step =  DAC_AMP*4 / length;
		for(uint16_t i = 0; i < length; i++)
		{
			dds_lut[i] = DAC_AMP*2*(1-pow(2.82, -(exp_step*i / DAC_AMP)));
		}		
		
	}
	

}























