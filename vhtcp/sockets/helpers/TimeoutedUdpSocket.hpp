#ifndef TIMEOUTEDSOCKET_HPP
#define TIMEOUTEDSOCKET_HPP

#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/bind/bind.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

using boost::asio::deadline_timer;
using boost::asio::ip::udp;
using namespace boost::posix_time;
using namespace boost::placeholders;

namespace helpers {

class TimeoutedUdpSocket {
public:
  explicit TimeoutedUdpSocket(const udp::endpoint& endpoint, boost::posix_time::time_duration timeout = milliseconds(1));

  std::size_t Receive(boost::asio::mutable_buffer buffer, udp::endpoint& remote);

  void SendTo(const boost::asio::const_buffer buffers, const udp::endpoint& destination);

  udp::socket& GetSocket();
private:
  void CheckDeadline();

  static void HandleReceive(const boost::system::error_code& ec, std::size_t length, boost::system::error_code* out_ec, std::size_t* out_length);

private:
  boost::asio::io_service io_service_;
  boost::posix_time::time_duration timeout_;
  udp::socket socket_;
  deadline_timer deadline_;
};

} // helpers

#endif
