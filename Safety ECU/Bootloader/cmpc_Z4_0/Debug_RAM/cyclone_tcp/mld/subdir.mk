################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cyclone_tcp/mld/mld_common.c \
../cyclone_tcp/mld/mld_debug.c \
../cyclone_tcp/mld/mld_node.c \
../cyclone_tcp/mld/mld_node_misc.c 

OBJS += \
./cyclone_tcp/mld/mld_common.o \
./cyclone_tcp/mld/mld_debug.o \
./cyclone_tcp/mld/mld_node.o \
./cyclone_tcp/mld/mld_node_misc.o 

C_DEPS += \
./cyclone_tcp/mld/mld_common.d \
./cyclone_tcp/mld/mld_debug.d \
./cyclone_tcp/mld/mld_node.d \
./cyclone_tcp/mld/mld_node_misc.d 


# Each subdirectory must supply rules for building sources it contributes
cyclone_tcp/mld/%.o: ../cyclone_tcp/mld/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@cyclone_tcp/mld/mld_common.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


