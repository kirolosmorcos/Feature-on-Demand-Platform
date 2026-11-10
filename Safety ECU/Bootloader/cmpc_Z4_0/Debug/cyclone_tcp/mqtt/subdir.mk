################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cyclone_tcp/mqtt/mqtt_client.c \
../cyclone_tcp/mqtt/mqtt_client_misc.c \
../cyclone_tcp/mqtt/mqtt_client_packet.c \
../cyclone_tcp/mqtt/mqtt_client_transport.c 

OBJS += \
./cyclone_tcp/mqtt/mqtt_client.o \
./cyclone_tcp/mqtt/mqtt_client_misc.o \
./cyclone_tcp/mqtt/mqtt_client_packet.o \
./cyclone_tcp/mqtt/mqtt_client_transport.o 

C_DEPS += \
./cyclone_tcp/mqtt/mqtt_client.d \
./cyclone_tcp/mqtt/mqtt_client_misc.d \
./cyclone_tcp/mqtt/mqtt_client_packet.d \
./cyclone_tcp/mqtt/mqtt_client_transport.d 


# Each subdirectory must supply rules for building sources it contributes
cyclone_tcp/mqtt/%.o: ../cyclone_tcp/mqtt/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@cyclone_tcp/mqtt/mqtt_client.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


