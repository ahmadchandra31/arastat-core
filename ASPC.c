/*
ARASTAT POTENTIOSTAT CORE
Written by arachanx21
ARA APPLIED 2023


This software is licensed by ARA APPLIED through BSD-3 License. ARA APPLIED provides the code AS IS AND 
is not RESPONSIBLE for any DAMAGES incurred from the use of the code.  All text here must be included in any distribution
You may copy, modify,redistribute directly or in derivatives as long as it's in compliance with the license.
*/
#include "ASPC.h"

void ASPC_init(ASPC_Conf *_ASPC){
    //default configuration for the ASPC
    _ASPC->dac_sequence=NULL;
    _ASPC->DAC_RES=12;
    #ifdef USE_5V
    _ASPC->V_ref=ASPC_5V_REF;
    #else
    _ASPC->V_ref=ASPC_3_3V_REF;
    #endif
    _ASPC->V_start=-600;
    //V_initial set the same as reference to disable the initial voltage
    _ASPC->V_initial=_ASPC->V_start;
    _ASPC->V_scanRate=100;
    _ASPC->mode=CYCLIC_VOLTAMMETRY;
    _ASPC->rate=SAMPLERATE;
    _ASPC->vRef=_ASPC->V_ref/2;
    // _ASPC->dac_sequence=ASPC_GetDACSequence(_ASPC);
}

/**
 * @brief Configure an ASPC device instance from a packed parameter array.
 *
 * This function initializes several configuration fields of the provided
 * ASPC_Conf structure using values supplied in a 4-element int16_t array.
 * It also validates the reference voltage field and forces a default if it
 * is not one of the supported reference constants.
 *
 * Parameters expected in the data array (in order):
 *   data[0] - operating mode (ASPC mode identifier)
 *   data[1] - start voltage (millivolts)
 *   data[2] - final/target voltage (millivolts)
 *   data[3] - scan rate (implementation-defined units)
 *
 * @param _ASPC Pointer to the ASPC_Conf structure to configure. Must be non-NULL.
 * @param data  Pointer to an int16_t array containing at least 4 elements as described above.
 *
 * Side effects:
 * - Calls ASPC_SetMode, ASPC_SetStartVoltage, ASPC_SetVFinal and ASPC_setScanRate
 *   to apply individual settings.
 * - If _ASPC->V_ref is not equal to ASPC_3_3V_REF or ASPC_5V_REF, a message is printed
 *   and _ASPC->V_ref is set to 3300 (3.3 V expressed in millivolts).
 *
 * Notes:
 * - The function does not perform NULL-pointer checks; callers must ensure valid pointers.
 * - The meaning and valid ranges of "mode" and "scan rate" are defined by the ASPC API.
 * - Voltage values are expressed in millivolts.
 */
void ASPC_configure(ASPC_Conf *_ASPC,int16_t *data){
    ASPC_SetMode(_ASPC,*(data));
    ASPC_SetStartVoltage(_ASPC,*(data+1));
    ASPC_SetVFinal(_ASPC,*(data+2));
    ASPC_SetScanRate(_ASPC,*(data+3));
    
    if (_ASPC->V_ref!=ASPC_3_3V_REF && _ASPC->V_ref!=ASPC_5V_REF) {
        printf("Reference voltage is not set\n");
        printf("Setting to default 3.3V\n");
        _ASPC->V_ref=3300;
    }
    
    // _ASPC->rate=SAMPLERATE;
    // _ASPC->DAC_RES=DAC_RESOLUTION;
    // _ASPC->V_initial=_ASPC->V_ref;
    _ASPC->dac_sequence=ASPC_GetDACSequence(_ASPC);
}

void ASPC_deinit(ASPC_Conf *_ASPC){
	//free the dac sequence
	if (_ASPC->dac_sequence !=NULL) free(_ASPC->dac_sequence);

	//free the ASPC_Conf
	free(_ASPC);
    // _ASPC->dac_sequence=NULL;
    // _ASPC=NULL;
    
	return;
}

//Setters
// void ASPC_SetDACResolution(ASPC_Conf *_ASPC, uint8_t DACResolution){
//     _ASPC->DAC_RES=DACResolution;
//     return;
// }

void ASPC_SetScanRate(ASPC_Conf *_ASPC,uint16_t VScan){
    _ASPC->V_scanRate=VScan;
    return;
}

void ASPC_SetStartVoltage(ASPC_Conf *_ASPC,int16_t VStart){
    if (VStart>(_ASPC->V_initial)) _ASPC->V_initial=VStart;
    _ASPC->V_start=VStart;
    return;
}

