################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Demo_Sources/debug.c \
../Demo_Sources/main.c \
../Demo_Sources/res.c \
../Demo_Sources/startup.c \
../Demo_Sources/strcasecmp.c \
../Demo_Sources/strtok_r.c \
../Demo_Sources/system_mpc5748g.c 

S_UPPER_SRCS += \
../Demo_Sources/core0_intc_sw_handlers.S \
../Demo_Sources/interrupt_vectors.S 

OBJS += \
./Demo_Sources/core0_intc_sw_handlers.o \
./Demo_Sources/debug.o \
./Demo_Sources/interrupt_vectors.o \
./Demo_Sources/main.o \
./Demo_Sources/res.o \
./Demo_Sources/startup.o \
./Demo_Sources/strcasecmp.o \
./Demo_Sources/strtok_r.o \
./Demo_Sources/system_mpc5748g.o 

C_DEPS += \
./Demo_Sources/debug.d \
./Demo_Sources/main.d \
./Demo_Sources/res.d \
./Demo_Sources/startup.d \
./Demo_Sources/strcasecmp.d \
./Demo_Sources/strtok_r.d \
./Demo_Sources/system_mpc5748g.d 


# Each subdirectory must supply rules for building sources it contributes
Demo_Sources/%.o: ../Demo_Sources/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS Assembler'
	powerpc-eabivle-gcc "@Demo_Sources/core0_intc_sw_handlers.args" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Demo_Sources/%.o: ../Demo_Sources/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@Demo_Sources/debug.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


