################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cyclone_tcp/igmp/igmp_common.c \
../cyclone_tcp/igmp/igmp_debug.c \
../cyclone_tcp/igmp/igmp_host.c \
../cyclone_tcp/igmp/igmp_host_misc.c \
../cyclone_tcp/igmp/igmp_router.c \
../cyclone_tcp/igmp/igmp_router_misc.c \
../cyclone_tcp/igmp/igmp_snooping.c \
../cyclone_tcp/igmp/igmp_snooping_misc.c 

OBJS += \
./cyclone_tcp/igmp/igmp_common.o \
./cyclone_tcp/igmp/igmp_debug.o \
./cyclone_tcp/igmp/igmp_host.o \
./cyclone_tcp/igmp/igmp_host_misc.o \
./cyclone_tcp/igmp/igmp_router.o \
./cyclone_tcp/igmp/igmp_router_misc.o \
./cyclone_tcp/igmp/igmp_snooping.o \
./cyclone_tcp/igmp/igmp_snooping_misc.o 

C_DEPS += \
./cyclone_tcp/igmp/igmp_common.d \
./cyclone_tcp/igmp/igmp_debug.d \
./cyclone_tcp/igmp/igmp_host.d \
./cyclone_tcp/igmp/igmp_host_misc.d \
./cyclone_tcp/igmp/igmp_router.d \
./cyclone_tcp/igmp/igmp_router_misc.d \
./cyclone_tcp/igmp/igmp_snooping.d \
./cyclone_tcp/igmp/igmp_snooping_misc.d 


# Each subdirectory must supply rules for building sources it contributes
cyclone_tcp/igmp/%.o: ../cyclone_tcp/igmp/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@cyclone_tcp/igmp/igmp_common.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


