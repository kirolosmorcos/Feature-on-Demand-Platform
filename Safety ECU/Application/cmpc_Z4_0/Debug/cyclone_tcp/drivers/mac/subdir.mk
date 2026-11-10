################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cyclone_tcp/drivers/mac/mpc5748_eth1_driver.c \
../cyclone_tcp/drivers/mac/mpc5748_eth2_driver.c 

OBJS += \
./cyclone_tcp/drivers/mac/mpc5748_eth1_driver.o \
./cyclone_tcp/drivers/mac/mpc5748_eth2_driver.o 

C_DEPS += \
./cyclone_tcp/drivers/mac/mpc5748_eth1_driver.d \
./cyclone_tcp/drivers/mac/mpc5748_eth2_driver.d 


# Each subdirectory must supply rules for building sources it contributes
cyclone_tcp/drivers/mac/%.o: ../cyclone_tcp/drivers/mac/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@cyclone_tcp/drivers/mac/mpc5748_eth1_driver.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


