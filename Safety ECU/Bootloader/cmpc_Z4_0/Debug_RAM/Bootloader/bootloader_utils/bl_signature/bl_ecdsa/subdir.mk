################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Bootloader/bootloader_utils/bl_signature/bl_ecdsa/uECC.c 

OBJS += \
./Bootloader/bootloader_utils/bl_signature/bl_ecdsa/uECC.o 

C_DEPS += \
./Bootloader/bootloader_utils/bl_signature/bl_ecdsa/uECC.d 


# Each subdirectory must supply rules for building sources it contributes
Bootloader/bootloader_utils/bl_signature/bl_ecdsa/%.o: ../Bootloader/bootloader_utils/bl_signature/bl_ecdsa/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@Bootloader/bootloader_utils/bl_signature/bl_ecdsa/uECC.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


