################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cyclone_tcp/nat/nat.c \
../cyclone_tcp/nat/nat_misc.c 

OBJS += \
./cyclone_tcp/nat/nat.o \
./cyclone_tcp/nat/nat_misc.o 

C_DEPS += \
./cyclone_tcp/nat/nat.d \
./cyclone_tcp/nat/nat_misc.d 


# Each subdirectory must supply rules for building sources it contributes
cyclone_tcp/nat/%.o: ../cyclone_tcp/nat/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@cyclone_tcp/nat/nat.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


