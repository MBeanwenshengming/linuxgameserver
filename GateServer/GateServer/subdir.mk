################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../LClient.cpp \
../LClientManager.cpp \
../LConnectToMasterServer.cpp \
../LConnectToServerNetWork.cpp \
../LGateServerLogicProcess.cpp \
../LGateServerMainLogic.cpp \
../LGateServerPacketProcess_Client_Lobby.cpp \
../LGateServerPacketProcess_Client_Login.cpp \
../LGateServerPacketProcess_FromMasterServer.cpp \
../LGateServerPacketProcess_Proc.cpp \
../LServer.cpp \
../LServerInfo.cpp \
../LServerManager.cpp \
../main.cpp 

OBJS += \
./LClient.o \
./LClientManager.o \
./LConnectToMasterServer.o \
./LConnectToServerNetWork.o \
./LGateServerLogicProcess.o \
./LGateServerMainLogic.o \
./LGateServerPacketProcess_Client_Lobby.o \
./LGateServerPacketProcess_Client_Login.o \
./LGateServerPacketProcess_FromMasterServer.o \
./LGateServerPacketProcess_Proc.o \
./LServer.o \
./LServerInfo.o \
./LServerManager.o \
./main.o 

CPP_DEPS += \
./LClient.d \
./LClientManager.d \
./LConnectToMasterServer.d \
./LConnectToServerNetWork.d \
./LGateServerLogicProcess.d \
./LGateServerMainLogic.d \
./LGateServerPacketProcess_Client_Lobby.d \
./LGateServerPacketProcess_Client_Login.d \
./LGateServerPacketProcess_FromMasterServer.d \
./LGateServerPacketProcess_Proc.d \
./LServer.d \
./LServerInfo.d \
./LServerManager.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/home/wenshengming/workspace/work/Servers/NetWork" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


