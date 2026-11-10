################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_UPPER_SRCS += \
../Project_Settings/Startup_Code/startup_mpc5748g.S 

OBJS += \
./Project_Settings/Startup_Code/startup_mpc5748g.o 


# Each subdirectory must supply rules for building sources it contributes
Project_Settings/Startup_Code/%.o: ../Project_Settings/Startup_Code/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS Assembler'
	powerpc-eabivle-gcc "@Project_Settings/Startup_Code/startup_mpc5748g.args" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


