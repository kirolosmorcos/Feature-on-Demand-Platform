################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../UDS/ResetWrapper.c \
../UDS/UDS.c 

OBJS += \
./UDS/ResetWrapper.o \
./UDS/UDS.o 

C_DEPS += \
./UDS/ResetWrapper.d \
./UDS/UDS.d 


# Each subdirectory must supply rules for building sources it contributes
UDS/%.o: ../UDS/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@UDS/ResetWrapper.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


