################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../freertos/portable/memmang/heap_4.c 

OBJS += \
./freertos/portable/memmang/heap_4.o 

C_DEPS += \
./freertos/portable/memmang/heap_4.d 


# Each subdirectory must supply rules for building sources it contributes
freertos/portable/memmang/%.o: ../freertos/portable/memmang/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@freertos/portable/memmang/heap_4.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


