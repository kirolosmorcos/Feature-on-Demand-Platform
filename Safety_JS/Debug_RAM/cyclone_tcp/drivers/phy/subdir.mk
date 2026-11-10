################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cyclone_tcp/drivers/phy/ksz8081_driver.c 

OBJS += \
./cyclone_tcp/drivers/phy/ksz8081_driver.o 

C_DEPS += \
./cyclone_tcp/drivers/phy/ksz8081_driver.d 


# Each subdirectory must supply rules for building sources it contributes
cyclone_tcp/drivers/phy/%.o: ../cyclone_tcp/drivers/phy/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@cyclone_tcp/drivers/phy/ksz8081_driver.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


