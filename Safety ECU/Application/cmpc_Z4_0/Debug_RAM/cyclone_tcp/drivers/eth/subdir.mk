################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cyclone_tcp/drivers/eth/adin1110_driver.c \
../cyclone_tcp/drivers/eth/dm9000_driver.c \
../cyclone_tcp/drivers/eth/enc28j60_driver.c \
../cyclone_tcp/drivers/eth/enc424j600_driver.c \
../cyclone_tcp/drivers/eth/enc624j600_driver.c \
../cyclone_tcp/drivers/eth/ksz8851_driver.c \
../cyclone_tcp/drivers/eth/lan8650_driver.c \
../cyclone_tcp/drivers/eth/lan8651_driver.c \
../cyclone_tcp/drivers/eth/lan9250_driver.c \
../cyclone_tcp/drivers/eth/ncn26010_driver.c \
../cyclone_tcp/drivers/eth/ncv7410_driver.c \
../cyclone_tcp/drivers/eth/w3150a_driver.c \
../cyclone_tcp/drivers/eth/w5100_driver.c \
../cyclone_tcp/drivers/eth/w5100s_driver.c \
../cyclone_tcp/drivers/eth/w5200_driver.c \
../cyclone_tcp/drivers/eth/w5500_driver.c \
../cyclone_tcp/drivers/eth/w6100_driver.c 

OBJS += \
./cyclone_tcp/drivers/eth/adin1110_driver.o \
./cyclone_tcp/drivers/eth/dm9000_driver.o \
./cyclone_tcp/drivers/eth/enc28j60_driver.o \
./cyclone_tcp/drivers/eth/enc424j600_driver.o \
./cyclone_tcp/drivers/eth/enc624j600_driver.o \
./cyclone_tcp/drivers/eth/ksz8851_driver.o \
./cyclone_tcp/drivers/eth/lan8650_driver.o \
./cyclone_tcp/drivers/eth/lan8651_driver.o \
./cyclone_tcp/drivers/eth/lan9250_driver.o \
./cyclone_tcp/drivers/eth/ncn26010_driver.o \
./cyclone_tcp/drivers/eth/ncv7410_driver.o \
./cyclone_tcp/drivers/eth/w3150a_driver.o \
./cyclone_tcp/drivers/eth/w5100_driver.o \
./cyclone_tcp/drivers/eth/w5100s_driver.o \
./cyclone_tcp/drivers/eth/w5200_driver.o \
./cyclone_tcp/drivers/eth/w5500_driver.o \
./cyclone_tcp/drivers/eth/w6100_driver.o 

C_DEPS += \
./cyclone_tcp/drivers/eth/adin1110_driver.d \
./cyclone_tcp/drivers/eth/dm9000_driver.d \
./cyclone_tcp/drivers/eth/enc28j60_driver.d \
./cyclone_tcp/drivers/eth/enc424j600_driver.d \
./cyclone_tcp/drivers/eth/enc624j600_driver.d \
./cyclone_tcp/drivers/eth/ksz8851_driver.d \
./cyclone_tcp/drivers/eth/lan8650_driver.d \
./cyclone_tcp/drivers/eth/lan8651_driver.d \
./cyclone_tcp/drivers/eth/lan9250_driver.d \
./cyclone_tcp/drivers/eth/ncn26010_driver.d \
./cyclone_tcp/drivers/eth/ncv7410_driver.d \
./cyclone_tcp/drivers/eth/w3150a_driver.d \
./cyclone_tcp/drivers/eth/w5100_driver.d \
./cyclone_tcp/drivers/eth/w5100s_driver.d \
./cyclone_tcp/drivers/eth/w5200_driver.d \
./cyclone_tcp/drivers/eth/w5500_driver.d \
./cyclone_tcp/drivers/eth/w6100_driver.d 


# Each subdirectory must supply rules for building sources it contributes
cyclone_tcp/drivers/eth/%.o: ../cyclone_tcp/drivers/eth/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@cyclone_tcp/drivers/eth/adin1110_driver.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


