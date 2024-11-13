#ifndef TCP_PACKAGE_HEADER_HPP
#define TCP_PACKAGE_HEADER_HPP

#include <algorithm>
#include <cstdint>
#include <cstddef>
#include <string>
#include <optional>

namespace vhtcp {

struct  __attribute__((packed)) TcpPackageHeader {
    uint64_t id;
    uint64_t ack_id;
    uint32_t data_offset;
    uint32_t ACK : 1;
    uint32_t MSG : 1;
};

struct Package {
    TcpPackageHeader header;
    std::string data;
};

static constexpr size_t kHeaderSize = sizeof(TcpPackageHeader);
static constexpr size_t kPackageMaxSize = 50000;
static constexpr size_t kPackageMaxDataSize = kPackageMaxSize - kHeaderSize;
using PackageBuffer = std::array<char, kPackageMaxSize + 100>;

enum PackageType {
    AckPackage,
    MsgPackage
};

TcpPackageHeader MakePackageHeader(uint64_t id);

size_t PackageToBytes(const TcpPackageHeader& header, void* buffer);

size_t PackageToBytes(const Package& package, void* buffer);

std::optional<Package> BytesToPackage(void* buffer, size_t len);

} // vhtcp

#endif
