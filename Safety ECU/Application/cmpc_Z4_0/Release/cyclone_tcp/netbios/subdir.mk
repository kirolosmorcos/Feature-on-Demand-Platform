################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cyclone_tcp/netbios/nbns_client.c \
../cyclone_tcp/netbios/nbns_common.c \
../cyclone_tcp/netbios/nbns_responder.c 

OBJS += \
./cyclone_tcp/netbios/nbns_client.o \
./cyclone_tcp/netbios/nbns_common.o \
./cyclone_tcp/netbios/nbns_responder.o 

C_DEPS += \
./cyclone_tcp/netbios/nbns_client.d \
./cyclone_tcp/netbios/nbns_common.d \
./cyclone_tcp/netbios/nbns_responder.d 


# Each subdirectory must supply rules for building sources it contributes
cyclone_tcp/netbios/%.o: ../cyclone_tcp/netbios/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@cyclone_tcp/netbios/nbns_client.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


