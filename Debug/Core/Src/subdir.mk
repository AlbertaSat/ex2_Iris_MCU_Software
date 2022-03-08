################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/I2C.c \
../Core/Src/arducam.c \
../Core/Src/cli.c \
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
../Core/Src/system_stm32l0xx.c 

OBJS += \
./Core/Src/I2C.o \
./Core/Src/arducam.o \
./Core/Src/cli.o \
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
./Core/Src/system_stm32l0xx.o 

C_DEPS += \
./Core/Src/I2C.d \
./Core/Src/arducam.d \
./Core/Src/cli.d \
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
./Core/Src/system_stm32l0xx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L071xx -c -I../Core/Inc -I../Drivers/STM32L0xx_HAL_Driver/Inc -I../Drivers/STM32L0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L0xx/Include -I../Drivers/CMSIS/Include -I../Core/SPI -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Core/Src/main.o: ../Core/Src/main.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L071xx -c -I../Core/Inc -I../Drivers/STM32L0xx_HAL_Driver/Inc -I../Drivers/STM32L0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L0xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/I2C.d ./Core/Src/I2C.o ./Core/Src/arducam.d ./Core/Src/arducam.o ./Core/Src/cli.d ./Core/Src/cli.o ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/nand_m79a.d ./Core/Src/nand_m79a.o ./Core/Src/nand_m79a_lld.d ./Core/Src/nand_m79a_lld.o ./Core/Src/nand_spi.d ./Core/Src/nand_spi.o ./Core/Src/printf.d ./Core/Src/printf.o ./Core/Src/spi_bitbang.d ./Core/Src/spi_bitbang.o ./Core/Src/stm32l0xx_hal_msp.d ./Core/Src/stm32l0xx_hal_msp.o ./Core/Src/stm32l0xx_it.d ./Core/Src/stm32l0xx_it.o ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/system_stm32l0xx.d ./Core/Src/system_stm32l0xx.o

.PHONY: clean-Core-2f-Src

