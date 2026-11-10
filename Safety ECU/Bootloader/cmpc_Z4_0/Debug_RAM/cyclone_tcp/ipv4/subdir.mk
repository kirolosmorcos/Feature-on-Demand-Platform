################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cyclone_tcp/ipv4/arp.c \
../cyclone_tcp/ipv4/arp_cache.c \
../cyclone_tcp/ipv4/auto_ip.c \
../cyclone_tcp/ipv4/auto_ip_misc.c \
../cyclone_tcp/ipv4/icmp.c \
../cyclone_tcp/ipv4/ipv4.c \
../cyclone_tcp/ipv4/ipv4_frag.c \
../cyclone_tcp/ipv4/ipv4_misc.c \
../cyclone_tcp/ipv4/ipv4_multicast.c 

OBJS += \
./cyclone_tcp/ipv4/arp.o \
./cyclone_tcp/ipv4/arp_cache.o \
./cyclone_tcp/ipv4/auto_ip.o \
./cyclone_tcp/ipv4/auto_ip_misc.o \
./cyclone_tcp/ipv4/icmp.o \
./cyclone_tcp/ipv4/ipv4.o \
./cyclone_tcp/ipv4/ipv4_frag.o \
./cyclone_tcp/ipv4/ipv4_misc.o \
./cyclone_tcp/ipv4/ipv4_multicast.o 

C_DEPS += \
./cyclone_tcp/ipv4/arp.d \
./cyclone_tcp/ipv4/arp_cache.d \
./cyclone_tcp/ipv4/auto_ip.d \
./cyclone_tcp/ipv4/auto_ip_misc.d \
./cyclone_tcp/ipv4/icmp.d \
./cyclone_tcp/ipv4/ipv4.d \
./cyclone_tcp/ipv4/ipv4_frag.d \
./cyclone_tcp/ipv4/ipv4_misc.d \
./cyclone_tcp/ipv4/ipv4_multicast.d 


# Each subdirectory must supply rules for building sources it contributes
cyclone_tcp/ipv4/%.o: ../cyclone_tcp/ipv4/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@cyclone_tcp/ipv4/arp.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


