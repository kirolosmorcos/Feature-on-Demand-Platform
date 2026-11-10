################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cyclone_tcp/mqtt_sn/mqtt_sn_client.c \
../cyclone_tcp/mqtt_sn/mqtt_sn_client_message.c \
../cyclone_tcp/mqtt_sn/mqtt_sn_client_misc.c \
../cyclone_tcp/mqtt_sn/mqtt_sn_client_transport.c \
../cyclone_tcp/mqtt_sn/mqtt_sn_debug.c \
../cyclone_tcp/mqtt_sn/mqtt_sn_message.c 

OBJS += \
./cyclone_tcp/mqtt_sn/mqtt_sn_client.o \
./cyclone_tcp/mqtt_sn/mqtt_sn_client_message.o \
./cyclone_tcp/mqtt_sn/mqtt_sn_client_misc.o \
./cyclone_tcp/mqtt_sn/mqtt_sn_client_transport.o \
./cyclone_tcp/mqtt_sn/mqtt_sn_debug.o \
./cyclone_tcp/mqtt_sn/mqtt_sn_message.o 

C_DEPS += \
./cyclone_tcp/mqtt_sn/mqtt_sn_client.d \
./cyclone_tcp/mqtt_sn/mqtt_sn_client_message.d \
./cyclone_tcp/mqtt_sn/mqtt_sn_client_misc.d \
./cyclone_tcp/mqtt_sn/mqtt_sn_client_transport.d \
./cyclone_tcp/mqtt_sn/mqtt_sn_debug.d \
./cyclone_tcp/mqtt_sn/mqtt_sn_message.d 


# Each subdirectory must supply rules for building sources it contributes
cyclone_tcp/mqtt_sn/%.o: ../cyclone_tcp/mqtt_sn/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@cyclone_tcp/mqtt_sn/mqtt_sn_client.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


