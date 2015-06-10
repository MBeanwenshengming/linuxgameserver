################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../SelectServerMainLogicThread.cpp \
../main.cpp 

OBJS += \
./SelectServerMainLogicThread.o \
./main.o 

CPP_DEPS += \
./SelectServerMainLogicThread.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -D__EPOLL_TEST_STATISTIC__ -D__USE_SESSION_BUF_TO_SEND_DATA__ -I"/home/wenshengming/workspace/GSAR/trunk/NetWork" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


