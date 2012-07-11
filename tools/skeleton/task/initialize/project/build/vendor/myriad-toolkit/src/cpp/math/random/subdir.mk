################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../vendor/myriad-toolkit/src/cpp/math/random/RNG.cpp \
../vendor/myriad-toolkit/src/cpp/math/random/SurrogateKeyGenerator.cpp 

OBJS += \
./vendor/myriad-toolkit/src/cpp/math/random/RNG.o \
./vendor/myriad-toolkit/src/cpp/math/random/SurrogateKeyGenerator.o 

CPP_DEPS += \
./vendor/myriad-toolkit/src/cpp/math/random/RNG.d \
./vendor/myriad-toolkit/src/cpp/math/random/SurrogateKeyGenerator.d 


# Each subdirectory must supply rules for building sources it contributes
vendor/myriad-toolkit/src/cpp/math/random/%.o: ../vendor/myriad-toolkit/src/cpp/math/random/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"$(POCO_INCLUDE_PATH)" -I"../src/cpp" -I"../vendor/myriad-toolkit/src/cpp" -O2 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


