################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cyclone_tcp/ntp/ntp_common.c \
../cyclone_tcp/ntp/ntp_debug.c 

OBJS += \
./cyclone_tcp/ntp/ntp_common.o \
./cyclone_tcp/ntp/ntp_debug.o 

C_DEPS += \
./cyclone_tcp/ntp/ntp_common.d \
./cyclone_tcp/ntp/ntp_debug.d 


# Each subdirectory must supply rules for building sources it contributes
cyclone_tcp/ntp/%.o: ../cyclone_tcp/ntp/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@cyclone_tcp/ntp/ntp_common.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


