#include "TimeoutedUdpSocket.hpp"

namespace helpers {

TimeoutedUdpSocket::TimeoutedUdpSocket(const udp::endpoint& endpoint, boost::posix_time::time_duration timeout) 
    : socket_(io_service_, endpoint), deadline_(io_service_), timeout_(timeout) {
    deadline_.expires_at(boost::posix_time::pos_infin);
    CheckDeadline();
}

std::size_t TimeoutedUdpSocket::Receive(boost::asio::mutable_buffer buffer, udp::endpoint& remote) {
    deadline_.expires_from_now(timeout_);

    boost::system::error_code ec = boost::asio::error::would_block;
    std::size_t length = 0;

    socket_.async_receive_from(boost::asio::buffer(buffer), remote, boost::bind(&TimeoutedUdpSocket::HandleReceive, _1, _2, &ec, &length));
    
    do { 
        io_service_.run_one(); 
    } while (ec == boost::asio::error::would_block);

    return length;
}

void TimeoutedUdpSocket::SendTo(boost::asio::const_buffer buffer, const udp::endpoint& destination) {
    socket_.send_to(buffer, destination);
}

udp::socket& TimeoutedUdpSocket::GetSocket() {
    return socket_;
}

void TimeoutedUdpSocket::CheckDeadline() {

    if (deadline_.expires_at() <= deadline_timer::traits_type::now()) {
        socket_.cancel();
        deadline_.expires_at(boost::posix_time::pos_infin);
    }

    deadline_.async_wait(boost::bind(&TimeoutedUdpSocket::CheckDeadline, this));

}

void TimeoutedUdpSocket::HandleReceive(const boost::system::error_code& ec, std::size_t length, boost::system::error_code* out_ec, std::size_t* out_length) {
    *out_ec = ec;
    *out_length = length;
}

} // helpers
