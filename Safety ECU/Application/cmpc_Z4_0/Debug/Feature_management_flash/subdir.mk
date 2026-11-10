################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Feature_management_flash/feature_management_flash.c 

OBJS += \
./Feature_management_flash/feature_management_flash.o 

C_DEPS += \
./Feature_management_flash/feature_management_flash.d 


# Each subdirectory must supply rules for building sources it contributes
Feature_management_flash/%.o: ../Feature_management_flash/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@Feature_management_flash/feature_management_flash.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


