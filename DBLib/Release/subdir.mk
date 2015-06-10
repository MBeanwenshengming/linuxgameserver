################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../XDBBase.cpp \
../XMySqlConnector.cpp \
../XSqlServerConnector.cpp 

OBJS += \
./XDBBase.o \
./XMySqlConnector.o \
./XSqlServerConnector.o 

CPP_DEPS += \
./XDBBase.d \
./XMySqlConnector.d \
./XSqlServerConnector.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/include/mysql -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


