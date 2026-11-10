################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cyclone_tcp/icecast/icecast_client.c 

OBJS += \
./cyclone_tcp/icecast/icecast_client.o 

C_DEPS += \
./cyclone_tcp/icecast/icecast_client.d 


# Each subdirectory must supply rules for building sources it contributes
cyclone_tcp/icecast/%.o: ../cyclone_tcp/icecast/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@cyclone_tcp/icecast/icecast_client.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


