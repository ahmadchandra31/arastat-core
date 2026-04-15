/*
ARASTAT POTENTIOSTAT CORE
Written by arachanx21
ARA APPLIED 2023

Converted to C++ by GitHub Copilot

This software is licensed by ARA APPLIED through BSD-3 License. ARA APPLIED provides the code AS IS AND
is not RESPONSIBLE for any DAMAGES incurred from the use of the code.  All text here must be included in any distribution
You may copy, modify,redistribute directly or in derivatives as long as it's in compliance with the license.
*/
#include "ASPC.hpp"
#include <algorithm>
#include <iostream>

volatile uint16_t DACIndex = 0;

ASPC::ASPC() {
    // default configuration for the ASPC
    DAC_RES = 12;
#ifdef USE_5V
    V_ref = ASPC_5V_REF;
#else
    V_ref = ASPC_3_3V_REF;
#endif
    V_start = -600;
    // V_initial set the same as reference to disable the initial voltage
    V_initial = V_start;
    V_scanRate = 100;
    mode = CYCLIC_VOLTAMMETRY;
    rate = SAMPLERATE;
    vRef = V_ref / 2;
    // dac_sequence will be generated when needed
    dac_sequence = nullptr;
    buffer_volt = nullptr;
    buffer_curr = nullptr;
    RTIA = 1000; // default RTIA value, can be changed by the user
}

void ASPC::configure(int16_t *data) {
    setMode(data[0]);
    setStartVoltage(data[1]);
    setVFinal(data[2]);
    setScanRate(data[3]);

    if (V_ref != ASPC_3_3V_REF && V_ref != ASPC_5V_REF) {
        std::cout << "Reference voltage is not set" << std::endl;
        std::cout << "Setting to default 3.3V" << std::endl;
        V_ref = 3300;
    }

    getDACs();
}

void ASPC::deinit() {
    // In C++, destructor handles cleanup, but if needed, clear vectors
    free((void *)dac_sequence);
    dac_sequence = nullptr;
    free((void *)buffer_volt);
    buffer_volt = nullptr;
    free((void *)buffer_curr);
    buffer_curr = nullptr;
}

// Setters
void ASPC::setScanRate(uint16_t VScan) {
    V_scanRate = VScan;
}

void ASPC::setStartVoltage(int16_t VStart) {
    if (VStart > V_initial) V_initial = VStart;
    V_start = VStart;
}

void ASPC::setVInit(int16_t Vinit) {
    if (Vinit > (V_ref) / 2) {
        std::cout << "Initial voltage is out of range of positive reference voltage" << std::endl;
        return;
    } else if (Vinit < (-(V_ref) / 2)) {
        std::cout << "Initial voltage is out of range of negative reference voltage" << std::endl;
        return;
    }
    // if the scan is increasing, the initial voltage should be less than the final voltage and more than the starting one
    else if ((V_start < V_final)) {
        if (Vinit > V_final || Vinit < V_start) {
            std::cout << "Initial voltage is out of range" << std::endl;
            std::cout << "The initial voltage is lower than starting voltage or higher than final voltage" << std::endl;
            return;
        }
    }
    // if the scan is decreasing, the initial voltage should be more than the final voltage and less than the starting one
    else if (V_start > V_final) {
        if (Vinit < V_final || Vinit > V_start) {
            std::cout << "Initial voltage is out of range" << std::endl;
            std::cout << "The initial voltage is higher than starting voltage or lower than final voltage" << std::endl;
            return;
        }
    } else {
        std::cout << "The initial voltage cannot be set" << std::endl;
        return;
    }
    V_initial = Vinit;
    getDACs();
    std::cout << "The initial voltage has been successfully set. Initial voltage: " << Vinit << std::endl;
}

void ASPC::setVFinal(int16_t Vfinal) {
    V_final = Vfinal;
}

void ASPC::setReferenceVoltage(uint16_t Vref) {
    /* set the reference voltage for the DAC value generation
        @param
        vRef: Desired reference voltage, in mV

        @return None
    */
    // set V_initial as V_ref if it is the same as V_initial
    if (V_initial == V_ref) {
        V_initial = Vref;
    }
    V_ref = Vref;
    if (dac_sequence != nullptr) {
        getDACs();
    }
}

