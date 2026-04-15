#ifndef ASPC_HPP
#define ASPC_HPP

#include <cstdint>
#include <cstdio>

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

class ASPC {
private:
    /* data all voltages are in mV*/
    uint16_t V_scanRate;
    uint16_t V_ref;
    int16_t V_initial; //initial voltage scan set the same as the V_ref to disable it.
    int16_t V_start; //start scan
    int16_t V_final; //final scan
    uint8_t DAC_RES;
    uint16_t rate;
    uint8_t mode;
    bool isDAQEnabledFlag; //flag to store data or not
    int16_t *buffer_volt; //buffer to store adc values
    int16_t *buffer_curr; //buffer to store adc values
    //private generated from get_dac_sequence
    uint16_t _dac_size;
    uint16_t *dac_sequence;
    int16_t vRef; //measured reference voltage
    uint16_t RTIA; //transimpedance amplifier resistor value
    volatile uint16_t indexDAC;


public:
    ASPC();
    ~ASPC() = default;
    ASPC(const ASPC&) = delete; // disable copy constructor

    void configure(int16_t *data);
    void deinit(); // Note: in C++ destructor handles deallocation

    // Setters
    void setScanRate(uint16_t VScan);
    void setStartVoltage(int16_t VStart);
    void setVInit(int16_t Vinit);
    void setVFinal(int16_t Vfinal);
    void setReferenceVoltage(uint16_t Vref);
    void setDACResolution(uint8_t _DAC_RES);
    void setMode(uint8_t mode);
    void setSampleRate(uint16_t sample_rate);

    // Getters
    uint16_t getDACInitialVoltage();
    uint16_t getDACVFinal();
    static uint16_t VToDAC(uint16_t vRef, int16_t vTarget);
    float getDACStepValue();
    int16_t DACtoVolt(uint16_t DACValue);
    uint16_t *getDACSequence();
    void getDACs();
    float getCurrent(uint16_t Rval, int16_t adcValue);
    void enableDataAcquisition();
    void disableDataAcquisition();
    bool isDAQEnabled() const;
    void getRawData();
    static uint16_t voltageToDAC(int16_t voltage, uint16_t vRef, uint16_t dacResolution);
    void setReferenceMeasuredVoltage(int16_t vRef);
    void setRTIA(uint16_t RTIA);
    void computeCurrent();
    void nextDAC(void);
    uint16_t getCurrentDAC(void);
    int16_t getReferenceMeasuredVoltage() const;
    uint16_t getRTIA() const;

private:
    uint16_t *generateSequence(int16_t V_start, int16_t V_final, bool cyclic);
};

#endif // ASPC_HPP