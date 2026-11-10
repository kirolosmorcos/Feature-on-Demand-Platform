################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cyclone_tcp/ipv6/icmpv6.c \
../cyclone_tcp/ipv6/ipv6.c \
../cyclone_tcp/ipv6/ipv6_frag.c \
../cyclone_tcp/ipv6/ipv6_misc.c \
../cyclone_tcp/ipv6/ipv6_multicast.c \
../cyclone_tcp/ipv6/ipv6_pmtu.c \
../cyclone_tcp/ipv6/ipv6_routing.c \
../cyclone_tcp/ipv6/ndp.c \
../cyclone_tcp/ipv6/ndp_cache.c \
../cyclone_tcp/ipv6/ndp_misc.c \
../cyclone_tcp/ipv6/ndp_router_adv.c \
../cyclone_tcp/ipv6/ndp_router_adv_misc.c \
../cyclone_tcp/ipv6/slaac.c \
../cyclone_tcp/ipv6/slaac_misc.c 

OBJS += \
./cyclone_tcp/ipv6/icmpv6.o \
./cyclone_tcp/ipv6/ipv6.o \
./cyclone_tcp/ipv6/ipv6_frag.o \
./cyclone_tcp/ipv6/ipv6_misc.o \
./cyclone_tcp/ipv6/ipv6_multicast.o \
./cyclone_tcp/ipv6/ipv6_pmtu.o \
./cyclone_tcp/ipv6/ipv6_routing.o \
./cyclone_tcp/ipv6/ndp.o \
./cyclone_tcp/ipv6/ndp_cache.o \
./cyclone_tcp/ipv6/ndp_misc.o \
./cyclone_tcp/ipv6/ndp_router_adv.o \
./cyclone_tcp/ipv6/ndp_router_adv_misc.o \
./cyclone_tcp/ipv6/slaac.o \
./cyclone_tcp/ipv6/slaac_misc.o 

C_DEPS += \
./cyclone_tcp/ipv6/icmpv6.d \
./cyclone_tcp/ipv6/ipv6.d \
./cyclone_tcp/ipv6/ipv6_frag.d \
./cyclone_tcp/ipv6/ipv6_misc.d \
./cyclone_tcp/ipv6/ipv6_multicast.d \
./cyclone_tcp/ipv6/ipv6_pmtu.d \
./cyclone_tcp/ipv6/ipv6_routing.d \
./cyclone_tcp/ipv6/ndp.d \
./cyclone_tcp/ipv6/ndp_cache.d \
./cyclone_tcp/ipv6/ndp_misc.d \
./cyclone_tcp/ipv6/ndp_router_adv.d \
./cyclone_tcp/ipv6/ndp_router_adv_misc.d \
./cyclone_tcp/ipv6/slaac.d \
./cyclone_tcp/ipv6/slaac_misc.d 


# Each subdirectory must supply rules for building sources it contributes
cyclone_tcp/ipv6/%.o: ../cyclone_tcp/ipv6/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@cyclone_tcp/ipv6/icmpv6.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


