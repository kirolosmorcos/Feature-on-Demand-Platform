################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../DoIP/DoIP.c 

OBJS += \
./DoIP/DoIP.o 

C_DEPS += \
./DoIP/DoIP.d 


# Each subdirectory must supply rules for building sources it contributes
DoIP/%.o: ../DoIP/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@DoIP/DoIP.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


