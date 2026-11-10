################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../freertos/portable/common/mpu_wrappers.c 

OBJS += \
./freertos/portable/common/mpu_wrappers.o 

C_DEPS += \
./freertos/portable/common/mpu_wrappers.d 


# Each subdirectory must supply rules for building sources it contributes
freertos/portable/common/%.o: ../freertos/portable/common/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@freertos/portable/common/mpu_wrappers.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


