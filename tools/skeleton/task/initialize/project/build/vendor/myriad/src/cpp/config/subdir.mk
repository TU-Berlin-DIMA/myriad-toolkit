################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../vendor/myriad/src/cpp/config/AbstractGeneratorConfig.cpp \
../vendor/myriad/src/cpp/config/FunctionPool.cpp 

OBJS += \
./vendor/myriad/src/cpp/config/AbstractGeneratorConfig.o \
./vendor/myriad/src/cpp/config/FunctionPool.o 

CPP_DEPS += \
./vendor/myriad/src/cpp/config/AbstractGeneratorConfig.d \
./vendor/myriad/src/cpp/config/FunctionPool.d 


# Each subdirectory must supply rules for building sources it contributes
vendor/myriad/src/cpp/config/%.o: ../vendor/myriad/src/cpp/config/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"$(POCO_INCLUDE_PATH)" -I"../vendor/myriad/src/cpp" -I"../src/cpp" -O2 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


