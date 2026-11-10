################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../nxp/devices/mpc57xx/drivers/interrupt_manager_MPC57xx.c 

OBJS += \
./nxp/devices/mpc57xx/drivers/interrupt_manager_MPC57xx.o 

C_DEPS += \
./nxp/devices/mpc57xx/drivers/interrupt_manager_MPC57xx.d 


# Each subdirectory must supply rules for building sources it contributes
nxp/devices/mpc57xx/drivers/%.o: ../nxp/devices/mpc57xx/drivers/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@nxp/devices/mpc57xx/drivers/interrupt_manager_MPC57xx.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


