################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Bootloader/Bootloader.c \
../Bootloader/shared_state.c 

OBJS += \
./Bootloader/Bootloader.o \
./Bootloader/shared_state.o 

C_DEPS += \
./Bootloader/Bootloader.d \
./Bootloader/shared_state.d 


# Each subdirectory must supply rules for building sources it contributes
Bootloader/%.o: ../Bootloader/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@Bootloader/Bootloader.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


