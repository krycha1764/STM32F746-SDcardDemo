################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../FATFS/Target/option/cc932.c \
../FATFS/Target/option/cc936.c \
../FATFS/Target/option/cc949.c \
../FATFS/Target/option/cc950.c \
../FATFS/Target/option/ccsbcs.c \
../FATFS/Target/option/syscall.c \
../FATFS/Target/option/unicode.c 

OBJS += \
./FATFS/Target/option/cc932.o \
./FATFS/Target/option/cc936.o \
./FATFS/Target/option/cc949.o \
./FATFS/Target/option/cc950.o \
./FATFS/Target/option/ccsbcs.o \
./FATFS/Target/option/syscall.o \
./FATFS/Target/option/unicode.o 

C_DEPS += \
./FATFS/Target/option/cc932.d \
./FATFS/Target/option/cc936.d \
./FATFS/Target/option/cc949.d \
./FATFS/Target/option/cc950.d \
./FATFS/Target/option/ccsbcs.d \
./FATFS/Target/option/syscall.d \
./FATFS/Target/option/unicode.d 


# Each subdirectory must supply rules for building sources it contributes
FATFS/Target/option/%.o FATFS/Target/option/%.su FATFS/Target/option/%.cyclo: ../FATFS/Target/option/%.c FATFS/Target/option/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F746xx -c -I../Core/Inc -I../Drivers/BSP/STM32746G-Discovery -I../FATFS/Target -I../FATFS/App -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FatFs/src -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I../Drivers/BSP/STM32746G-Discovery -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-FATFS-2f-Target-2f-option

clean-FATFS-2f-Target-2f-option:
	-$(RM) ./FATFS/Target/option/cc932.cyclo ./FATFS/Target/option/cc932.d ./FATFS/Target/option/cc932.o ./FATFS/Target/option/cc932.su ./FATFS/Target/option/cc936.cyclo ./FATFS/Target/option/cc936.d ./FATFS/Target/option/cc936.o ./FATFS/Target/option/cc936.su ./FATFS/Target/option/cc949.cyclo ./FATFS/Target/option/cc949.d ./FATFS/Target/option/cc949.o ./FATFS/Target/option/cc949.su ./FATFS/Target/option/cc950.cyclo ./FATFS/Target/option/cc950.d ./FATFS/Target/option/cc950.o ./FATFS/Target/option/cc950.su ./FATFS/Target/option/ccsbcs.cyclo ./FATFS/Target/option/ccsbcs.d ./FATFS/Target/option/ccsbcs.o ./FATFS/Target/option/ccsbcs.su ./FATFS/Target/option/syscall.cyclo ./FATFS/Target/option/syscall.d ./FATFS/Target/option/syscall.o ./FATFS/Target/option/syscall.su ./FATFS/Target/option/unicode.cyclo ./FATFS/Target/option/unicode.d ./FATFS/Target/option/unicode.o ./FATFS/Target/option/unicode.su

.PHONY: clean-FATFS-2f-Target-2f-option

