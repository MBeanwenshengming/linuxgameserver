################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../CRC32.cpp \
../LAcceptThread.cpp \
../LAutoReleaseMutex.cpp \
../LCloseSocketThread.cpp \
../LConnector.cpp \
../LConnectorWorkManager.cpp \
../LEpollThread.cpp \
../LEpollThreadManager.cpp \
../LErrorWriter.cpp \
../LFixLenCircleBuf.cpp \
../LIniFileReadAndWrite.cpp \
../LListenSocket.cpp \
../LMainLogicBroadCast.cpp \
../LMainLogicThread.cpp \
../LMainTest.cpp \
../LNetWorkConfigFileProcessor.cpp \
../LNetWorkServices.cpp \
../LNetWorkServicesIdleSendPacket_Recycle.cpp \
../LPacketBase.cpp \
../LPacketBroadCast.cpp \
../LPacketPoolManager.cpp \
../LPacketProcessBaseModel.cpp \
../LPacketSingle.cpp \
../LRecvThread.cpp \
../LRecvThreadManager.cpp \
../LSelectServer.cpp \
../LSendThread.cpp \
../LSendThreadManager.cpp \
../LServerBaseNetWork.cpp \
../LSession.cpp \
../LSessionManager.cpp \
../LSocket.cpp \
../LThreadBase.cpp \
../LWorkItem.cpp \
../LWorkItemPoolManager.cpp \
../LWorkQueueManager.cpp 

OBJS += \
./CRC32.o \
./LAcceptThread.o \
./LAutoReleaseMutex.o \
./LCloseSocketThread.o \
./LConnector.o \
./LConnectorWorkManager.o \
./LEpollThread.o \
./LEpollThreadManager.o \
./LErrorWriter.o \
./LFixLenCircleBuf.o \
./LIniFileReadAndWrite.o \
./LListenSocket.o \
./LMainLogicBroadCast.o \
./LMainLogicThread.o \
./LMainTest.o \
./LNetWorkConfigFileProcessor.o \
./LNetWorkServices.o \
./LNetWorkServicesIdleSendPacket_Recycle.o \
./LPacketBase.o \
./LPacketBroadCast.o \
./LPacketPoolManager.o \
./LPacketProcessBaseModel.o \
./LPacketSingle.o \
./LRecvThread.o \
./LRecvThreadManager.o \
./LSelectServer.o \
./LSendThread.o \
./LSendThreadManager.o \
./LServerBaseNetWork.o \
./LSession.o \
./LSessionManager.o \
./LSocket.o \
./LThreadBase.o \
./LWorkItem.o \
./LWorkItemPoolManager.o \
./LWorkQueueManager.o 

CPP_DEPS += \
./CRC32.d \
./LAcceptThread.d \
./LAutoReleaseMutex.d \
./LCloseSocketThread.d \
./LConnector.d \
./LConnectorWorkManager.d \
./LEpollThread.d \
./LEpollThreadManager.d \
./LErrorWriter.d \
./LFixLenCircleBuf.d \
./LIniFileReadAndWrite.d \
./LListenSocket.d \
./LMainLogicBroadCast.d \
./LMainLogicThread.d \
./LMainTest.d \
./LNetWorkConfigFileProcessor.d \
./LNetWorkServices.d \
./LNetWorkServicesIdleSendPacket_Recycle.d \
./LPacketBase.d \
./LPacketBroadCast.d \
./LPacketPoolManager.d \
./LPacketProcessBaseModel.d \
./LPacketSingle.d \
./LRecvThread.d \
./LRecvThreadManager.d \
./LSelectServer.d \
./LSendThread.d \
./LSendThreadManager.d \
./LServerBaseNetWork.d \
./LSession.d \
./LSessionManager.d \
./LSocket.d \
./LThreadBase.d \
./LWorkItem.d \
./LWorkItemPoolManager.d \
./LWorkQueueManager.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -D__EPOLL_TEST_STATISTIC__ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