void ASPC_SetVInit(ASPC_Conf *_ASPC,int16_t Vinit){
  if (Vinit>(_ASPC->V_ref)/2) {
      printf("Initial voltage is out of range of positive reference voltage\n");
      return;
  }
  else if (Vinit<(-(_ASPC->V_ref)/2)) {
  	printf("Initial voltage is out of range of negative reference voltage\n");
  	return;
  }
  //if the scan is increasing, the initial voltage should be less than the final voltage and more than the starting one
  else if ((_ASPC->V_start<_ASPC->V_final)){    
   if (Vinit>_ASPC->V_final || Vinit<_ASPC->V_start){
      printf("Initial voltage is out of range\n");
      printf("The initial voltage is lower than starting voltage or higher than final voltage\n");
      return;
    }
  }
  //if the scan is decreasing, the initial voltage should be more than the final voltage and less than the starting one
  else if (_ASPC->V_start>_ASPC->V_final){
      if (Vinit<_ASPC->V_final || Vinit>_ASPC->V_start){
      printf("Initial voltage is out of range\n");
      printf("The initial voltage is higher than starting voltage or lower than final voltage\n");
      return;
      }
  }
  else {
    printf("The initial voltage cannot be set\n");
    return;
  }
  _ASPC->V_initial=Vinit;
  _ASPC->dac_sequence=ASPC_GetDACSequence(_ASPC);
  printf("The inital voltage has been successfully. set initial voltage:%d\n",Vinit);
  return;
    
}

void ASPC_SetVFinal(ASPC_Conf *_ASPC,int16_t Vfinal){
    _ASPC->V_final=Vfinal;
    return;
}

void ASPC_SetReferenceVoltage(ASPC_Conf *_ASPC,uint16_t Vref){
/* set the reference voltage for the DAC value generation
    @param
    ASPC_Conf *: pointer to store the data in the structs
    vRef: Desired reference voltage, in mV
    
    @return None
*/
    //set V_initial as V_ref if it is the same as V_initial
    if (_ASPC->V_initial==_ASPC->V_ref) {
        _ASPC->V_initial=Vref;    
    }
    _ASPC->V_ref=Vref;
    if (_ASPC->dac_sequence!=NULL) {
        free(_ASPC->dac_sequence);
        _ASPC->dac_sequence=ASPC_GetDACSequence(_ASPC);
    }
    return;
}

void ASPC_SetDACResolution(ASPC_Conf *_ASPC,uint8_t _DAC_RES){
    _ASPC->DAC_RES=_DAC_RES;
    //update the DAC sequence following the resolution change
    if (_ASPC->dac_sequence!=NULL) {
        free(_ASPC->dac_sequence);
        _ASPC->dac_sequence=ASPC_GetDACSequence(_ASPC);
    }
    return;
}

void ASPC_SetMode(ASPC_Conf *_ASPC, uint8_t mode){
    //set the mode of the potentiostat
    //1 for linear sweep voltammetry
    //2 for cyclic voltammetry
    //3 for chronoamperometry
    _ASPC->mode=mode;
    if (_ASPC->dac_sequence!=NULL) {
        free(_ASPC->dac_sequence);
        _ASPC->dac_sequence=ASPC_GetDACSequence(_ASPC);
    }
    //update the DAC sequence following the mode change
    return;
}


void ASPC_SetSampleRate(ASPC_Conf *_ASPC,uint16_t sample_rate){
    if (sample_rate<0) {
        printf("Sample rate cannot be less than 0, setting up the default value\n");
        _ASPC->rate=SAMPLERATE;
        return;
    }
    _ASPC->rate=sample_rate;
    if (_ASPC->dac_sequence!=NULL) {
        free(_ASPC->dac_sequence);
        _ASPC->dac_sequence=ASPC_GetDACSequence(_ASPC);
    }
    return;
}

//getters
uint16_t ASPC_GetDACInitialVoltage(ASPC_Conf *_ASPC){
    uint16_t dacValue;
    uint16_t dacResValue = (uint16_t) 4095;
    float voltage=(_ASPC->V_ref/2)+_ASPC->V_start; 
    dacValue =(uint16_t) (voltage/_ASPC->V_ref*dacResValue);
    return dacValue;
}

uint16_t ASPC_GetDACVFinal(ASPC_Conf *_ASPC){
    uint16_t dacValue;
    uint16_t dacResValue = (uint16_t) 4095;
    float voltage=(_ASPC->V_ref/2)+_ASPC->V_final; 
    dacValue =(uint16_t) (voltage/_ASPC->V_ref*dacResValue);
    return dacValue;
}

uint16_t ASPC_VToDAC(uint16_t vRef, int16_t vTarget){
    uint16_t dacValue;
    //convert the voltage relative to the reference
    uint16_t voltage = (vRef/2) + vTarget;
    dacValue = (uint16_t) voltage*4095/vRef;
    return dacValue;
}

