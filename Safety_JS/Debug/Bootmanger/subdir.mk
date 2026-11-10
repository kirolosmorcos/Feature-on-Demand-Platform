################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Bootmanger/Bootmanager.c 

OBJS += \
./Bootmanger/Bootmanager.o 

C_DEPS += \
./Bootmanger/Bootmanager.d 


# Each subdirectory must supply rules for building sources it contributes
Bootmanger/%.o: ../Bootmanger/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@Bootmanger/Bootmanager.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


