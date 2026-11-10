################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../freertos/portable/gcc/powerpc/portasm.s 

C_SRCS += \
../freertos/portable/gcc/powerpc/port.c \
../freertos/portable/gcc/powerpc/porttimer.c 

OBJS += \
./freertos/portable/gcc/powerpc/port.o \
./freertos/portable/gcc/powerpc/portasm.o \
./freertos/portable/gcc/powerpc/porttimer.o 

C_DEPS += \
./freertos/portable/gcc/powerpc/port.d \
./freertos/portable/gcc/powerpc/porttimer.d 


# Each subdirectory must supply rules for building sources it contributes
freertos/portable/gcc/powerpc/%.o: ../freertos/portable/gcc/powerpc/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@freertos/portable/gcc/powerpc/port.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

freertos/portable/gcc/powerpc/%.o: ../freertos/portable/gcc/powerpc/%.s
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS Assembler'
	powerpc-eabivle-gcc "@freertos/portable/gcc/powerpc/portasm.args" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


