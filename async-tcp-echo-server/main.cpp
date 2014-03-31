/**
* Пример сервера на базе boost::asio.
*
* @source http://boost.org/doc/libs/1_53_0/doc/html/boost_asio/example/echo/async_tcp_echo_server.cpp
*/


#define _WIN32_WINNT 0x0501

//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>


using boost::asio::ip::tcp;




class session
{
public:
  session(boost::asio::io_service& io_service)
    : socket_(io_service)
  {
  }




  tcp::socket& socket()
  {
    return socket_;
  }




  void start()
  {
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&session::handle_read, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }




private:
  void handle_read(const boost::system::error_code& error,
      size_t bytes_transferred)
  {
    if ( error ) {
      delete this;
      return;
    }

    auto buffer = boost::asio::buffer( data_, bytes_transferred );

#ifdef _DEBUG
    const auto sb = boost::asio::buffer_cast< const char* >( buffer );
    const std::string  s( sb, sb + bytes_transferred );
    std::cout << "[ " << s << " ]" << std::endl;
#endif

    boost::asio::async_write(socket_,
        buffer,
        boost::bind(&session::handle_write, this,
        boost::asio::placeholders::error));
  }




  void handle_write(const boost::system::error_code& error)
  {
    if (!error)
    {
      socket_.async_read_some(boost::asio::buffer(data_, max_length),
          boost::bind(&session::handle_read, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
    else
    {
      delete this;
    }
  }




  tcp::socket socket_;
  enum { max_length = 1024 };
  char data_[max_length];
};




class server
{
public:
  server(boost::asio::io_service& io_service, short port)
    : io_service_(io_service),
      acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
  {
    start_accept();
  }




private:
  void start_accept()
  {
    session* new_session = new session(io_service_);
    acceptor_.async_accept(new_session->socket(),
        boost::bind(&server::handle_accept, this, new_session,
          boost::asio::placeholders::error));
  }




  void handle_accept(session* new_session,
      const boost::system::error_code& error)
  {
    if (!error)
    {
      new_session->start();
    }
    else
    {
      delete new_session;
    }

    start_accept();
  }




  boost::asio::io_service&  io_service_;
  tcp::acceptor  acceptor_;
};




int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: async_tcp_echo_server <port>\n";
      return 1;
    }

    boost::asio::io_service io_service;

    using namespace std; // For atoi.
    server s(io_service, atoi(argv[1]));

    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
