################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cyclone_tcp/dns_sd/dns_sd_responder.c \
../cyclone_tcp/dns_sd/dns_sd_responder_misc.c 

OBJS += \
./cyclone_tcp/dns_sd/dns_sd_responder.o \
./cyclone_tcp/dns_sd/dns_sd_responder_misc.o 

C_DEPS += \
./cyclone_tcp/dns_sd/dns_sd_responder.d \
./cyclone_tcp/dns_sd/dns_sd_responder_misc.d 


# Each subdirectory must supply rules for building sources it contributes
cyclone_tcp/dns_sd/%.o: ../cyclone_tcp/dns_sd/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@cyclone_tcp/dns_sd/dns_sd_responder.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


