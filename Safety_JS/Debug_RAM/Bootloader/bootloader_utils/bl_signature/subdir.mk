################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Bootloader/bootloader_utils/bl_signature/bl_signature.c \
../Bootloader/bootloader_utils/bl_signature/bl_signature_pbcfg.c 

OBJS += \
./Bootloader/bootloader_utils/bl_signature/bl_signature.o \
./Bootloader/bootloader_utils/bl_signature/bl_signature_pbcfg.o 

C_DEPS += \
./Bootloader/bootloader_utils/bl_signature/bl_signature.d \
./Bootloader/bootloader_utils/bl_signature/bl_signature_pbcfg.d 


# Each subdirectory must supply rules for building sources it contributes
Bootloader/bootloader_utils/bl_signature/%.o: ../Bootloader/bootloader_utils/bl_signature/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@Bootloader/bootloader_utils/bl_signature/bl_signature.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


