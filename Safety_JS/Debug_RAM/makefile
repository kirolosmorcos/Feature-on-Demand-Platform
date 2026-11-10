################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include nxp/devices/mpc57xx/drivers/src/flash_c55/subdir.mk
-include nxp/devices/mpc57xx/drivers/src/crc/subdir.mk
-include nxp/devices/mpc57xx/drivers/subdir.mk
-include freertos/portable/memmang/subdir.mk
-include freertos/portable/gcc/powerpc/subdir.mk
-include freertos/portable/common/subdir.mk
-include freertos/subdir.mk
-include cyclone_tcp/tftp/subdir.mk
-include cyclone_tcp/syslog/subdir.mk
-include cyclone_tcp/sntp/subdir.mk
-include cyclone_tcp/ppp/subdir.mk
-include cyclone_tcp/ntp/subdir.mk
-include cyclone_tcp/netbios/subdir.mk
-include cyclone_tcp/nat/subdir.mk
-include cyclone_tcp/mqtt_sn/subdir.mk
-include cyclone_tcp/mqtt/subdir.mk
-include cyclone_tcp/modbus/subdir.mk
-include cyclone_tcp/mld/subdir.mk
-include cyclone_tcp/mdns/subdir.mk
-include cyclone_tcp/llmnr/subdir.mk
-include cyclone_tcp/lldp/subdir.mk
-include cyclone_tcp/ipv6/subdir.mk
-include cyclone_tcp/ipv4/subdir.mk
-include cyclone_tcp/igmp/subdir.mk
-include cyclone_tcp/icecast/subdir.mk
-include cyclone_tcp/http/subdir.mk
-include cyclone_tcp/echo/subdir.mk
-include cyclone_tcp/drivers/phy/subdir.mk
-include cyclone_tcp/drivers/mac/subdir.mk
-include cyclone_tcp/drivers/loopback/subdir.mk
-include cyclone_tcp/drivers/eth/subdir.mk
-include cyclone_tcp/dns_sd/subdir.mk
-include cyclone_tcp/dns/subdir.mk
-include cyclone_tcp/dhcpv6/subdir.mk
-include cyclone_tcp/dhcp/subdir.mk
-include cyclone_tcp/core/subdir.mk
-include cyclone_tcp/coap/subdir.mk
-include common/subdir.mk
-include UDS/subdir.mk
-include Project_Settings/Startup_Code/subdir.mk
-include Project_Settings/Linker_Files/subdir.mk
-include DoIP/subdir.mk
-include Demo_Sources/subdir.mk
-include Bootmanger/subdir.mk
-include Bootloader/bootloader_utils/bl_signature/bl_hash/subdir.mk
-include Bootloader/bootloader_utils/bl_signature/bl_ecdsa/subdir.mk
-include Bootloader/bootloader_utils/bl_signature/subdir.mk
-include Bootloader/bootloader_utils/bl_flash/subdir.mk
-include Bootloader/bootloader_utils/bl_decompress/subdir.mk
-include Bootloader/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_SIZE += \
cmpc_Z4_0.siz \


# All Target
all:
	$(MAKE) --no-print-directory pre-build
	$(MAKE) --no-print-directory main-build

# Main-build Target
main-build: cmpc_Z4_0.elf secondary-outputs

# Tool invocations
cmpc_Z4_0.elf: $(OBJS) C:\coding\cycloneM\cycloneM\cmpc\cmpc_Z4_0\Project_Settings\Linker_Files\mpc5748g_ram.ld $(OSPT) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Standard S32DS C Linker'
	powerpc-eabivle-gcc -o "cmpc_Z4_0.elf" "@cmpc_Z4_0.args"  $(USER_OBJS) $(LIBS) $(OSPT)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

cmpc_Z4_0.siz: cmpc_Z4_0.elf
	@echo 'Invoking: Standard S32DS Print Size'
	powerpc-eabivle-size --format=berkeley cmpc_Z4_0.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) ./*
	-@echo ' '

pre-build:
	-C:/coding/cycloneM/cycloneM/cmpc/cmpc_Z4_0/../../../../../utils/ResourceCompiler/bin/rc.exe C:/coding/cycloneM/cycloneM/cmpc/cmpc_Z4_0/../resources/ C:/coding/cycloneM/cycloneM/cmpc/cmpc_Z4_0/../src/res.c
	-@echo ' '

post-build:
	-powerpc-eabivle-objcopy -O srec cmpc_Z4_0.elf cmpc_Z4_0.s19
	-@echo ' '

secondary-outputs: $(SECONDARY_SIZE)

.PHONY: all clean dependents main-build pre-build post-build

-include ../makefile.targets
