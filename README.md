# EX2_IRIS_MCU_SOFTWARE 
Welcome 👋 - this is the central repository for the Ex-Alta 2 Iris Payload. 

Separate folders contain software submodules for the Equipment Handlers (EH), Hardware Interface (HAL) and On-board services.

### File Structure
* Core/
	* Filesystem/
		* Software pertaining to the NAND flash, both Low Level Drivers (LLD) and Hardware Interface code.
	* Inc/
		* Header files for HAL / EH.
	* Src/
		* Source files for HAL / EH. Main entry point. 
		
* Drivers/
	* STM32CubeMX generated source files including hardware drivers, and configurations

## Getting Started
1. Set up an SSH key with GitHub. [Instructions](https://docs.github.com/en/github/authenticating-to-github/connecting-to-github-with-ssh/adding-a-new-ssh-key-to-your-github-account)

2. Clone this repository.

3. [Download](https://www.st.com/en/development-tools/stm32cubeide.html) STM32CubeIDE Version 1.9.0 

4. Import this code as a STM32CubeIDE Project:
	Select `File > Import`
	Select `General` =>  `Existing Projects into Workspace`
	Then browse to this repository, and select the discovered project
	Select `Finish`

5. You  can now build and debug, and flash this project to Iris with a ST-LINKV/2 Probe!!

## Software Architecture
Iris software uses a bare-metal design involving a global loop, defined in `main.c`, executing infinitely. Inside
of the loop there is a finite state machine that is responsible for listening and handling commands coming from the
OBC; 4 states: IDLE, LISTENING, HANDLE_COMMAND, FINISH.

The documentation and design of the software is very much self-explanatory, feel free to examine the inner working. From
the prespective of an operator or user, there should not be any hands-on interaction with Iris software. Instead, all
commands are meant to be initiatied from the ground station

## Hardware Architecture
Iris (STM32 MCU) hardware is configured via the `.ioc` file. All configurations have been tested and prepared for flight
usage. Modification of system and/or peripherals settings may lead to unwanted behaviour and should be avoided. 

The compiler is optimized using `-O3` flag. The increase performance has enable to improve image transfer speeds. Thus,
changing this will affect pre-determined delays between Iris and OBC

## Configuring
To configure the repo for the version of Iris you are using, edit the file `/Core/Inc/iris_system.h`
For flight assembly, use the following configuration:  
	* `#define SPI_HANDLER`  
	* `#define IRIS_FM`  
For debugging purpose, use the following configuration:
	* `define DEBUG_OUTPUT`
The rest will be handled by the software. If not for flight, use the pertinent combination of defines for your specific use case.

## Creating Iris binaries
To create an Iris binary (`.bin`) file, go to STM32CubeIDE, open project and build project. You will find the binary file
on the following path `ex2_Iris_MCU_Software/Debug/ex2_Iris_MCU_Software.bin`

### Using Iris binary file to perform firmware update
Here are the steps to perform firmware updates on Iris during flight:
1. Build project to create binary file
2. Locate binary file
3. Ensure that the OBC and ground station are up-to-date and working
4. Start the OBC
5. Using `ftp` upload the binary file to OBC's sd card. Please ensure the file is stored under the follwing name `ex2_Iris_MCU_Software.bin`
6. Using `cli`, write Iris firmware update command (`ex2.iris.iris_program_flash`) to start firmware update

## Contributing
* Branches
	* Branches must change  or implement one feature
	* Branches created from branches must be merged in the order they are created
* Branch Naming
	* Branches are named as follows: <name_of_author>/<branch_type>/<description_of_branch>
	* Branch types may be one of the following: {hotfix,bugfix,experimental,feature} 
* Code style
	* Code should be formatted with clang using settings in .clang-format
