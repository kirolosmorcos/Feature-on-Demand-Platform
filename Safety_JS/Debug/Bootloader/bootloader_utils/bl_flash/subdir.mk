################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Bootloader/bootloader_utils/bl_flash/BootloaderFlash.c \
../Bootloader/bootloader_utils/bl_flash/BootloaderFlashCRCConfig.c 

OBJS += \
./Bootloader/bootloader_utils/bl_flash/BootloaderFlash.o \
./Bootloader/bootloader_utils/bl_flash/BootloaderFlashCRCConfig.o 

C_DEPS += \
./Bootloader/bootloader_utils/bl_flash/BootloaderFlash.d \
./Bootloader/bootloader_utils/bl_flash/BootloaderFlashCRCConfig.d 


# Each subdirectory must supply rules for building sources it contributes
Bootloader/bootloader_utils/bl_flash/%.o: ../Bootloader/bootloader_utils/bl_flash/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@Bootloader/bootloader_utils/bl_flash/BootloaderFlash.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


