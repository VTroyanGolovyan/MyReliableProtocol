
#include "servers.hpp"
#include <iostream>
#include <chrono>

void SetupNetem(double packet_loss, double duplicate, double reorder) {
    std::string netem_cmd = "tc qdisc replace dev lo root netem loss " + std::to_string(int(packet_loss * 100)) + "% duplicate " + std::to_string(int(duplicate * 100)) + "%";

    if (reorder > 0) {
        netem_cmd += " reorder " + std::to_string(100 - reorder) + "%";
    }
    netem_cmd += " delay 10ms rate 1Mbit";
    std::cout << netem_cmd << std::endl;
    system(netem_cmd.c_str());
}

template <class FirstActor, class SecondActor>
void RunTest(size_t iterations, size_t msg_size) {
    static size_t port = 28500;
    udp::endpoint first_endpoint(udp::v4(), port++);
    udp::endpoint second_endpoint(udp::v4(), port++);

    vhtcp::Socket a(first_endpoint, second_endpoint);
    vhtcp::Socket b(second_endpoint, first_endpoint);

    std::thread th1([&]() {
        FirstActor x(a, iterations, msg_size);
        x.run();
    });


    std::thread th2([&]() {
        SecondActor x(b, iterations, msg_size);
        x.run();
    });

    th1.join();
    th2.join();

    a.Close();
    b.Close();
}

bool TestBasic(double tl) {
    std::cout << "TestBasic started" << std::endl;
    SetupNetem(0.0, 0.0, 0.0);
    for (size_t iterations : { 10, 50, 100 }) {
        auto start = std::chrono::high_resolution_clock::now();
        RunTest<EchoServer, EchoClient>(iterations, 11);
        std::cout << "  - TestBasic OK with iterations:" << iterations << std::endl;
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        std::cout << "     Time taken: " << duration.count() << " seconds, Limit: " << tl << std::endl;
        if (duration.count() > tl) {
            return false;
        }
    }
    std::cout << "TestBasic full OK" << std::endl;
    return true;
}

bool TestSmallLoss(double tl) {
    std::cout << "TestSmallLoss started" << std::endl;
    SetupNetem(0.02, 0.0, 0.0);
    for (size_t iterations : { 10, 100, 500 }) {
        auto start = std::chrono::high_resolution_clock::now();
        RunTest<EchoServer, EchoClient>(iterations, 14);
        std::cout << "  - Small loss OK with iterations:" << iterations << std::endl;
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        std::cout << "     Time taken: " << duration.count() << " seconds, Limit: " << tl << std::endl;
        if (duration.count() > tl) {
            return false;
        }
    }
    std::cout << "Test Small loss full OK" << std::endl;
    return true;
}

bool TestSmallDuplicate(double tl) {
    std::cout << "TestSmallDuplicate started" << std::endl;
    SetupNetem(0.00, 0.02, 0.0);
    for (size_t iterations : { 10, 100, 500 }) {
        auto start = std::chrono::high_resolution_clock::now();
        RunTest<EchoServer, EchoClient>(iterations, 14);
        std::cout << "  - TestSmallDuplicate OK with iterations:" << iterations << std::endl;
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        std::cout << "     Time taken: " << duration.count() << " seconds, Limit: " << tl << std::endl;
        if (duration.count() > tl) {
            return false;
        }
    }
    std::cout << "TestSmallDuplicate full OK" << std::endl;
    return true;
}

bool TestHighLoss(double tl) {
    std::cout << "TestHighLoss started" << std::endl;
    SetupNetem(0.1, 0.0, 0.0);
    for (size_t iterations : { 10, 100, 500 }) {
        auto start = std::chrono::high_resolution_clock::now();
        RunTest<EchoServer, EchoClient>(iterations, 17);
        std::cout << "  - TestHighLoss OK with iterations:" << iterations << std::endl;
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        std::cout << "     Time taken: " << duration.count() << " seconds, Limit: " << tl << std::endl;
        if (duration.count() > tl) {
            return false;
        }
    }
    std::cout << "TestHighLoss full OK" << std::endl;
    return true;
}

