################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cyclone_tcp/echo/echo_server.c \
../cyclone_tcp/echo/echo_server_misc.c 

OBJS += \
./cyclone_tcp/echo/echo_server.o \
./cyclone_tcp/echo/echo_server_misc.o 

C_DEPS += \
./cyclone_tcp/echo/echo_server.d \
./cyclone_tcp/echo/echo_server_misc.d 


# Each subdirectory must supply rules for building sources it contributes
cyclone_tcp/echo/%.o: ../cyclone_tcp/echo/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@cyclone_tcp/echo/echo_server.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


