################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../LClient.cpp \
../LClientManager.cpp \
../LConnectToAccountDBServer.cpp \
../LConnectToMaster.cpp \
../LLSServer.cpp \
../LLSServerManager.cpp \
../LLoginServerConnectToServerNetWork.cpp \
../LLoginServerMainLogic.cpp \
../LLoginServerPacketProcess_AccountDB.cpp \
../LLoginServerPacketProcess_Client.cpp \
../LLoginServerPacketProcess_Master.cpp \
../LLoginServerPacketProcess_Proc.cpp \
../main.cpp 

OBJS += \
./LClient.o \
./LClientManager.o \
./LConnectToAccountDBServer.o \
./LConnectToMaster.o \
./LLSServer.o \
./LLSServerManager.o \
./LLoginServerConnectToServerNetWork.o \
./LLoginServerMainLogic.o \
./LLoginServerPacketProcess_AccountDB.o \
./LLoginServerPacketProcess_Client.o \
./LLoginServerPacketProcess_Master.o \
./LLoginServerPacketProcess_Proc.o \
./main.o 

CPP_DEPS += \
./LClient.d \
./LClientManager.d \
./LConnectToAccountDBServer.d \
./LConnectToMaster.d \
./LLSServer.d \
./LLSServerManager.d \
./LLoginServerConnectToServerNetWork.d \
./LLoginServerMainLogic.d \
./LLoginServerPacketProcess_AccountDB.d \
./LLoginServerPacketProcess_Client.d \
./LLoginServerPacketProcess_Master.d \
./LLoginServerPacketProcess_Proc.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/home/wenshengming/workspace/work/Servers/NetWork" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


