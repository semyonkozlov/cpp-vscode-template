#include <boost/cobalt.hpp>
#include <boost/cobalt/main.hpp>
#include <boost/asio.hpp>
#include <boost/beast.hpp>

namespace cobalt = boost::cobalt;
namespace beast = boost::beast;
namespace ws = beast::websocket;
using boost::asio::detached;
using boost::asio::ip::tcp;
using tcp_acceptor = cobalt::use_op_t::as_default_on_t<tcp::acceptor>;
using tcp_socket = cobalt::use_op_t::as_default_on_t<tcp::socket>;

cobalt::promise<void> session(tcp_socket socket) {
  try {
    ws::stream<tcp_socket> ws{std::move(socket)};
    co_await ws.async_accept();

    beast::flat_buffer buffer;
    for (;;) {
      std::size_t n = co_await ws.async_read(buffer);
      ws.text(ws.got_text());
      co_await ws.async_write(buffer.data());
      buffer.consume(buffer.size());
    }
  } catch (std::exception& e) {
    std::printf("echo: exception: %s\n", e.what());
  }
}
cobalt::generator<tcp_socket> listen() {
  tcp_acceptor acceptor({co_await cobalt::this_coro::executor}, {tcp::v4(), 5600});
  for (;;) {
    tcp_socket sock = co_await acceptor.async_accept();
    co_yield std::move(sock);
  }
  co_return tcp_socket{acceptor.get_executor()};
}
cobalt::promise<void> run_server(cobalt::wait_group& workers) {
  auto l = listen();
  while (true) {
    if (workers.size() == 10u)
      co_await workers.wait_one();
    else
      workers.push_back(session(co_await l));
  }
}
// end::run_server[]

// tag::main[]
cobalt::main co_main(int argc, char** argv) {
  co_await cobalt::with(cobalt::wait_group(), &run_server);
  co_return 0u;
}
// end::main[]
