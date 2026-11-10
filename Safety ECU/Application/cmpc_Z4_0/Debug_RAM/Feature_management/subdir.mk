################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Feature_management/feature_management.c 

OBJS += \
./Feature_management/feature_management.o 

C_DEPS += \
./Feature_management/feature_management.d 


# Each subdirectory must supply rules for building sources it contributes
Feature_management/%.o: ../Feature_management/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@Feature_management/feature_management.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