float ASPC_GetDACStepValue(ASPC_Conf *_ASPC){
    float stepValue;
    uint16_t dacResValue = (uint16_t) 4095;
    stepValue = (float) _ASPC->V_scanRate/(_ASPC->V_ref)*dacResValue/_ASPC->rate;
    return stepValue;
}

int16_t ASPC_DACtoVolt(ASPC_Conf *_ASPC,uint16_t DACValue){
    int16_t volt;
    uint16_t dacResValue = (uint16_t) 4095;
    volt = DACValue*(_ASPC->V_ref)/dacResValue;
    return volt;
}
void ASPC_GetDACs(ASPC_Conf *_ASPC){
    //get the DAC values for the start and final voltages
    if (_ASPC->dac_sequence!=NULL) free(_ASPC->dac_sequence);
    _ASPC->dac_sequence = ASPC_GetDACSequence(_ASPC);
    return;
}

uint16_t * ASPC_GetDACSequence(ASPC_Conf *_ASPC){
  /*generating array of dac values for potentiostat voltage sequences
  @params ASPC_Conf ASPC_Conf struct

  @return array pointer of dac sequences of values 
  */
  //setting up the multiplier
//   int multiplier=0;
//   if (_ASPC->V_start>=0) multiplier=1;
//   else multiplier=-1;

  //sanitary check
  if (_ASPC->dac_sequence!=NULL) {
    free(_ASPC->dac_sequence);
    _ASPC->dac_sequence=NULL;
  }
  uint16_t *dac_seq=NULL;
  uint16_t size;
    
  if (_ASPC->mode == LINEAR_SWEEP_VOLTAMMETRY) { //linear forward
    if (_ASPC->V_start==_ASPC->V_final) return NULL;
    return GenerateSequence(_ASPC,_ASPC->V_start,_ASPC->V_final,0);
  }
    
else if (_ASPC->mode==CYCLIC_VOLTAMMETRY){ 
  //if the initial voltage isn't set
  if (_ASPC->V_initial==_ASPC->V_ref || _ASPC->V_initial==_ASPC->V_start){
    dac_seq =  GenerateSequence(_ASPC,_ASPC->V_start,_ASPC->V_final,1);
  
  }
  else{
    uint16_t *seq1 = GenerateSequence(_ASPC,_ASPC->V_initial,_ASPC->V_final,1);
    size = _ASPC->_dac_size;
    uint16_t *seq2 = GenerateSequence(_ASPC,_ASPC->V_initial,_ASPC->V_start,1);
    
    //when sequence 1 and 2 are added, there is one same value in the start of sequence 2
    //the size is reduced by 1 to remove redundancy
    dac_seq = (uint16_t *) malloc((size-1+_ASPC->_dac_size)*sizeof(uint16_t));
    for (int i=0;i<size;i++){
        *(dac_seq+i)=*(seq1+i);
    }
    //the first value of seq2 is the same as the last value of seq1, skipped 1 increment.
    for (int i=1;i<_ASPC->_dac_size;i++){
        *(dac_seq+size+i-1)=*(seq2+i);
    }
    _ASPC->_dac_size+=size-1;
    free(seq1);
    free(seq2);
  }   
}

return dac_seq;
}

uint16_t *GenerateSequence(ASPC_Conf *_ASPC, int16_t V_start,int16_t V_final, uint8_t cyclic){
    uint16_t size;
    uint16_t *dac_seq = NULL;
    uint16_t mid;
    int8_t multiplier = 1;
    if ((V_start)>(V_final)) size = (uint16_t) ((V_start)-(V_final))*(_ASPC->rate)/(_ASPC->V_scanRate);
    else size = (uint16_t) (-V_start+V_final)*(_ASPC->rate)/(_ASPC->V_scanRate);
    if (cyclic) {
        size*=2;
        if (size%2!=0) {
            size++;
            mid=size/2;
            _ASPC->_dac_size=size+1;//yet to debug
            dac_seq = (uint16_t *) malloc((_ASPC->_dac_size)*sizeof(uint16_t));
            }

        else{
            size++;
            mid=size/2;
            _ASPC->_dac_size=size;
            dac_seq = (uint16_t *) malloc((_ASPC->_dac_size)*sizeof(uint16_t));
        }
    }
    else {
      mid=size;
      dac_seq = (uint16_t *) malloc((_ASPC->_dac_size)*sizeof(uint16_t));
      }
      _ASPC->_dac_size=size;
      printf("size:%hu\n",size);

      float val=ASPC_VToDAC(_ASPC->V_ref,V_start);
      float step_val = ASPC_GetDACStepValue(_ASPC);
      printf("Step value: %d\n",(uint16_t)step_val);
      //if Vstart <0 DAC value increases, then from 1 should be multiplied by -1 otherwise changed from -1 to reverse the increment.
      if (V_start>V_final ) multiplier*=-1;

      for (int i=0;i<mid;i++){
          *(dac_seq+i)=(uint16_t) val;
          if (cyclic) *(dac_seq+size-1-i)=*(dac_seq+i);
          val+=step_val*multiplier;
      } 
      if (cyclic) *(dac_seq+mid)=ASPC_VToDAC(_ASPC->V_ref,V_final);
      else *(dac_seq+size-1)=ASPC_VToDAC(_ASPC->V_ref,V_final);
    
    return dac_seq;
}
/*
 get_current_value
 @params ASPC_Conf *_ASPC
 @params uint16_t Rval Resistor value in the Transimpedance Amplifier
 @params int16_t adcValue adc measured value


@return current in mA
 */

