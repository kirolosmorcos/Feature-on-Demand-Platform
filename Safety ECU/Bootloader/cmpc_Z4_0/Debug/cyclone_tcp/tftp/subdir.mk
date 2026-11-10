################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cyclone_tcp/tftp/tftp_client.c \
../cyclone_tcp/tftp/tftp_client_misc.c \
../cyclone_tcp/tftp/tftp_server.c \
../cyclone_tcp/tftp/tftp_server_misc.c 

OBJS += \
./cyclone_tcp/tftp/tftp_client.o \
./cyclone_tcp/tftp/tftp_client_misc.o \
./cyclone_tcp/tftp/tftp_server.o \
./cyclone_tcp/tftp/tftp_server_misc.o 

C_DEPS += \
./cyclone_tcp/tftp/tftp_client.d \
./cyclone_tcp/tftp/tftp_client_misc.d \
./cyclone_tcp/tftp/tftp_server.d \
./cyclone_tcp/tftp/tftp_server_misc.d 


# Each subdirectory must supply rules for building sources it contributes
cyclone_tcp/tftp/%.o: ../cyclone_tcp/tftp/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@cyclone_tcp/tftp/tftp_client.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


