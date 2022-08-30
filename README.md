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

6. Import this code as a STM32CubeIDE Project:
	Select `File > Import`
	Select `General` =>  `Existing Projects into Workspace`
	Then browse to this repository, and select the discovered project
	Select `Finish`

7. You  can now build and debug, and flash this project to Iris with a ST-LINKV/2 Probe!!

## Configuring
To configure the repo for the version of Iris you are using, edit the file `/Core/Inc/iris_system.h`
For flight assembly, use the following configuration:
\n	* `#define SPI_HANDLER`
\n	* `#define IRIS_FM`
The rest will be handled by the software. If not for flight, use the pertinent combination of defines for your specific use case.
## Contributing

* Branches
	* Branches must change  or implement one feature
	* Branches created from branches must be merged in the order they are created
* Branch Naming
	* Branches are named as follows: <name_of_author>/<branch_type>/<description_of_branch>
	* Branch types may be one of the following: {hotfix,bugfix,experimental,feature} 
* Code style
	* Code should be formatted with clang using settings in .clang-format
