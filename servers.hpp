#include "vhtcp/vhtcp.hpp"

#include <vector>
#include <random>
#include <algorithm>
#include <cassert>

std::string RandomMsg(size_t n) {
    static const char alphabet[] =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789";
    
    std::random_device rd;
    std::default_random_engine rng(rd());
    std::uniform_int_distribution<> dist(0,sizeof(alphabet)/sizeof(*alphabet)-2);

    std::string msg;

    std::generate_n(std::back_inserter(msg), n, [&]() { return alphabet[dist(rng)];}); 

    return msg;

}
class EchoBase {
    public:
        EchoBase(vhtcp::Socket& sock, size_t iterations, size_t msg_size) : sock(sock), iterations(iterations), msg_size(msg_size) {}

    protected:
        vhtcp::Socket& sock;
        size_t iterations;
        size_t msg_size;
};

class EchoServer : public EchoBase {
public:
    EchoServer(vhtcp::Socket& sock, size_t iterations, size_t msg_size) : EchoBase(sock, iterations, msg_size) {
    }

    void run() {
        for (size_t i = 0; i < iterations; ++i) {
            auto msg = sock.Receive(msg_size);
            sock.Send(msg);
        }
    }
};

class EchoClient : public EchoBase {
public:
    EchoClient(vhtcp::Socket& sock, size_t iterations, size_t msg_size) : EchoBase(sock, iterations, msg_size) {
    }

    void run() {
        auto msg = RandomMsg(msg_size);
        for (size_t i = 0; i < iterations; ++i) {
            sock.Send(msg);
            auto received = sock.Receive(msg_size);
            if (msg != received) {
                std::cout << received << std::endl;
                std::cout << received << std::endl;
                std::this_thread::sleep_for(1s);
            }
            assert(msg == received);
        }
    }
};

class ParallelClientServer : public EchoBase {
public:
    ParallelClientServer(vhtcp::Socket& sock, size_t iterations, size_t msg_size) : EchoBase(sock, iterations, msg_size) {
    }

    void run() {
        size_t n = 0;
        auto msg = RandomMsg(msg_size);
        for (size_t i = 0; i < iterations; ++i) {
            n += msg_size;
            sock.Send(msg);
        }
        size_t received = 0;
        for (size_t i = 0; i < iterations; ++i) {
            auto msg = sock.Receive(msg_size);
            received += msg.length();
        }
        assert(n == received);
    }
};