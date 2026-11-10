################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../nxp/devices/mpc57xx/drivers/src/crc/crc_c55_hw_access.c \
../nxp/devices/mpc57xx/drivers/src/crc/crc_driver.c 

OBJS += \
./nxp/devices/mpc57xx/drivers/src/crc/crc_c55_hw_access.o \
./nxp/devices/mpc57xx/drivers/src/crc/crc_driver.o 

C_DEPS += \
./nxp/devices/mpc57xx/drivers/src/crc/crc_c55_hw_access.d \
./nxp/devices/mpc57xx/drivers/src/crc/crc_driver.d 


# Each subdirectory must supply rules for building sources it contributes
nxp/devices/mpc57xx/drivers/src/crc/%.o: ../nxp/devices/mpc57xx/drivers/src/crc/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@nxp/devices/mpc57xx/drivers/src/crc/crc_c55_hw_access.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


