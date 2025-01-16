#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <nlohmann/json.hpp>

#include <iostream>
#include <memory>

namespace beast = boost::beast;
namespace ws = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

void doSession(tcp::socket socket, net::yield_context yield) {
  try {
    ws::stream<tcp::socket> ws{std::move(socket)};

    // Accept the WebSocket handshake
    ws.async_accept(yield);

    beast::flat_buffer buffer;

    for (;;) {
      // Read a message
      ws.async_read(buffer, yield);

      // Echo the message back
      ws.text(ws.got_text());
      ws.async_write(buffer.data(), yield);

      // Clear the buffer
      buffer.consume(buffer.size());
    }
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}

void doListen(net::io_context& ioc, tcp::endpoint endpoint, net::yield_context yield) {
  tcp::acceptor acceptor{ioc};

  // Open the acceptor
  acceptor.open(endpoint.protocol());

  // Bind to the server address
  acceptor.bind(endpoint);

  // Start listening for connections
  acceptor.listen();

  for (;;) {
    beast::error_code ec;
    tcp::socket socket{ioc};
    acceptor.async_accept(socket, yield[ec]);
    if (!ec) {
      net::spawn(
        ioc, [socket = std::move(socket)](net::yield_context yield) mutable { doSession(std::move(socket), yield); });
    }
  }
}

int main() {
  try {
    const auto address = net::ip::make_address("0.0.0.0");
    unsigned short port = 5600;

    net::io_context ioc;

    // Launch the listening coroutine
    net::spawn(ioc, [&ioc, ep = tcp::endpoint{address, port}](auto yc) { doListen(ioc, ep, yc); });

    // Run the I/O service
    ioc.run();
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}