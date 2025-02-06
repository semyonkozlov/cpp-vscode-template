#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/cobalt.hpp>
#include <boost/cobalt/main.hpp>
#include <boost/program_options.hpp>

#include <fmt/core.h>
#include <nlohmann/json.hpp>

#include <chrono>

namespace cobalt = boost::cobalt;
namespace beast = boost::beast;
namespace ws = beast::websocket;
namespace po = boost::program_options;
namespace this_coro = cobalt::this_coro;
using boost::asio::ip::tcp;
using tcp_acceptor = cobalt::use_op_t::as_default_on_t<tcp::acceptor>;
using tcp_socket = cobalt::use_op_t::as_default_on_t<tcp::socket>;
using steady_timer = cobalt::use_op_t::as_default_on_t<boost::asio::steady_timer>;
using std::chrono::steady_clock;
using namespace std::literals::chrono_literals;

cobalt::promise<void> timeout(steady_clock::duration d) {
  steady_timer timer{co_await this_coro::executor};
  timer.expires_after(d);
  co_await timer.async_wait();
}

cobalt::promise<void> session(tcp_socket socket) {
  static int total_sessions = 0;
  total_sessions++;
  fmt::println("Start new session, total: {}", total_sessions);

  try {
    ws::stream<tcp_socket> ws{std::move(socket)};
    co_await ws.async_accept();

    beast::flat_buffer buffer;
    while (true) {
      // if no input for 5 seconds, raise exception and finish the session
      co_await cobalt::race(ws.async_read(buffer), timeout(5s));

      ws.text(ws.got_text());
      co_await ws.async_write(buffer.data());
      buffer.consume(buffer.size());
    }
  } catch (std::exception& e) {
    total_sessions--;
    fmt::println("Finish session: {}, total: {}", e.what(), total_sessions);
  }
}

cobalt::generator<tcp_socket> listen(unsigned short port) {
  tcp_acceptor acceptor{co_await this_coro::executor, {tcp::v4(), port}};
  while (true) {
    tcp_socket sock = co_await acceptor.async_accept();
    co_yield std::move(sock);
  }
  co_return tcp_socket{acceptor.get_executor()};
}

unsigned short port = 0;
cobalt::promise<void> run_server(cobalt::wait_group& workers) {
  fmt::println("Listening on port {}", port);
  auto l = listen(port);

  try {
    while (true) {
      workers.push_back(session(co_await l));
    }
  } catch (std::exception& e) {
    fmt::println("Error: {}", e.what());
    workers.cancel();
  }
}

cobalt::main co_main(int argc, char** argv) {
  po::options_description desc{"Allowed options"};
  desc.add_options()("port,p", po::value<unsigned short>(&port)->default_value(5600));
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  fmt::println("Starting server...");
  co_await cobalt::with(cobalt::wait_group(), &run_server);
  fmt::println("Graceful shutdown");

  co_return EXIT_SUCCESS;
}