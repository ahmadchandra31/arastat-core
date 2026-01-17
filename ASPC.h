#ifndef ASPC_H
#define ASPC_H

// #include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
// #include "math.h"
// #include "stm32g0xx_hal.h"
#include "stdio.h"

#define ASPC_SCANRATE_50MV  50
#define ASPC_SCANRATE_100MV 100 
#define ASPC_SCANRATE_300MV 300
#define ASPC_SCANRATE_500MV 500

#define ASPC_5V_REF 5000
#define ASPC_3_3V_REF 3300

#define SAMPLERATE 10   //#samples generated per second
#define DAC_RESOLUTION 12 //12-bit
#define TIA_RESISTOR 100 //1k Resistor is placed in the TIA potentiostat

extern volatile uint16_t DACIndex;

typedef struct
{
    /* data all voltages are in mV*/ 
    uint16_t V_scanRate;
    uint16_t V_ref;
    int16_t V_initial; //initial voltage scan set the same as the V_ref to disable it.
    int16_t V_start; //start scan 
    int16_t V_final; //final scan 
    uint8_t DAC_RES;
    uint16_t rate;
    uint8_t mode;
    uint8_t isDAQEnabled; //flag to store data or not
    int16_t *buffer_volt; //buffer to store adc values
    int16_t *buffer_curr; //buffer to store adc values    
    //private generated from get_dac_sequence
    uint16_t _dac_size;
    uint16_t *dac_sequence;
    int16_t vRef; //measured reference voltage
    uint16_t RTIA; //transimpedance amplifier resistor value 
}ASPC_Conf;



typedef enum {
	LINEAR_SWEEP_VOLTAMMETRY = 1,
    CYCLIC_VOLTAMMETRY,
    CHRONOAMPEROMETRY,
    ASPC_SET_SAMPLE_RATE,
    ASPC_SET_ADC_DAC_DELAY,
    ASPC_SET_DAC_IDLE_VOLTAGE,
    ASPC_SET_CYCLES,
    ASPC_SET_DAC_EEPROM,
    ASPC_SET_STARTING_VOLTAGE,
    SET_REF_MEAS_VOLTAGE,
    SET_RTIA,
    SET_QUIET_TIME,
}ASPC_Mode_t;

void ASPC_init(ASPC_Conf *_ASPC);
void ASPC_configure(ASPC_Conf *_ASPC,int16_t * data);
void ASPC_deinit(ASPC_Conf *_ASPC);
int16_t ASPC_DACtoVolt(ASPC_Conf *_ASPC,uint16_t DACValue);
float ASPC_GetDACStepValue(ASPC_Conf *_ASPC);
uint16_t ASPC_GetDACInitialVoltage(ASPC_Conf *_ASPC);
void ASPC_SetDACResolution(ASPC_Conf *_ASPC,uint8_t _DAC_RES);
void ASPC_SetReferenceVoltage(ASPC_Conf *_ASPC,uint16_t Vref);
void ASPC_SetStartVoltage(ASPC_Conf *_ASPC,int16_t VStart);
void ASPC_SetVInit(ASPC_Conf *_ASPC,int16_t Vinit);
void ASPC_SetVFinal(ASPC_Conf *_ASPC,int16_t Vfinal);
void ASPC_SetScanRate(ASPC_Conf *_ASPC,uint16_t VScan);
void ASPC_SetMode(ASPC_Conf *_ASPC, uint8_t mode);
void ASPC_SetDACResolution(ASPC_Conf *_ASPC, uint8_t resolution);
void ASPC_SetSampleRate(ASPC_Conf *_ASPC,uint16_t sample_rate);
uint16_t ASPC_GetDACVFinal(ASPC_Conf *_ASPC);
uint16_t ASPC_VToDAC(uint16_t vRef, int16_t vTarget);
uint16_t * ASPC_GetDACSequence(ASPC_Conf *_ASPC);
void ASPC_GetDACs(ASPC_Conf *_ASPC);
float ASPC_GetCurrent(ASPC_Conf *_ASPC, uint16_t Rval, int16_t adcValue);
void enableDataAcquisition(ASPC_Conf* _ASPC);
void disableDataAcquisition(ASPC_Conf* _ASPC);
uint8_t isDAQEnabled(ASPC_Conf* _ASPC);
uint16_t *GenerateSequence(ASPC_Conf *_ASPC, int16_t V_start,int16_t V_final, uint8_t cyclic);
uint16_t VoltageToDAC(int16_t voltage,uint16_t vRef, uint16_t dacResolution);
void get_raw_data(ASPC_Conf* _ASPC);

//Computation functions
void ASPC_setReferenceMeasuredVoltage(ASPC_Conf* _ASPC, int16_t vRef);
void ASPC_setRTIA(ASPC_Conf* _ASPC, uint16_t RTIA);
void ASPC_ComputeCurrent(ASPC_Conf* _ASPC);
int16_t ASPC_getRerefenceMeasuredVoltage(ASPC_Conf* _ASPC);
uint16_t ASPC_getRTIA(ASPC_Conf* _ASPC);


#endif
