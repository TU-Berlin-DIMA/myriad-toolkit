################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm

# All of the sources participating in the build are defined here
-include sources.mk
-include vendor/myriad-toolkit/src/cpp/math/random/subdir.mk
-include vendor/myriad-toolkit/src/cpp/math/probability/subdir.mk
-include vendor/myriad-toolkit/src/cpp/math/algebra/subdir.mk
-include vendor/myriad-toolkit/src/cpp/generator/subdir.mk
-include vendor/myriad-toolkit/src/cpp/core/types/subdir.mk
-include vendor/myriad-toolkit/src/cpp/core/subdir.mk
-include vendor/myriad-toolkit/src/cpp/config/subdir.mk
-include vendor/myriad-toolkit/src/cpp/communication/subdir.mk
-include src/cpp/generator/base/subdir.mk
-include src/cpp/generator/subdir.mk
-include src/cpp/core/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: bin/${{dgen_name}}-node

# Tool invocations
bin/${{dgen_name}}-node: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C++ Linker'
	g++ -L$(POCO_LIBRARY_PATH) -Wl,-rpath,'$$ORIGIN/../lib/c++' -o "bin/${{dgen_name}}-node" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C++_DEPS)$(C_DEPS)$(CC_DEPS)$(CPP_DEPS)$(EXECUTABLES)$(CXX_DEPS)$(C_UPPER_DEPS) bin/${{dgen_name}}-node
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
