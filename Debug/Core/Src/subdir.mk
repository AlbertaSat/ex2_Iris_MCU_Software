################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/I2C.c \
../Core/Src/IEB_TESTS.c \
../Core/Src/SPI_IT.c \
../Core/Src/arducam.c \
../Core/Src/cli.c \
../Core/Src/command_handler.c \
../Core/Src/housekeeping.c \
../Core/Src/ina209.c \
../Core/Src/main.c \
../Core/Src/nand_m79a.c \
../Core/Src/nand_m79a_lld.c \
../Core/Src/nand_spi.c \
../Core/Src/printf.c \
../Core/Src/spi_bitbang.c \
../Core/Src/stm32l0xx_hal_msp.c \
../Core/Src/stm32l0xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32l0xx.c \
../Core/Src/tmp421.c 

OBJS += \
./Core/Src/I2C.o \
./Core/Src/IEB_TESTS.o \
./Core/Src/SPI_IT.o \
./Core/Src/arducam.o \
./Core/Src/cli.o \
./Core/Src/command_handler.o \
./Core/Src/housekeeping.o \
./Core/Src/ina209.o \
./Core/Src/main.o \
./Core/Src/nand_m79a.o \
./Core/Src/nand_m79a_lld.o \
./Core/Src/nand_spi.o \
./Core/Src/printf.o \
./Core/Src/spi_bitbang.o \
./Core/Src/stm32l0xx_hal_msp.o \
./Core/Src/stm32l0xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32l0xx.o \
./Core/Src/tmp421.o 

C_DEPS += \
./Core/Src/I2C.d \
./Core/Src/IEB_TESTS.d \
./Core/Src/SPI_IT.d \
./Core/Src/arducam.d \
./Core/Src/cli.d \
./Core/Src/command_handler.d \
./Core/Src/housekeeping.d \
./Core/Src/ina209.d \
./Core/Src/main.d \
./Core/Src/nand_m79a.d \
./Core/Src/nand_m79a_lld.d \
./Core/Src/nand_spi.d \
./Core/Src/printf.d \
./Core/Src/spi_bitbang.d \
./Core/Src/stm32l0xx_hal_msp.d \
./Core/Src/stm32l0xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32l0xx.d \
./Core/Src/tmp421.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L071xx -c -I../Core/Inc -I../Drivers/STM32L0xx_HAL_Driver/Inc -I../Drivers/STM32L0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L0xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/I2C.d ./Core/Src/I2C.o ./Core/Src/I2C.su ./Core/Src/IEB_TESTS.d ./Core/Src/IEB_TESTS.o ./Core/Src/IEB_TESTS.su ./Core/Src/SPI_IT.d ./Core/Src/SPI_IT.o ./Core/Src/SPI_IT.su ./Core/Src/arducam.d ./Core/Src/arducam.o ./Core/Src/arducam.su ./Core/Src/cli.d ./Core/Src/cli.o ./Core/Src/cli.su ./Core/Src/command_handler.d ./Core/Src/command_handler.o ./Core/Src/command_handler.su ./Core/Src/housekeeping.d ./Core/Src/housekeeping.o ./Core/Src/housekeeping.su ./Core/Src/ina209.d ./Core/Src/ina209.o ./Core/Src/ina209.su ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/nand_m79a.d ./Core/Src/nand_m79a.o ./Core/Src/nand_m79a.su ./Core/Src/nand_m79a_lld.d ./Core/Src/nand_m79a_lld.o ./Core/Src/nand_m79a_lld.su ./Core/Src/nand_spi.d ./Core/Src/nand_spi.o ./Core/Src/nand_spi.su ./Core/Src/printf.d ./Core/Src/printf.o ./Core/Src/printf.su ./Core/Src/spi_bitbang.d ./Core/Src/spi_bitbang.o ./Core/Src/spi_bitbang.su ./Core/Src/stm32l0xx_hal_msp.d ./Core/Src/stm32l0xx_hal_msp.o ./Core/Src/stm32l0xx_hal_msp.su ./Core/Src/stm32l0xx_it.d ./Core/Src/stm32l0xx_it.o ./Core/Src/stm32l0xx_it.su ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32l0xx.d ./Core/Src/system_stm32l0xx.o ./Core/Src/system_stm32l0xx.su ./Core/Src/tmp421.d ./Core/Src/tmp421.o ./Core/Src/tmp421.su

.PHONY: clean-Core-2f-Src