float ASPC_GetCurrent(ASPC_Conf *_ASPC, uint16_t Rval, int16_t adcValue){
    //convert the adc value to the current value
	int value = adcValue - (_ASPC->V_ref/2);
	float current = -value/Rval;

	return current;
}

uint8_t isDAQEnabled(ASPC_Conf* _ASPC){
    return _ASPC->isDAQEnabled;
}

void enableDataAcquisition(ASPC_Conf* _ASPC){
    //enable data acquisition
    _ASPC->isDAQEnabled=1;

    if (_ASPC->buffer_curr!=NULL) free(_ASPC->buffer_curr);
    if (_ASPC->buffer_volt!=NULL) free(_ASPC->buffer_volt);
    _ASPC->buffer_curr=(int16_t *) malloc((_ASPC->_dac_size)*sizeof(int16_t));
    _ASPC->buffer_volt=(int16_t *) malloc((_ASPC->_dac_size)*sizeof(int16_t));
}

void disableDataAcquisition(ASPC_Conf* _ASPC){
    _ASPC->isDAQEnabled=0;
    if (_ASPC->buffer_curr!=NULL) free(_ASPC->buffer_curr);
    if (_ASPC->buffer_volt!=NULL) free(_ASPC->buffer_volt);
    _ASPC->buffer_curr=NULL;
    _ASPC->buffer_volt=NULL;
    
}

/*
get data*/
void get_raw_data(ASPC_Conf* _ASPC){
    if (_ASPC->isDAQEnabled==0) {
        printf("Data acquisition is not enabled\n");
        return;
    }
    if (_ASPC->buffer_curr==NULL || _ASPC->buffer_volt==NULL) {
        printf("Data buffers are not allocated\n");
        return;
    }
    for (int i=0;i<_ASPC->_dac_size;i++){
        printf("DAC Value: %d\t",*(_ASPC->dac_sequence+i));
        printf("adc_buffer: %d\n",*(_ASPC->buffer_volt+i));
        printf("curr_buffer: %d\n",*(_ASPC->buffer_curr+i));
    }
}

uint16_t VoltageToDAC(int16_t voltage,uint16_t vRef, uint16_t dacResolution){
    uint16_t dacVal;
    if (voltage>vRef/2 || (voltage<-vRef/2)) {
        printf("Currently using %d mV reference voltage\n",vRef);
        printf("Voltage is out of range\n");
        return 0;
    }
    dacVal = (uint16_t) (voltage+(vRef/2))*dacResolution/vRef;
    return dacVal;    
}

void ASPC_setReferenceMeasuredVoltage(ASPC_Conf* _ASPC, int16_t vRef){
    _ASPC->vRef=vRef;
    return;
}

void ASPC_setRTIA(ASPC_Conf* _ASPC, uint16_t RTIA){
    _ASPC->RTIA=RTIA;
    return;
}

int16_t ASPC_getRerefenceMeasuredVoltage(ASPC_Conf* _ASPC){
    return _ASPC->vRef;
}

uint16_t ASPC_getRTIA(ASPC_Conf* _ASPC){
    return _ASPC->RTIA;
}

void ASPC_ComputeCurrent(ASPC_Conf* _ASPC){
    if (_ASPC->isDAQEnabled==0) {
        printf("Data acquisition is not enabled\n");
        return;
    }
    if (_ASPC->buffer_curr==NULL || _ASPC->buffer_volt==NULL) {
        printf("Data buffers are not allocated\n");
        return;
    }
    if (_ASPC->RTIA==0) {
        printf("RTIA is not set\n");
        return;
    }
    for (int i=0;i<_ASPC->_dac_size;i++){
        *(_ASPC->buffer_curr+i)=ASPC_GetCurrent(_ASPC,_ASPC->RTIA,*(_ASPC->buffer_volt+i));
    }
    return;
}
