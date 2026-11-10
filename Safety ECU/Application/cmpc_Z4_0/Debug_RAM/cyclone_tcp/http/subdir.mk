################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cyclone_tcp/http/http_client.c \
../cyclone_tcp/http/http_client_auth.c \
../cyclone_tcp/http/http_client_misc.c \
../cyclone_tcp/http/http_client_transport.c \
../cyclone_tcp/http/http_common.c \
../cyclone_tcp/http/http_server.c \
../cyclone_tcp/http/http_server_auth.c \
../cyclone_tcp/http/http_server_misc.c \
../cyclone_tcp/http/mime.c \
../cyclone_tcp/http/ssi.c 

OBJS += \
./cyclone_tcp/http/http_client.o \
./cyclone_tcp/http/http_client_auth.o \
./cyclone_tcp/http/http_client_misc.o \
./cyclone_tcp/http/http_client_transport.o \
./cyclone_tcp/http/http_common.o \
./cyclone_tcp/http/http_server.o \
./cyclone_tcp/http/http_server_auth.o \
./cyclone_tcp/http/http_server_misc.o \
./cyclone_tcp/http/mime.o \
./cyclone_tcp/http/ssi.o 

C_DEPS += \
./cyclone_tcp/http/http_client.d \
./cyclone_tcp/http/http_client_auth.d \
./cyclone_tcp/http/http_client_misc.d \
./cyclone_tcp/http/http_client_transport.d \
./cyclone_tcp/http/http_common.d \
./cyclone_tcp/http/http_server.d \
./cyclone_tcp/http/http_server_auth.d \
./cyclone_tcp/http/http_server_misc.d \
./cyclone_tcp/http/mime.d \
./cyclone_tcp/http/ssi.d 


# Each subdirectory must supply rules for building sources it contributes
cyclone_tcp/http/%.o: ../cyclone_tcp/http/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@cyclone_tcp/http/http_client.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


