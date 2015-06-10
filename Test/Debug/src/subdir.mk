################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/LErrorWriter.cpp \
../src/LFixLenCircleBuf.cpp \
../src/Test.cpp 

OBJS += \
./src/LErrorWriter.o \
./src/LFixLenCircleBuf.o \
./src/Test.o 

CPP_DEPS += \
./src/LErrorWriter.d \
./src/LFixLenCircleBuf.d \
./src/Test.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