void ASPC::setDACResolution(uint8_t _DAC_RES) {
    DAC_RES = _DAC_RES;
    // update the DAC sequence following the resolution change
    if (dac_sequence != nullptr) {
        getDACs();
    }
}

void ASPC::setMode(uint8_t mode) {
    // set the mode of the potentiostat
    this->mode = mode;
    if (dac_sequence != nullptr) {
        getDACs();
    }
}

void ASPC::setSampleRate(uint16_t sample_rate) {
    if (sample_rate < 0) {
        std::cout << "Sample rate cannot be less than 0, setting up the default value" << std::endl;
        rate = SAMPLERATE;
        return;
    }
    rate = sample_rate;
    if (dac_sequence != nullptr) {
        getDACs();
    }
}

// Getters
uint16_t ASPC::getDACInitialVoltage() {
    uint16_t dacValue;
    uint16_t dacResValue = 4095;
    float voltage = (V_ref / 2) + V_start;
    dacValue = (uint16_t)(voltage / V_ref * dacResValue);
    return dacValue;
}

uint16_t ASPC::getDACVFinal() {
    uint16_t dacValue;
    uint16_t dacResValue = 4095;
    float voltage = (V_ref / 2) + V_final;
    dacValue = (uint16_t)(voltage / V_ref * dacResValue);
    return dacValue;
}

uint16_t ASPC::VToDAC(uint16_t vRef, int16_t vTarget) {
    uint16_t dacValue;
    // convert the voltage relative to the reference
    uint16_t voltage = (vRef / 2) + vTarget;
    dacValue = (uint16_t)voltage * 4095 / vRef;
    return dacValue;
}

float ASPC::getDACStepValue() {
    float stepValue;
    uint16_t dacResValue = 4095;
    stepValue = (float)V_scanRate / V_ref * dacResValue / rate;
    return stepValue;
}

int16_t ASPC::DACtoVolt(uint16_t DACValue) {
    int16_t volt;
    uint16_t dacResValue = 4095;
    volt = DACValue * (V_ref) / dacResValue;
    return volt;
}

void ASPC::getDACs() {
    // get the DAC values for the start and final voltages
    dac_sequence = getDACSequence();
}

    uint16_t *ASPC::getDACSequence() {
    if (dac_sequence!=nullptr) {
    free(dac_sequence);
    dac_sequence=nullptr;
  }
  uint16_t *dac_seq=nullptr;
  uint16_t size;
    
  if (mode == LINEAR_SWEEP_VOLTAMMETRY) { //linear forward
    if (V_start==V_final) return nullptr;
    return generateSequence(V_start,V_final,0);
  }
    
else if (mode==CYCLIC_VOLTAMMETRY){ 
  //if the initial voltage isn't set
  if (V_initial==V_ref || V_initial==V_start){
    dac_seq =  generateSequence(V_start,V_final,1);
  
  }
  else{
    uint16_t *seq1 = generateSequence(V_initial,V_final,1);
    size = _dac_size;
    uint16_t *seq2 = generateSequence(V_initial,V_start,1);
    
    //when sequence 1 and 2 are added, there is one same value in the start of sequence 2
    //the size is reduced by 1 to remove redundancy
    dac_seq = (uint16_t *) malloc((size-1+_dac_size)*sizeof(uint16_t));
    for (int i=0;i<size;i++){
        *(dac_seq+i)=*(seq1+i);
    }
    //the first value of seq2 is the same as the last value of seq1, skipped 1 increment.
    for (int i=1;i<_dac_size;i++){
        *(dac_seq+size+i-1)=*(seq2+i);
    }
    _dac_size+=size-1;
    free(seq1);
    seq1=nullptr;
    free(seq2);
    seq2=nullptr;
  }   
}

return dac_seq;
}

