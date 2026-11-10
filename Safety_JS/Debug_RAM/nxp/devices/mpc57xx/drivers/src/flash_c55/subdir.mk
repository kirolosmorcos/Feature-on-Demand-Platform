################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../nxp/devices/mpc57xx/drivers/src/flash_c55/flash_c55_driver.c 

OBJS += \
./nxp/devices/mpc57xx/drivers/src/flash_c55/flash_c55_driver.o 

C_DEPS += \
./nxp/devices/mpc57xx/drivers/src/flash_c55/flash_c55_driver.d 


# Each subdirectory must supply rules for building sources it contributes
nxp/devices/mpc57xx/drivers/src/flash_c55/%.o: ../nxp/devices/mpc57xx/drivers/src/flash_c55/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@nxp/devices/mpc57xx/drivers/src/flash_c55/flash_c55_driver.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


