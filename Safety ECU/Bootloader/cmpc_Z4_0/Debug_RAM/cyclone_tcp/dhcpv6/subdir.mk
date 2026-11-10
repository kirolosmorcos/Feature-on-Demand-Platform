################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cyclone_tcp/dhcpv6/dhcpv6_client.c \
../cyclone_tcp/dhcpv6/dhcpv6_client_fsm.c \
../cyclone_tcp/dhcpv6/dhcpv6_client_misc.c \
../cyclone_tcp/dhcpv6/dhcpv6_common.c \
../cyclone_tcp/dhcpv6/dhcpv6_debug.c \
../cyclone_tcp/dhcpv6/dhcpv6_relay.c 

OBJS += \
./cyclone_tcp/dhcpv6/dhcpv6_client.o \
./cyclone_tcp/dhcpv6/dhcpv6_client_fsm.o \
./cyclone_tcp/dhcpv6/dhcpv6_client_misc.o \
./cyclone_tcp/dhcpv6/dhcpv6_common.o \
./cyclone_tcp/dhcpv6/dhcpv6_debug.o \
./cyclone_tcp/dhcpv6/dhcpv6_relay.o 

C_DEPS += \
./cyclone_tcp/dhcpv6/dhcpv6_client.d \
./cyclone_tcp/dhcpv6/dhcpv6_client_fsm.d \
./cyclone_tcp/dhcpv6/dhcpv6_client_misc.d \
./cyclone_tcp/dhcpv6/dhcpv6_common.d \
./cyclone_tcp/dhcpv6/dhcpv6_debug.d \
./cyclone_tcp/dhcpv6/dhcpv6_relay.d 


# Each subdirectory must supply rules for building sources it contributes
cyclone_tcp/dhcpv6/%.o: ../cyclone_tcp/dhcpv6/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@cyclone_tcp/dhcpv6/dhcpv6_client.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


