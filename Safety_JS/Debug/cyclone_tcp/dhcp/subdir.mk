################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cyclone_tcp/dhcp/dhcp_client.c \
../cyclone_tcp/dhcp/dhcp_client_fsm.c \
../cyclone_tcp/dhcp/dhcp_client_misc.c \
../cyclone_tcp/dhcp/dhcp_common.c \
../cyclone_tcp/dhcp/dhcp_debug.c \
../cyclone_tcp/dhcp/dhcp_server.c \
../cyclone_tcp/dhcp/dhcp_server_misc.c 

OBJS += \
./cyclone_tcp/dhcp/dhcp_client.o \
./cyclone_tcp/dhcp/dhcp_client_fsm.o \
./cyclone_tcp/dhcp/dhcp_client_misc.o \
./cyclone_tcp/dhcp/dhcp_common.o \
./cyclone_tcp/dhcp/dhcp_debug.o \
./cyclone_tcp/dhcp/dhcp_server.o \
./cyclone_tcp/dhcp/dhcp_server_misc.o 

C_DEPS += \
./cyclone_tcp/dhcp/dhcp_client.d \
./cyclone_tcp/dhcp/dhcp_client_fsm.d \
./cyclone_tcp/dhcp/dhcp_client_misc.d \
./cyclone_tcp/dhcp/dhcp_common.d \
./cyclone_tcp/dhcp/dhcp_debug.d \
./cyclone_tcp/dhcp/dhcp_server.d \
./cyclone_tcp/dhcp/dhcp_server_misc.d 


# Each subdirectory must supply rules for building sources it contributes
cyclone_tcp/dhcp/%.o: ../cyclone_tcp/dhcp/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@cyclone_tcp/dhcp/dhcp_client.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


