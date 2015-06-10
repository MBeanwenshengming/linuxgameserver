################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../LDBOpThread.cpp \
../LDBServerConnectToMasterServer.cpp \
../LDBServerMainLogicThread.cpp \
../LDBServerMessageProcess.cpp \
../LDBServerOpProcessManager.cpp \
../LDBServerPacketProcess.cpp \
../LDBServerPacketProcess_Common.cpp \
../LDBServerUserInfo.cpp \
../LDBServerUserInfoManager.cpp \
../LServer.cpp \
../LServerManager.cpp \
../main.cpp 

OBJS += \
./LDBOpThread.o \
./LDBServerConnectToMasterServer.o \
./LDBServerMainLogicThread.o \
./LDBServerMessageProcess.o \
./LDBServerOpProcessManager.o \
./LDBServerPacketProcess.o \
./LDBServerPacketProcess_Common.o \
./LDBServerUserInfo.o \
./LDBServerUserInfoManager.o \
./LServer.o \
./LServerManager.o \
./main.o 

CPP_DEPS += \
./LDBOpThread.d \
./LDBServerConnectToMasterServer.d \
./LDBServerMainLogicThread.d \
./LDBServerMessageProcess.d \
./LDBServerOpProcessManager.d \
./LDBServerPacketProcess.d \
./LDBServerPacketProcess_Common.d \
./LDBServerUserInfo.d \
./LDBServerUserInfoManager.d \
./LServer.d \
./LServerManager.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/home/wenshengming/workspace/GSAR/trunk/DBLib" -I"/home/wenshengming/workspace/GSAR/trunk/NetWork" -I/usr/include/mysql -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


