################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cyclone_tcp/dns/dns_cache.c \
../cyclone_tcp/dns/dns_client.c \
../cyclone_tcp/dns/dns_common.c \
../cyclone_tcp/dns/dns_debug.c 

OBJS += \
./cyclone_tcp/dns/dns_cache.o \
./cyclone_tcp/dns/dns_client.o \
./cyclone_tcp/dns/dns_common.o \
./cyclone_tcp/dns/dns_debug.o 

C_DEPS += \
./cyclone_tcp/dns/dns_cache.d \
./cyclone_tcp/dns/dns_client.d \
./cyclone_tcp/dns/dns_common.d \
./cyclone_tcp/dns/dns_debug.d 


# Each subdirectory must supply rules for building sources it contributes
cyclone_tcp/dns/%.o: ../cyclone_tcp/dns/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@cyclone_tcp/dns/dns_cache.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


