/**
* Пример клиента на базе boost::asio.
*
* @source http://boost.org/doc/libs/1_53_0/doc/html/boost_asio/example/echo/blocking_tcp_echo_client.cpp
*/


#define _WIN32_WINNT 0x0501

//
// blocking_tcp_echo_client.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>


using boost::asio::ip::tcp;


enum { max_length = 1024 };




int main(int argc, char* argv[])
{
  try
  {
    if (argc != 3)
    {
      std::cerr << "Usage: blocking_tcp_echo_client <host> <port>\n";
      return 1;
    }

    boost::asio::io_service  io_service;

    tcp::resolver  resolver(io_service);
    tcp::resolver::query  query(tcp::v4(), argv[1], argv[2]);
    tcp::resolver::iterator  iterator = resolver.resolve(query);

    tcp::socket  s(io_service);
    boost::asio::connect(s, iterator);

    using namespace std; // For strlen.

    char request[max_length];
    for ( ; std::strcmp( "q", request ) != 0; ) {
        std::cout << "Enter message: ";
        std::cin.getline(request, max_length);
        size_t request_length = strlen(request);
        boost::asio::write(s, boost::asio::buffer(request, request_length));

        char reply[max_length];
        size_t reply_length = boost::asio::read(s,
            boost::asio::buffer(reply, request_length));
        std::cout << "Reply is: ";
        std::cout.write(reply, reply_length);
        std::cout << "\n\n";

    } // for ( ; ;)

  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  std::cout << "\n^\n";
  std::cin.ignore();

  return 0;
}
