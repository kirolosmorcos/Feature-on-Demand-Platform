################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cyclone_tcp/mdns/mdns_client.c \
../cyclone_tcp/mdns/mdns_common.c \
../cyclone_tcp/mdns/mdns_responder.c \
../cyclone_tcp/mdns/mdns_responder_misc.c 

OBJS += \
./cyclone_tcp/mdns/mdns_client.o \
./cyclone_tcp/mdns/mdns_common.o \
./cyclone_tcp/mdns/mdns_responder.o \
./cyclone_tcp/mdns/mdns_responder_misc.o 

C_DEPS += \
./cyclone_tcp/mdns/mdns_client.d \
./cyclone_tcp/mdns/mdns_common.d \
./cyclone_tcp/mdns/mdns_responder.d \
./cyclone_tcp/mdns/mdns_responder_misc.d 


# Each subdirectory must supply rules for building sources it contributes
cyclone_tcp/mdns/%.o: ../cyclone_tcp/mdns/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@cyclone_tcp/mdns/mdns_client.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


