/*********************************************************** 
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2024-01-26 14:26:08
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2024-02-28 16:43:45
 * @FilePath: /studyMuduo/InetAddress.cpp
 * @Description: 
 * @
 * @Copyright (c) 2024 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved. 
 ************************************************************/
#include "InetAddress.h"
#include <string.h>
#include <iostream>

InetAddress::InetAddress(uint16_t port, std::string ip)
{
    bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());
}

std::string InetAddress::toIp() const
{
    char buf[64] = {0};
    inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    return buf;
}

std::string InetAddress::toIpPort() const
{
    std::string ipPort = toIp();
    uint16_t port = ntohs(addr_.sin_port);
    ipPort = ipPort + " : " + std::to_string(port);
    return ipPort;
}

uint16_t InetAddress::toPort() const
{
    return ntohs(addr_.sin_port);
}


// int main(int argc, char const *argv[])
// {
//     InetAddress addr(8080);

//     std::cout << addr.toIpPort() << std::endl;
//     return 0;
// }
