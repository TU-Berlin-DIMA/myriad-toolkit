################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/cpp/config/SizingConfigurator.cpp 

OBJS += \
./src/cpp/config/SizingConfigurator.o 

CPP_DEPS += \
./src/cpp/config/SizingConfigurator.d 


# Each subdirectory must supply rules for building sources it contributes
src/cpp/config/%.o: ../src/cpp/config/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"$(POCO_INCLUDE_PATH)" -I"../vendor/myriad/src/cpp" -I"../src/cpp" -O2 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


