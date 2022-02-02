# SPI Bit Bang Drivers

Bit Banging drivers for SPI communication 

  
## Version History

Ver. |      Date      |     Comments     |
---  |      ---       |       ---        |
0.1	 |	  Jan 2022    |  In Development  | 					


## Contents

- spi_bitbang
    - Functions for reading and writing bytes to an SPI slave chip
    - Utilizes STM32 GPIO

## Usage 

- Clone this folder to Drivers/ in the STM32 IDE generated folder structure. 
- Add to project by going to Project -> Properties -> C/C++ General -> Paths and Symbols -> Includes
- Add `#include "spi_bitbang.h"` to main.c
- Make sure SPI and GPIO are set up (see `SPI_Init` in spi_bitbang.c and spi_bitbang.h for expected settings and pin assignments respectively)

## References 

### Documents

### Similar Drivers

## To-Do List

### Hardware 
- Continue validating driver functions
- Optimize functions
- Test speed (Arducam has a max clock speed of 8 MHz!)



