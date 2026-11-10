################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../BlindSpot/bsd_receiver_task.c 

OBJS += \
./BlindSpot/bsd_receiver_task.o 

C_DEPS += \
./BlindSpot/bsd_receiver_task.d 


# Each subdirectory must supply rules for building sources it contributes
BlindSpot/%.o: ../BlindSpot/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@BlindSpot/bsd_receiver_task.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


