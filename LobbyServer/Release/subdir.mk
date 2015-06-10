################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../LLBConnectToGameDBServer.cpp \
../LLBConnectToMasterServer.cpp \
../LLBConnectToServersNetWork.cpp \
../LLBGateServer.cpp \
../LLBGateServerManager.cpp \
../LLBServer.cpp \
../LLBServerManager.cpp \
../LLobbyServerMainLogic.cpp \
../LLobbyServerPacketProcess_FromGateServer.cpp \
../LLobbyServerPacketProcess_FromMaster.cpp \
../LLobbyServerPacketProcess_Proc.cpp \
../LLobbyServerPacketProcess_ServerToServer.cpp \
../LUser.cpp \
../LUserManager.cpp \
../main.cpp 

OBJS += \
./LLBConnectToGameDBServer.o \
./LLBConnectToMasterServer.o \
./LLBConnectToServersNetWork.o \
./LLBGateServer.o \
./LLBGateServerManager.o \
./LLBServer.o \
./LLBServerManager.o \
./LLobbyServerMainLogic.o \
./LLobbyServerPacketProcess_FromGateServer.o \
./LLobbyServerPacketProcess_FromMaster.o \
./LLobbyServerPacketProcess_Proc.o \
./LLobbyServerPacketProcess_ServerToServer.o \
./LUser.o \
./LUserManager.o \
./main.o 

CPP_DEPS += \
./LLBConnectToGameDBServer.d \
./LLBConnectToMasterServer.d \
./LLBConnectToServersNetWork.d \
./LLBGateServer.d \
./LLBGateServerManager.d \
./LLBServer.d \
./LLBServerManager.d \
./LLobbyServerMainLogic.d \
./LLobbyServerPacketProcess_FromGateServer.d \
./LLobbyServerPacketProcess_FromMaster.d \
./LLobbyServerPacketProcess_Proc.d \
./LLobbyServerPacketProcess_ServerToServer.d \
./LUser.d \
./LUserManager.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/home/wenshengming/workspace/GSAR/trunk/NetWork" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


