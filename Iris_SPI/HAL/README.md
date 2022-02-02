# SPI STM32l0xx HAL Drivers

Drivers for SPI communication 
- Dependencies
  - STM32 L0 Series Hardware Abstraction Library (HAL) 

  
## Version History

Ver. |      Date      |     Comments     |
---  |      ---       |       ---        |
0.1	 |	  Jan 2022    |  In Development  | 					


## Contents

- hal_spi
    - Functions for reading and writing bytes to an SPI slave chip
    - Utilizes STM32L0xx HAL library

## Usage 

- Clone this folder to Drivers/ in the STM32 IDE generated folder structure. 
- Add to project by going to Project -> Properties -> C/C++ General -> Paths and Symbols -> Includes
- Add `#include "hal_spi.h"` to main.c
- Make sure SPI and GPIO are set up (see `SPI_Init` in hal_spi.c and hal_spi.h for expected settings and pin assignments (CS!))

## References 

### Documents

### Similar Drivers
- 
## To-Do List

### Hardware 
- Validate driver functions
- Optimize functions
- Test speed (Arducam has a max clock speed of 8 MHz!)


### Higher level features
- Assuming we can use HAL SPI libraries for a USART-bus SPI interface
    - Function wrapper to take image and return successful once image has been captured
    - Function wrapper to request image and do something(?) with it

