################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../LMainLogicThread.cpp \
../LMasterServerMainLogic.cpp \
../LMasterServerPacketProcess_Common.cpp \
../LMasterServerPacketProcess_GameServer.cpp \
../LMasterServerPacketProcess_GateServer.cpp \
../LMasterServerPacketProcess_LobbyServer.cpp \
../LMasterServerPacketProcess_LoginServer.cpp \
../LMasterServer_PacketProcess_Proc.cpp \
../LServer.cpp \
../LServerManager.cpp \
../LUser.cpp \
../LUserManager.cpp \
../main.cpp 

OBJS += \
./LMainLogicThread.o \
./LMasterServerMainLogic.o \
./LMasterServerPacketProcess_Common.o \
./LMasterServerPacketProcess_GameServer.o \
./LMasterServerPacketProcess_GateServer.o \
./LMasterServerPacketProcess_LobbyServer.o \
./LMasterServerPacketProcess_LoginServer.o \
./LMasterServer_PacketProcess_Proc.o \
./LServer.o \
./LServerManager.o \
./LUser.o \
./LUserManager.o \
./main.o 

CPP_DEPS += \
./LMainLogicThread.d \
./LMasterServerMainLogic.d \
./LMasterServerPacketProcess_Common.d \
./LMasterServerPacketProcess_GameServer.d \
./LMasterServerPacketProcess_GateServer.d \
./LMasterServerPacketProcess_LobbyServer.d \
./LMasterServerPacketProcess_LoginServer.d \
./LMasterServer_PacketProcess_Proc.d \
./LServer.d \
./LServerManager.d \
./LUser.d \
./LUserManager.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/home/wenshengming/workspace/GSAR/trunk/NetWork" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


