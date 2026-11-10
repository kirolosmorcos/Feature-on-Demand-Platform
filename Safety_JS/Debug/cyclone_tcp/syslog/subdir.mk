################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cyclone_tcp/syslog/syslog_client.c \
../cyclone_tcp/syslog/syslog_client_misc.c 

OBJS += \
./cyclone_tcp/syslog/syslog_client.o \
./cyclone_tcp/syslog/syslog_client_misc.o 

C_DEPS += \
./cyclone_tcp/syslog/syslog_client.d \
./cyclone_tcp/syslog/syslog_client_misc.d 


# Each subdirectory must supply rules for building sources it contributes
cyclone_tcp/syslog/%.o: ../cyclone_tcp/syslog/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@cyclone_tcp/syslog/syslog_client.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


