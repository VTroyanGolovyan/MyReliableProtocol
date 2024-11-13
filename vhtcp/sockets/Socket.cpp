#include "Socket.hpp"

#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <iostream>

namespace vhtcp {

Socket::Socket(udp::endpoint self, udp::endpoint remote) : udp_service_(self, remote) {
    udp_service_.Start();
}

void Socket::Send(std::string data) {
    udp_service_.Send(data);
}

std::string Socket::Receive(size_t n) {
    return udp_service_.Read(n);
}

void Socket::Close() {
    udp_service_.Stop();
}


} // vhtcp
