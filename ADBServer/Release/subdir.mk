################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../LADBServerDBMessageProcess.cpp \
../LADBServerDBOpProcessManager.cpp \
../LADBServerDBOpProcess_Account.cpp \
../LDBOpThread.cpp \
../LDBServerConnectToMasterServer.cpp \
../LDBServerMainLogicThread.cpp \
../LDBServerPacketProcess.cpp \
../LDBServerPacketProcess_Common.cpp \
../LServer.cpp \
../LServerManager.cpp \
../LUserInfo.cpp \
../LUserInfoManager.cpp \
../main.cpp 

C_SRCS += \
../LUserDBOpMessageDefine.c 

OBJS += \
./LADBServerDBMessageProcess.o \
./LADBServerDBOpProcessManager.o \
./LADBServerDBOpProcess_Account.o \
./LDBOpThread.o \
./LDBServerConnectToMasterServer.o \
./LDBServerMainLogicThread.o \
./LDBServerPacketProcess.o \
./LDBServerPacketProcess_Common.o \
./LServer.o \
./LServerManager.o \
./LUserDBOpMessageDefine.o \
./LUserInfo.o \
./LUserInfoManager.o \
./main.o 

C_DEPS += \
./LUserDBOpMessageDefine.d 

CPP_DEPS += \
./LADBServerDBMessageProcess.d \
./LADBServerDBOpProcessManager.d \
./LADBServerDBOpProcess_Account.d \
./LDBOpThread.d \
./LDBServerConnectToMasterServer.d \
./LDBServerMainLogicThread.d \
./LDBServerPacketProcess.d \
./LDBServerPacketProcess_Common.d \
./LServer.d \
./LServerManager.d \
./LUserInfo.d \
./LUserInfoManager.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/home/wenshengming/workspace/GSAR/trunk/DBLib" -I"/home/wenshengming/workspace/GSAR/trunk/NetWork" -I/usr/include/mysql -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/wenshengming/workspace/GSAR/trunk/DBLib" -I"/home/wenshengming/workspace/GSAR/trunk/NetWork" -I/usr/include/mysql -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


