################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../common/cpu_endian.c \
../common/date_time.c \
../common/os_port_freertos.c \
../common/path.c \
../common/resource_manager.c \
../common/str.c 

OBJS += \
./common/cpu_endian.o \
./common/date_time.o \
./common/os_port_freertos.o \
./common/path.o \
./common/resource_manager.o \
./common/str.o 

C_DEPS += \
./common/cpu_endian.d \
./common/date_time.d \
./common/os_port_freertos.d \
./common/path.d \
./common/resource_manager.d \
./common/str.d 


# Each subdirectory must supply rules for building sources it contributes
common/%.o: ../common/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@common/cpu_endian.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


