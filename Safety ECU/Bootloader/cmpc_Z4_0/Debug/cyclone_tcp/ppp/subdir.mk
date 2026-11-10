################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cyclone_tcp/ppp/chap.c \
../cyclone_tcp/ppp/ipcp.c \
../cyclone_tcp/ppp/ipv6cp.c \
../cyclone_tcp/ppp/lcp.c \
../cyclone_tcp/ppp/pap.c \
../cyclone_tcp/ppp/ppp.c \
../cyclone_tcp/ppp/ppp_debug.c \
../cyclone_tcp/ppp/ppp_fsm.c \
../cyclone_tcp/ppp/ppp_hdlc.c \
../cyclone_tcp/ppp/ppp_misc.c 

OBJS += \
./cyclone_tcp/ppp/chap.o \
./cyclone_tcp/ppp/ipcp.o \
./cyclone_tcp/ppp/ipv6cp.o \
./cyclone_tcp/ppp/lcp.o \
./cyclone_tcp/ppp/pap.o \
./cyclone_tcp/ppp/ppp.o \
./cyclone_tcp/ppp/ppp_debug.o \
./cyclone_tcp/ppp/ppp_fsm.o \
./cyclone_tcp/ppp/ppp_hdlc.o \
./cyclone_tcp/ppp/ppp_misc.o 

C_DEPS += \
./cyclone_tcp/ppp/chap.d \
./cyclone_tcp/ppp/ipcp.d \
./cyclone_tcp/ppp/ipv6cp.d \
./cyclone_tcp/ppp/lcp.d \
./cyclone_tcp/ppp/pap.d \
./cyclone_tcp/ppp/ppp.d \
./cyclone_tcp/ppp/ppp_debug.d \
./cyclone_tcp/ppp/ppp_fsm.d \
./cyclone_tcp/ppp/ppp_hdlc.d \
./cyclone_tcp/ppp/ppp_misc.d 


# Each subdirectory must supply rules for building sources it contributes
cyclone_tcp/ppp/%.o: ../cyclone_tcp/ppp/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@cyclone_tcp/ppp/chap.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


