#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/fiber/all.hpp>
#include <boost/program_options.hpp>
#include <nlohmann/json.hpp>
#include <fmt/core.h>

#include "round_robin.hpp"
#include "yield.hpp"

namespace beast = boost::beast;
namespace ws = beast::websocket;
namespace net = boost::asio;
namespace fib = boost::fibers;
namespace po = boost::program_options;

using tcp = net::ip::tcp;

void doSession(tcp::socket socket) {
  static int total_sessions = 0;

  total_sessions++;
  fmt::println("Start new session, total: {}", total_sessions);
  try {
    ws::stream<tcp::socket> ws{std::move(socket)};

    // Accept the WebSocket handshake
    ws.async_accept(boost::fibers::asio::yield);

    beast::flat_buffer buffer;

    for (;;) {
      // Read a message
      ws.async_read(buffer, boost::fibers::asio::yield);

      // Echo the message back
      ws.text(ws.got_text());
      ws.async_write(buffer.data(), boost::fibers::asio::yield);

      // Clear the buffer
      buffer.consume(buffer.size());
    }
  } catch (const std::exception& e) {
    total_sessions--;
    fmt::println("Finish session: {}, total: {}", e.what(), total_sessions);
  }
}

void doListen(net::io_context& ioc, tcp::endpoint endpoint) {
  fmt::println("Listening fiber started");

  try {
    tcp::acceptor acceptor{ioc};

    fmt::println("Open the acceptor");
    acceptor.open(endpoint.protocol());

    fmt::println("Bind to the server address");
    acceptor.bind(endpoint);

    fmt::println("Start listening for connections");
    acceptor.listen();

    for (;;) {
      boost::system::error_code ec;
      tcp::socket socket{ioc};
      acceptor.async_accept(socket, boost::fibers::asio::yield[ec]);
      if (!ec) {
        fib::fiber(doSession, std::move(socket)).detach();
      } else {
        fmt::println("Accept error: {}", ec.message());
      }
    }
  } catch (const std::exception& e) {
    fmt::print("Error: {}", e.what());
  }
}

int main(int argc, char** argv) {
  try {
    const auto address = net::ip::make_address("0.0.0.0");
    unsigned short port = 0;

    po::options_description desc{"Allowed options"};
    desc.add_options()
      ("port,p", po::value<unsigned short>(&port)->default_value(5600))
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    auto ioc = std::make_shared<net::io_context>();
    fib::use_scheduling_algorithm<boost::fibers::asio::round_robin>(ioc);

    fmt::println("Launch listening fiber");
    fib::fiber(doListen, std::ref(*ioc), tcp::endpoint{address, port}).detach();

    fmt::println("Listening on port {}", port);
    ioc->run();
  } catch (const std::exception& e) {
    fmt::println("Error: {}", e.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}