################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cyclone_tcp/lldp/lldp.c \
../cyclone_tcp/lldp/lldp_debug.c \
../cyclone_tcp/lldp/lldp_ext_dot1.c \
../cyclone_tcp/lldp/lldp_ext_dot3.c \
../cyclone_tcp/lldp/lldp_ext_med.c \
../cyclone_tcp/lldp/lldp_fsm.c \
../cyclone_tcp/lldp/lldp_mgmt.c \
../cyclone_tcp/lldp/lldp_misc.c \
../cyclone_tcp/lldp/lldp_procedures.c \
../cyclone_tcp/lldp/lldp_rx_fsm.c \
../cyclone_tcp/lldp/lldp_tlv.c \
../cyclone_tcp/lldp/lldp_tx_fsm.c 

OBJS += \
./cyclone_tcp/lldp/lldp.o \
./cyclone_tcp/lldp/lldp_debug.o \
./cyclone_tcp/lldp/lldp_ext_dot1.o \
./cyclone_tcp/lldp/lldp_ext_dot3.o \
./cyclone_tcp/lldp/lldp_ext_med.o \
./cyclone_tcp/lldp/lldp_fsm.o \
./cyclone_tcp/lldp/lldp_mgmt.o \
./cyclone_tcp/lldp/lldp_misc.o \
./cyclone_tcp/lldp/lldp_procedures.o \
./cyclone_tcp/lldp/lldp_rx_fsm.o \
./cyclone_tcp/lldp/lldp_tlv.o \
./cyclone_tcp/lldp/lldp_tx_fsm.o 

C_DEPS += \
./cyclone_tcp/lldp/lldp.d \
./cyclone_tcp/lldp/lldp_debug.d \
./cyclone_tcp/lldp/lldp_ext_dot1.d \
./cyclone_tcp/lldp/lldp_ext_dot3.d \
./cyclone_tcp/lldp/lldp_ext_med.d \
./cyclone_tcp/lldp/lldp_fsm.d \
./cyclone_tcp/lldp/lldp_mgmt.d \
./cyclone_tcp/lldp/lldp_misc.d \
./cyclone_tcp/lldp/lldp_procedures.d \
./cyclone_tcp/lldp/lldp_rx_fsm.d \
./cyclone_tcp/lldp/lldp_tlv.d \
./cyclone_tcp/lldp/lldp_tx_fsm.d 


# Each subdirectory must supply rules for building sources it contributes
cyclone_tcp/lldp/%.o: ../cyclone_tcp/lldp/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@cyclone_tcp/lldp/lldp.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


