PROJECT_PATH = $(PWD)
HEADER_PATH = -I ${PROJECT_PATH}/../ \
			  -I ${PROJECT_PATH}/../easylogging++/

LIB_PATH += ${PROJECT_PATH}/libmuduo_library_static.a

CFLAGS= -g -Wall  ${LIB_PATH} ${HEADER_PATH} -std=c++11

all : EchoServer CrossThreadTest EventLoopThreadPoolTest
	
EchoServer : echoServer.cc
	g++ echoServer.cc ${PROJECT_PATH}/../easylogging++/easylogging++.cpp ${CFLAGS} -o EchoServer

CrossThreadTest : crossThreadTest.cpp
	g++ crossThreadTest.cpp ${PROJECT_PATH}/../easylogging++/easylogging++.cpp ${CFLAGS} -o CrossThreadTest

EventLoopThreadPoolTest : eventLoopThread_unitTest.cpp
	g++ eventLoopThread_unitTest.cpp ${PROJECT_PATH}/../easylogging++/easylogging++.cpp ${CFLAGS} -o EventLoopThreadPoolTest
clean :
	rm -rf *.o EchoServer CrossThreadTest EventLoopThreadPoolTest