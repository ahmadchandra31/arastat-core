# arastat-core

**ARA Applied Potentiostat Core** is a C++ library designed to generate DAC values for electrochemical potentiostat operations. It provides a standardized API for generating waveforms for Cyclic Voltammetry (CV) and Linear Sweep Voltammetry (LSV).

The library is architected for potentiostat designs utilizing a differential amplifier topology, where the DAC output is subtracted from a reference voltage to achieve the desired cell potential.

ARAStat Schematics (Simulated):
<img width="1171" alt="Screenshot 2025-01-21 at 18 09 09" src="https://github.com/user-attachments/assets/0607d480-1aae-4664-af6a-07dc5bb2d724" />

## Features

- **Waveform Generation**: High-precision signal generation for CV and LSV.
- **Hardware Optimized**: Specifically designed for subtracting/differential amplifier stages.
- **Bipolar Support**: Logic designed to allow bipolar sweeps using common unipolar power rails (3.3V/5V).
- **Lightweight**: Minimal dependencies, ideal for embedded systems or high-level simulations.

## Building the Library

Build the static library using CMake:

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

Include `ASPC.hpp` in your project and link against the `ASPC` library.

Example:

```cpp
#include "ASPC.hpp"

int main() {
    ASPC potentiostat;

    // Configuration array: [Mode, Start Voltage (mV), End Voltage (mV), Scan Rate (mV/s)]
    int16_t config[] = {CYCLIC_VOLTAMMETRY, -600, 600, 100};
    potentiostat.configure(config);
    
    //setting up DAC generation rate (e.g 10 DAC per second)
    potentiostat.setSampleRate(10);

    // The library calculates the necessary DAC steps based on your configuration.

    //obtaining the dac value
    //can be used during timer ISR when setting up the DAC
    uint16_t dac = potentiostat.getIndexDAC()  
    //setting up the next value
    aspc.nextDAC();


    return 0;
}
```

Here are some references we use to design and develop our potentiostat:

Crespo, J. R., Elliott, S. R., Hutter, T., & Águas, H. (2021). Development of a low-cost Arduino-based potentiostat. <a href=https://repositories.lib.utexas.edu/server/api/core/bitstreams/3e70ae06-19ce-4752-a574-c1441e6bb971/content>(link)</a><br>
Meloni, G. N. (2016). Building a microcontroller based potentiostat: A inexpensive and versatile platform for teaching electrochemistry and instrumentation. <a href=https://pubs.acs.org/doi/10.1021/acs.jchemed.5b00961>(link)</a><br>
Bio-Logic Science Instruments.(2005).The mystery of potentiostat stability explained <a href=https://www.biologic.net/wp-content/uploads/2019/08/battery-potentiostat_electrochemistry-an4.pdf>(link)</a>

Our design subtracts the DAC signal voltage with half of the reference voltage such that we can use available fixed voltages such as 5V and 3.3V.

Feel free to use or modify it depending on your case.
