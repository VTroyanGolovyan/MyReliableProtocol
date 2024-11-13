#include "TcpPackageHeader.hpp"

#include <cstdlib>
#include <cstring>
#include <iostream>
namespace vhtcp {


TcpPackageHeader MakePackageHeader(uint64_t id) {
    TcpPackageHeader header;
    std::memset(&header, 0, sizeof(header));
    header.data_offset = sizeof(header);
    header.id = id;
    header.ACK = false;
    header.MSG = false;
    return header;
}

size_t PackageToBytes(const TcpPackageHeader& header, void* buffer) {
    std::memcpy(buffer, &header, sizeof(header));
    return sizeof(header);
}

size_t PackageToBytes(const Package& package, void* buffer) {
    const auto& header = package.header;
    const auto& data = package.data;
    std::memcpy(buffer, &header, sizeof(header));
    std::memcpy(reinterpret_cast<char*>(buffer) + header.data_offset, data.data(), data.length());
    return sizeof(header) + data.length();
}

std::optional<Package> BytesToPackage(void* buffer, size_t len) {
    if (len < sizeof(TcpPackageHeader)) {
        return {};
    }
    Package result;
    std::memcpy(&result.header, buffer, sizeof(TcpPackageHeader));
    std::string result_data;
    if (len > sizeof(TcpPackageHeader)) {
        char* data_begin = reinterpret_cast<char*>(buffer) + result.header.data_offset;
        char* data_end = reinterpret_cast<char*>(buffer) + len;

        std::copy(data_begin, data_end, std::back_inserter(result_data));
    }
    result.data = result_data;
    return result;
}

} // vhtcp