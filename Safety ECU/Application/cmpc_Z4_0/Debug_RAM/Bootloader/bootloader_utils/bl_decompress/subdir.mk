################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Bootloader/bootloader_utils/bl_decompress/bl_decompress.c \
../Bootloader/bootloader_utils/bl_decompress/bl_decompress_pbcfg.c \
../Bootloader/bootloader_utils/bl_decompress/lz4.c 

OBJS += \
./Bootloader/bootloader_utils/bl_decompress/bl_decompress.o \
./Bootloader/bootloader_utils/bl_decompress/bl_decompress_pbcfg.o \
./Bootloader/bootloader_utils/bl_decompress/lz4.o 

C_DEPS += \
./Bootloader/bootloader_utils/bl_decompress/bl_decompress.d \
./Bootloader/bootloader_utils/bl_decompress/bl_decompress_pbcfg.d \
./Bootloader/bootloader_utils/bl_decompress/lz4.d 


# Each subdirectory must supply rules for building sources it contributes
Bootloader/bootloader_utils/bl_decompress/%.o: ../Bootloader/bootloader_utils/bl_decompress/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@Bootloader/bootloader_utils/bl_decompress/bl_decompress.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


