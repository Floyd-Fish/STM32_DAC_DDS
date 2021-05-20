#ifndef __DDS_H
#define __DDS_H

#include "main.h"

//	Maximum Length of the Look-Up-Table
#define LUT_MAX_LENGTH      	(uint32_t)4096

//	Output Amptitude of DAC (Full is 2048)
//	Do not use Full scale 
//	Due to opa2211's non-rail-to-rail, the output cannot reach power's rail
#define DAC_AMP								(uint16_t)1600

#define DDS_MAX_AMP 					(float)5.00f

//	Wave types listed below
enum 
{
    SINE_WAVE = 0,						//	Test Pass
    SQUARE_WAVE = 1,					//	Test Pass
    TRIANGLE_WAVE = 2,				//	Test Pass
	SINC_WAVE = 3,						//	Still under Testing
	EXP_WAVE = 4,							//	Test Pass
};


//	DDS Type Define
typedef struct 
{
    uint8_t         waveType;
    uint32_t        freq;
    uint32_t        lutLen;
    float        		amp;
}   DDS_TypeDef;


//	Start DDS
void ddsStart(void);

void ddsStop(void);

void DDS_setWaveParams(uint32_t freq, float amptitude, uint8_t type);

//	Get a specific type of wave's LUT
void getNewWaveLUT(uint32_t length, uint8_t type);




//void ddsSetAmptitude(uint16_t code);


















#endif
