#ifndef BASE_SOCKET_HPP
#define BASE_SOCKET_HPP

#include "AsyncUdpService.hpp"
#include "../TcpPackageHeader.hpp"

#include <random>
#include <limits>
#include <cassert>
#include <vector>
#include <set>
#include <iostream>

namespace vhtcp {

class Socket {
public:
    Socket(udp::endpoint self, udp::endpoint remote);

    std::string Receive(size_t n);

    void Send(std::string data);

    void Close();
protected:

    size_t CalculateNeededPackageCnt(size_t byte_cnt);

    std::vector<Package> SplitForPackages(const std::string& str);

    impl::AsyncUdpService udp_service_;
    
};

} // vhtcp

#endif