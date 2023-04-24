################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Middlewares/Third_Party/FatFs/src/option/subdir.mk
-include Middlewares/Third_Party/FatFs/src/subdir.mk
-include FATFS/Target/subdir.mk
-include FATFS/App/subdir.mk
-include Drivers/STM32L4xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
STM_SR_DHM.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
STM_SR_DHM.list \

OBJCOPY_BIN += \
STM_SR_DHM.bin \


# All Target
all: STM_SR_DHM.elf secondary-outputs

# Tool invocations
STM_SR_DHM.elf: $(OBJS) $(USER_OBJS) C:\Users\DostawcaPizzy\STM32CubeIDE\workspace_1.4.0\STM_SR_DHM\STM32L476RGTX_FLASH.ld
	arm-none-eabi-gcc -o "STM_SR_DHM.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"C:\Users\DostawcaPizzy\STM32CubeIDE\workspace_1.4.0\STM_SR_DHM\STM32L476RGTX_FLASH.ld" --specs=nosys.specs -Wl,-Map="STM_SR_DHM.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -u _printf_float -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

STM_SR_DHM.list: $(EXECUTABLES)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "STM_SR_DHM.list"
	@echo 'Finished building: $@'
	@echo ' '

STM_SR_DHM.bin: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "STM_SR_DHM.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_BIN)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