uint16_t *ASPC::generateSequence(int16_t V_start, int16_t V_final, bool cyclic) {
    uint16_t size;
    uint16_t *dac_seq = nullptr;
    uint16_t mid;
    int8_t multiplier = 1;
    if ((V_start)>(V_final)) size = (uint16_t) ((V_start)-(V_final))*(rate)/(V_scanRate);
    else size = (uint16_t) (-V_start+V_final)*(rate)/(V_scanRate);
    if (cyclic) {
        size*=2;
        if (size%2!=0) {
            size++;
            mid=size/2;
            // _ASPC->_dac_size=size+1;//yet to debug
            _dac_size = size + 1;
            dac_seq = (uint16_t *) malloc((_dac_size)*sizeof(uint16_t));
            }

        else{
            size++;
            mid=size/2;
            _dac_size = size;
            dac_seq = (uint16_t *) malloc((_dac_size)*sizeof(uint16_t));
        }
    }
    else {
      mid=size;
      dac_seq = (uint16_t *) malloc((_dac_size)*sizeof(uint16_t));
      }
      _dac_size=size;
      printf("size:%hu\n",size);

      float val=VToDAC(V_ref,V_start);
      float step_val = getDACStepValue();
      printf("Step value: %d\n",(uint16_t)step_val);
      //if Vstart <0 DAC value increases, then from 1 should be multiplied by -1 otherwise changed from -1 to reverse the increment.
      if (V_start>V_final ) multiplier*=-1;

      for (int i=0;i<mid;i++){
          *(dac_seq+i)=(uint16_t) val;
          if (cyclic) *(dac_seq+size-1-i)=*(dac_seq+i);
          val+=step_val*multiplier;
      } 
      if (cyclic) *(dac_seq+mid)=VToDAC(V_ref,V_final);
      else *(dac_seq+size-1)=VToDAC(V_ref,V_final);
    
    return dac_seq;
}

/*
 get_current_value
 @params uint16_t Rval Resistor value in the Transimpedance Amplifier
 @params int16_t adcValue adc measured value

@return current in mA
 */

float ASPC::getCurrent(uint16_t Rval, int16_t adcValue) {
    // convert the adc value to the current value
    int value = adcValue - (V_ref / 2);
    float current = -value / (float)Rval;
    return current;
}

bool ASPC::isDAQEnabled() const {
    return isDAQEnabledFlag;
}

void ASPC::enableDataAcquisition() {
    // enable data acquisition
    isDAQEnabledFlag = true;
    buffer_volt = (int16_t *)malloc(_dac_size * sizeof(int16_t));
    buffer_curr = (int16_t *)malloc(_dac_size * sizeof(int16_t));
}

void ASPC::disableDataAcquisition() {
    isDAQEnabledFlag = false;
    free((void *)buffer_curr);
    buffer_curr = nullptr;
    free((void *)buffer_volt);
    buffer_volt = nullptr;
}

/*
get data*/
void ASPC::getRawData() {
    if (!isDAQEnabled()) {
        std::cout << "Data acquisition is not enabled" << std::endl;
        return;
    }
    if (buffer_curr == nullptr|| buffer_volt == nullptr) {
        std::cout << "Data buffers are not allocated" << std::endl;
        return;
    }
    for (size_t i = 0; i < _dac_size; ++i) {
        std::cout << "DAC Value: " << dac_sequence[i] << "\t";
        std::cout << "adc_buffer: " << buffer_volt[i] << std::endl;
        std::cout << "curr_buffer: " << buffer_curr[i] << std::endl;
    }
}

uint16_t ASPC::voltageToDAC(int16_t voltage, uint16_t vRef, uint16_t dacResolution) {
    uint16_t dacVal;
    if (voltage > vRef / 2 || (voltage < -vRef / 2)) {
        std::cout << "Currently using " << vRef << " mV reference voltage" << std::endl;
        std::cout << "Voltage is out of range" << std::endl;
        return 0;
    }
    dacVal = (uint16_t)(voltage + (vRef / 2)) * dacResolution / vRef;
    return dacVal;
}

void ASPC::setReferenceMeasuredVoltage(int16_t vRef) {
    this->vRef = vRef;
}

void ASPC::setRTIA(uint16_t RTIA) {
    this->RTIA = RTIA;
}

int16_t ASPC::getReferenceMeasuredVoltage() const {
    return vRef;
}

uint16_t ASPC::getRTIA() const {
    return RTIA;
}

void ASPC::computeCurrent() {
    if (!isDAQEnabled()) {
        std::cout << "Data acquisition is not enabled" << std::endl;
        return;
    }
    if (buffer_curr == nullptr || buffer_volt == nullptr) {
        std::cout << "Data buffers are not allocated" << std::endl;
        return;
    }
    if (RTIA == 0) {
        std::cout << "RTIA is not set" << std::endl;
        return;
    }
    for (size_t i = 0; i < _dac_size; ++i) {
        buffer_curr[i] = getCurrent(RTIA, buffer_volt[i]);
    }
}