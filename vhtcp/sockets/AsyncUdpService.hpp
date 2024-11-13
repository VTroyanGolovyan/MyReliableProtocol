#include <boost/asio.hpp>

#include "../TcpPackageHeader.hpp"
#include "helpers/TimeoutedUdpSocket.hpp"
#include <functional>
#include <future>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <unordered_map>
#include <iostream>
#include <deque>
#include <algorithm>
using namespace std::chrono_literals;

using boost::asio::ip::udp;

using namespace vhtcp;

namespace impl {

class AsyncUdpService {
public:
    AsyncUdpService(const udp::endpoint& self, const udp::endpoint& remote) : socket_(self), remote_(remote) {
    }

    void Start() {
        reader_thread_ = std::make_unique<std::thread>([this]() {
            ReaderThread();
        });
        writer_thread_ = std::make_unique<std::thread>([this]() {
            WriterThread();
        });
    }

    void Stop() {
        stopped_ = true;
        reader_thread_->join();
        writer_thread_->join();
        socket_.GetSocket().close();
    }

    void ReaderThread() {
        PackageBuffer buff;
        while (!stopped_) {
            try {
                size_t len = socket_.Receive(boost::asio::buffer(buff, buff.size()), remote_);
                if (stopped_) { break; }

                auto package = BytesToPackage(buff.data(), len);

                if (package) {
                    HandleReceivedPackage(*package);

                    if (package->header.MSG) {
                        bool need_ack = false;
                        while (receive_cache_.contains(expected_receive_id)) {
                            auto& cahced = receive_cache_[expected_receive_id];
                            if (cahced.data.length() > 0) {
                                std::lock_guard lk(receive_m_);
                                received_data_ += cahced.data;
                                receive_cv_.notify_all();
                            }
                            receive_cache_.erase(expected_receive_id);
                            ++expected_receive_id;
                            need_ack = true;
                        }
                        if (need_ack) {
                            Package pack;
                            pack.header = MakePackageHeader(1);
                            pack.header.ACK = true;
                            pack.header.ack_id = expected_receive_id - 1;
                            size_t cnt = PackageToBytes(pack, buff.data());
                            socket_.SendTo(boost::asio::buffer(buff.data(), cnt), remote_);
                        }
                        
                    }
                    
                }
            } catch (...) {}
            
        }
    }

    void HandleReceivedPackage(Package& package) {
        PackageBuffer buff;
        
        if (package.header.ACK && package.header.ack_id >= sequence_id) {
            std::lock_guard<std::mutex> lk(m_);
            if (!sending_queue_.empty()) {
                size_t pop_n = std::min(package.header.ack_id - sequence_id + 1, static_cast<uint64_t>(sending_queue_.size()));
                for (size_t i = 0; i < pop_n; ++i) {
                    sending_queue_.pop_front();
                }
            }
            sequence_id = package.header.ack_id + 1;
            cv_.notify_all();
        }
        if (package.header.MSG && package.header.id == expected_receive_id) {
            if (package.data.length() > 0) {
                std::lock_guard lk(receive_m_);
                received_data_ += package.data;
                receive_cv_.notify_all();
            }
            ++expected_receive_id;
            if (package.data.length() > 0) {
                Package pack;
                pack.header = MakePackageHeader(1);
                pack.header.ACK = true;
                pack.header.ack_id = expected_receive_id - 1;
                size_t cnt = PackageToBytes(pack, buff.data());
                socket_.SendTo(boost::asio::buffer(buff.data(), cnt), remote_);
            }
        } else if (package.header.MSG) {
            receive_cache_[package.header.id] = package;
        }
        
    }

    void WriterThread() {
        PackageBuffer buff; 
        size_t last_size = 100;
        while (!stopped_) {
            try {

                std::deque<Package> current_sendings;
                {
                    std::unique_lock<std::mutex> lk(m_);
                    if (!sending_queue_.empty()) {
                        current_sendings = sending_queue_;
                    }
                }
                if (!current_sendings.empty()) {
                    for (auto& pack : current_sendings) {
                        if (expected_receive_id > 0) {
                            pack.header.ACK = true;
                            pack.header.ack_id = expected_receive_id - 1;
                        }
                        size_t cnt = PackageToBytes(pack, buff.data());
                        socket_.SendTo(boost::asio::buffer(buff.data(), cnt), remote_);
                        last_size = pack.data.length();
                        std::this_thread::sleep_for(1000us);
                    }
                } else {
                    Package ack;
                    ack.header = MakePackageHeader(0);
                    ack.header.ACK = true;
                    ack.header.ack_id = expected_receive_id - 1;
                    size_t cnt = PackageToBytes(ack, buff.data());
                    socket_.SendTo(boost::asio::buffer(buff.data(), cnt), remote_);
                    std::this_thread::sleep_for(1000us);
                }
                
                
            } catch(...) {}
        }
    }

    void Send(std::string data) {
        std::unique_lock<std::mutex> lk(m_);
        PrepareForPackages(data);
        cv_.wait(lk, [this](){
            return sending_queue_.empty();
        });
    }

    std::string Read(size_t data_byte_cnt) {
       std::unique_lock<std::mutex> lk(receive_m_);
       
       receive_cv_.wait(lk, [this, &data_byte_cnt](){
            return received_data_.length() >= data_byte_cnt;
       });
       
       std::string res = received_data_.substr(0, data_byte_cnt);
       received_data_ = received_data_.substr(data_byte_cnt);
       return res;
    }

    size_t CalculateNeededPackageCnt(size_t byte_cnt) {
        size_t packages_cnt = byte_cnt / kPackageMaxDataSize;
        packages_cnt += (byte_cnt % kPackageMaxDataSize != 0) ? 1 : 0;
        return packages_cnt;
    }

    void PrepareForPackages(const std::string& str) {
        size_t n = CalculateNeededPackageCnt(str.length());
        PackageBuffer buff;
        for (size_t i = 0; i < n; ++i) {
            Package pack;
            pack.header = MakePackageHeader(sequence_id + i);
            pack.header.MSG = true;
            pack.data = str.substr(i * kPackageMaxDataSize, std::min(kPackageMaxDataSize, str.length()));
            // fast path
            size_t cnt = PackageToBytes(pack, buff.data());
            socket_.SendTo(boost::asio::buffer(buff.data(), cnt), remote_);
            sending_queue_.push_back(pack);
        }
    }

private:

    std::unordered_map<uint64_t, Package> receive_cache_;

    std::unique_ptr<std::thread> reader_thread_;
    std::unique_ptr<std::thread> writer_thread_;

    helpers::TimeoutedUdpSocket socket_;
    udp::endpoint remote_;      

    std::condition_variable cv_;
    std::mutex m_;

    std::condition_variable receive_cv_;
    std::mutex receive_m_;

    std::deque<Package> sending_queue_;
    std::atomic<uint64_t> sequence_id{0};
    std::atomic<uint64_t> expected_receive_id{0};
    
    std::string received_data_;
    std::atomic<bool> stopped_{false};

};

}
