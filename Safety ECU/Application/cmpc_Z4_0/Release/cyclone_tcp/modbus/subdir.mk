################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cyclone_tcp/modbus/modbus_client.c \
../cyclone_tcp/modbus/modbus_client_misc.c \
../cyclone_tcp/modbus/modbus_client_pdu.c \
../cyclone_tcp/modbus/modbus_client_transport.c \
../cyclone_tcp/modbus/modbus_debug.c \
../cyclone_tcp/modbus/modbus_server.c \
../cyclone_tcp/modbus/modbus_server_misc.c \
../cyclone_tcp/modbus/modbus_server_pdu.c \
../cyclone_tcp/modbus/modbus_server_security.c \
../cyclone_tcp/modbus/modbus_server_transport.c 

OBJS += \
./cyclone_tcp/modbus/modbus_client.o \
./cyclone_tcp/modbus/modbus_client_misc.o \
./cyclone_tcp/modbus/modbus_client_pdu.o \
./cyclone_tcp/modbus/modbus_client_transport.o \
./cyclone_tcp/modbus/modbus_debug.o \
./cyclone_tcp/modbus/modbus_server.o \
./cyclone_tcp/modbus/modbus_server_misc.o \
./cyclone_tcp/modbus/modbus_server_pdu.o \
./cyclone_tcp/modbus/modbus_server_security.o \
./cyclone_tcp/modbus/modbus_server_transport.o 

C_DEPS += \
./cyclone_tcp/modbus/modbus_client.d \
./cyclone_tcp/modbus/modbus_client_misc.d \
./cyclone_tcp/modbus/modbus_client_pdu.d \
./cyclone_tcp/modbus/modbus_client_transport.d \
./cyclone_tcp/modbus/modbus_debug.d \
./cyclone_tcp/modbus/modbus_server.d \
./cyclone_tcp/modbus/modbus_server_misc.d \
./cyclone_tcp/modbus/modbus_server_pdu.d \
./cyclone_tcp/modbus/modbus_server_security.d \
./cyclone_tcp/modbus/modbus_server_transport.d 


# Each subdirectory must supply rules for building sources it contributes
cyclone_tcp/modbus/%.o: ../cyclone_tcp/modbus/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@cyclone_tcp/modbus/modbus_client.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


