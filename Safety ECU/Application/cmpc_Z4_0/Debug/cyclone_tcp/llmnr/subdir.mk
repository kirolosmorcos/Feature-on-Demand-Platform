################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cyclone_tcp/llmnr/llmnr_client.c \
../cyclone_tcp/llmnr/llmnr_common.c \
../cyclone_tcp/llmnr/llmnr_responder.c 

OBJS += \
./cyclone_tcp/llmnr/llmnr_client.o \
./cyclone_tcp/llmnr/llmnr_common.o \
./cyclone_tcp/llmnr/llmnr_responder.o 

C_DEPS += \
./cyclone_tcp/llmnr/llmnr_client.d \
./cyclone_tcp/llmnr/llmnr_common.d \
./cyclone_tcp/llmnr/llmnr_responder.d 


# Each subdirectory must supply rules for building sources it contributes
cyclone_tcp/llmnr/%.o: ../cyclone_tcp/llmnr/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@cyclone_tcp/llmnr/llmnr_client.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


