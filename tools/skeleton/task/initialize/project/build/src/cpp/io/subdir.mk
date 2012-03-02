################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/cpp/io/OutputCollector.cpp 

OBJS += \
./src/cpp/io/OutputCollector.o 

CPP_DEPS += \
./src/cpp/io/OutputCollector.d 


# Each subdirectory must supply rules for building sources it contributes
src/cpp/io/%.o: ../src/cpp/io/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"$(POCO_INCLUDE_PATH)" -I"../src/cpp" -I"../vendor/myriad/src/cpp" -O2 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