bool TestHighDuplicate(double tl) {
    std::cout << "TestHighDuplicate started" << std::endl;
    SetupNetem(0.0, 0.1, 0.0);
    for (size_t iterations : { 10, 100, 500 }) {
        auto start = std::chrono::high_resolution_clock::now();
        RunTest<EchoServer, EchoClient>(iterations, 17);
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "  - TestHighDuplicate OK with iterations:" << iterations << std::endl;
        std::chrono::duration<double> duration = end - start;
        std::cout << "     Time taken: " << duration.count() << " seconds, Limit: " << tl << std::endl;
        if (duration.count() > tl) {
            return false;
        }
    }
    std::cout << "TestHighDuplicate full OK" << std::endl;
    return true;
}

bool TestLargeMsg(double tl) {
    std::cout << "TestLargeMsg started" << std::endl;
    SetupNetem(0.02, 0.02, 0.01);
    for (size_t msg_size : { 100, 100000, 5000000 }) {
        auto start = std::chrono::high_resolution_clock::now();
        RunTest<EchoServer, EchoClient>(2, msg_size);
        std::cout << "  - TestLargeMsg OK with msgsize:" << msg_size << std::endl;
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        std::cout << "     Time taken: " << duration.count() << " seconds, Limit: " << tl << std::endl;
        if (duration.count() > tl) {
            return false;
        }
    }
    std::cout << "TestLargeMsg full OK" << std::endl;
    return true;
}

bool TestParallel(double tl) {
    std::cout << "TestParallel started" << std::endl;
    SetupNetem(0.0, 0.0, 0.0);
    for (size_t iterations : { 10, 100, 500 }) {
        auto start = std::chrono::high_resolution_clock::now();
        RunTest<ParallelClientServer, ParallelClientServer>(iterations, 5);
        std::cout << "TestParallel OK with iterations:" << iterations << std::endl;
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        std::cout << "     Time taken: " << duration.count() << " seconds, Limit: " << tl << std::endl;
        if (duration.count() > tl) {
            return false;
        }
    }
    std::cout << "Test TestParallel OK" << std::endl;
    return true;
}

bool TestPerformance(double tl) {
    std::cout << "TestPerformance started" << std::endl;
    SetupNetem(0.02, 0.02, 0.01);
    for (size_t iterations : { 50000 }) {
        auto start = std::chrono::high_resolution_clock::now();
        RunTest<EchoServer, EchoClient>(iterations, 10);
        std::cout << "TestPerformance OK with iterations:" << iterations << std::endl;
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        std::cout << "     Time taken: " << duration.count() << " seconds, Limit: " << tl << std::endl;
        if (duration.count() > tl) {
            return false;
        }
    }
    std::cout << "TestPerformance OK" << std::endl;
    return true;
}

void PrintMark(double sum) {
    std::cout << "=====================================" << std::endl;
    std::cout << "|    YOUR MARK NOW IS: " << sum << std::endl;
    std::cout << "=====================================" << std::endl;
}
int main() {
    double sum = 0;
    std::vector<std::unique_lock<std::mutex>> lks;
    std::mutex m1;
    std::mutex m2;
    lks.emplace_back(m1);
    lks.emplace_back(m2);
    
    if (TestBasic(30) && TestSmallDuplicate(30) && TestSmallLoss(30)) {
        sum += 0.5;
    }
    PrintMark(sum);
    if (TestHighDuplicate(30) && TestHighLoss(30)) {
        sum += 0.5;
    }
    PrintMark(sum);
    if (TestParallel(20)) {
        sum += 0.5;
    } 
    PrintMark(sum);

    if (TestLargeMsg(180)) {
       sum += 0.5;
    }
    PrintMark(sum);
    
    if (TestPerformance(500)) {
        sum += 1.0;
    } 
    PrintMark(sum);

    

    return 0;
}