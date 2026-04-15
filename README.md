# arastat-core (under development)
ARA Applied Potentiostat Core. A C++ library to generate DAC values for potentiostat operations. This library provides an API to generate DAC signals for cyclic voltammetry and LSV in potentiostats. Our potentiostat design utilizes a differential amplifier where the voltage applied from the digital-to-analog converter is subtracted with a reference voltage in the subtracting amplifier.

ARAStat Schematics (Simulated):
<img width="1171" alt="Screenshot 2025-01-21 at 18 09 09" src="https://github.com/user-attachments/assets/0607d480-1aae-4664-af6a-07dc5bb2d724" />

## Building the Library

This project uses CMake to build the static library.

```bash
mkdir build
cd build
cmake ..
make
```

## Usage (Deprecated Updates soon :D)

Include `ASPC.hpp` in your project and link against the `ASPC` library.

Example:

```cpp
#include "ASPC.hpp"

int main() {
    ASPC potentiostat;
    std::vector<int16_t> config = {static_cast<int16_t>(ASPC_Mode::CYCLIC_VOLTAMMETRY), -600, 600, 100};
    potentiostat.configure(config);
    // Use the potentiostat...
    return 0;
}
```

Here are some references we use to design and develop our potentiostat:

Crespo, J. R., Elliott, S. R., Hutter, T., & Águas, H. (2021). Development of a low-cost Arduino-based potentiostat. <a href=https://repositories.lib.utexas.edu/server/api/core/bitstreams/3e70ae06-19ce-4752-a574-c1441e6bb971/content>(link)</a><br>
Meloni, G. N. (2016). Building a microcontroller based potentiostat: A inexpensive and versatile platform for teaching electrochemistry and instrumentation. <a href=https://pubs.acs.org/doi/10.1021/acs.jchemed.5b00961>(link)</a><br>
Bio-Logic Science Instruments.(2005).The mystery of potentiostat stability explained <a href=https://www.biologic.net/wp-content/uploads/2019/08/battery-potentiostat_electrochemistry-an4.pdf>(link)</a>

Our design subtracts the DAC signal voltage with half of the reference voltage such that we can use available fixed voltages such as 5V and 3.3V.

Feel free to use or modify it depending on your case.
