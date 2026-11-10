################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cyclone_tcp/core/bsd_socket.c \
../cyclone_tcp/core/bsd_socket_misc.c \
../cyclone_tcp/core/bsd_socket_options.c \
../cyclone_tcp/core/ethernet.c \
../cyclone_tcp/core/ethernet_misc.c \
../cyclone_tcp/core/ip.c \
../cyclone_tcp/core/net.c \
../cyclone_tcp/core/net_mem.c \
../cyclone_tcp/core/net_misc.c \
../cyclone_tcp/core/nic.c \
../cyclone_tcp/core/ping.c \
../cyclone_tcp/core/raw_socket.c \
../cyclone_tcp/core/socket.c \
../cyclone_tcp/core/socket_misc.c \
../cyclone_tcp/core/tcp.c \
../cyclone_tcp/core/tcp_fsm.c \
../cyclone_tcp/core/tcp_misc.c \
../cyclone_tcp/core/tcp_timer.c \
../cyclone_tcp/core/udp.c 

OBJS += \
./cyclone_tcp/core/bsd_socket.o \
./cyclone_tcp/core/bsd_socket_misc.o \
./cyclone_tcp/core/bsd_socket_options.o \
./cyclone_tcp/core/ethernet.o \
./cyclone_tcp/core/ethernet_misc.o \
./cyclone_tcp/core/ip.o \
./cyclone_tcp/core/net.o \
./cyclone_tcp/core/net_mem.o \
./cyclone_tcp/core/net_misc.o \
./cyclone_tcp/core/nic.o \
./cyclone_tcp/core/ping.o \
./cyclone_tcp/core/raw_socket.o \
./cyclone_tcp/core/socket.o \
./cyclone_tcp/core/socket_misc.o \
./cyclone_tcp/core/tcp.o \
./cyclone_tcp/core/tcp_fsm.o \
./cyclone_tcp/core/tcp_misc.o \
./cyclone_tcp/core/tcp_timer.o \
./cyclone_tcp/core/udp.o 

C_DEPS += \
./cyclone_tcp/core/bsd_socket.d \
./cyclone_tcp/core/bsd_socket_misc.d \
./cyclone_tcp/core/bsd_socket_options.d \
./cyclone_tcp/core/ethernet.d \
./cyclone_tcp/core/ethernet_misc.d \
./cyclone_tcp/core/ip.d \
./cyclone_tcp/core/net.d \
./cyclone_tcp/core/net_mem.d \
./cyclone_tcp/core/net_misc.d \
./cyclone_tcp/core/nic.d \
./cyclone_tcp/core/ping.d \
./cyclone_tcp/core/raw_socket.d \
./cyclone_tcp/core/socket.d \
./cyclone_tcp/core/socket_misc.d \
./cyclone_tcp/core/tcp.d \
./cyclone_tcp/core/tcp_fsm.d \
./cyclone_tcp/core/tcp_misc.d \
./cyclone_tcp/core/tcp_timer.d \
./cyclone_tcp/core/udp.d 


# Each subdirectory must supply rules for building sources it contributes
cyclone_tcp/core/%.o: ../cyclone_tcp/core/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@cyclone_tcp/core/bsd_socket.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


