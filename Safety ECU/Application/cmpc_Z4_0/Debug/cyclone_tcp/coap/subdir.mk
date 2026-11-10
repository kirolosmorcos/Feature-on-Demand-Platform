################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cyclone_tcp/coap/coap_client.c \
../cyclone_tcp/coap/coap_client_block.c \
../cyclone_tcp/coap/coap_client_misc.c \
../cyclone_tcp/coap/coap_client_observe.c \
../cyclone_tcp/coap/coap_client_request.c \
../cyclone_tcp/coap/coap_client_transport.c \
../cyclone_tcp/coap/coap_debug.c \
../cyclone_tcp/coap/coap_message.c \
../cyclone_tcp/coap/coap_option.c \
../cyclone_tcp/coap/coap_server.c \
../cyclone_tcp/coap/coap_server_misc.c \
../cyclone_tcp/coap/coap_server_request.c \
../cyclone_tcp/coap/coap_server_transport.c 

OBJS += \
./cyclone_tcp/coap/coap_client.o \
./cyclone_tcp/coap/coap_client_block.o \
./cyclone_tcp/coap/coap_client_misc.o \
./cyclone_tcp/coap/coap_client_observe.o \
./cyclone_tcp/coap/coap_client_request.o \
./cyclone_tcp/coap/coap_client_transport.o \
./cyclone_tcp/coap/coap_debug.o \
./cyclone_tcp/coap/coap_message.o \
./cyclone_tcp/coap/coap_option.o \
./cyclone_tcp/coap/coap_server.o \
./cyclone_tcp/coap/coap_server_misc.o \
./cyclone_tcp/coap/coap_server_request.o \
./cyclone_tcp/coap/coap_server_transport.o 

C_DEPS += \
./cyclone_tcp/coap/coap_client.d \
./cyclone_tcp/coap/coap_client_block.d \
./cyclone_tcp/coap/coap_client_misc.d \
./cyclone_tcp/coap/coap_client_observe.d \
./cyclone_tcp/coap/coap_client_request.d \
./cyclone_tcp/coap/coap_client_transport.d \
./cyclone_tcp/coap/coap_debug.d \
./cyclone_tcp/coap/coap_message.d \
./cyclone_tcp/coap/coap_option.d \
./cyclone_tcp/coap/coap_server.d \
./cyclone_tcp/coap/coap_server_misc.d \
./cyclone_tcp/coap/coap_server_request.d \
./cyclone_tcp/coap/coap_server_transport.d 


# Each subdirectory must supply rules for building sources it contributes
cyclone_tcp/coap/%.o: ../cyclone_tcp/coap/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@cyclone_tcp/coap/coap_client.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


