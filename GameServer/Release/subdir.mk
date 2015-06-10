################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../LGSClient.cpp \
../LGSClientManager.cpp \
../LGSClientServer.cpp \
../LGSClientServerManager.cpp \
../LGSPacketProcessManager.cpp \
../LGSPacketProcess_Common.cpp \
../LGSServer.cpp \
../LGSServerManager.cpp \
../LGameServerConnectToMasterServer.cpp \
../LGameServerConnectToServerNetWork.cpp \
../LGameServerMainLogic.cpp \
../main.cpp 

OBJS += \
./LGSClient.o \
./LGSClientManager.o \
./LGSClientServer.o \
./LGSClientServerManager.o \
./LGSPacketProcessManager.o \
./LGSPacketProcess_Common.o \
./LGSServer.o \
./LGSServerManager.o \
./LGameServerConnectToMasterServer.o \
./LGameServerConnectToServerNetWork.o \
./LGameServerMainLogic.o \
./main.o 

CPP_DEPS += \
./LGSClient.d \
./LGSClientManager.d \
./LGSClientServer.d \
./LGSClientServerManager.d \
./LGSPacketProcessManager.d \
./LGSPacketProcess_Common.d \
./LGSServer.d \
./LGSServerManager.d \
./LGameServerConnectToMasterServer.d \
./LGameServerConnectToServerNetWork.d \
./LGameServerMainLogic.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/home/wenshengming/workspace/GSAR/trunk/NetWork" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


