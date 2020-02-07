#ifndef __CELL__H__
#define __CELL__H__

#ifdef _WIN32
#define FD_SETSIZE 1024
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include <unistd.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#define SOCKET int
#define closesocket close
#define INVALID_SOCKET (SOCKET)(~0)
#define SOCKET_ERROR (-1)
#endif

//Buf minimum size 
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240
#define SEND_BUFF_SIZE RECV_BUFF_SIZE
#endif

#include <vector>
#include <string.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <map>
#include <signal.h>
#include <memory>
#include <functional>

#include "Allocator.h"
#include "messageHeader.hpp"
#include "CellLogger.hpp"

#endif  //!__CELL__H__