/**
* Скелет прокси-сервера на базе boost::asio.
*
* @source http://stackoverflow.com/questions/11421678/how-to-make-a-proxy-server-in-c-using-boost
*/


#define _WIN32_WINNT 0x0501

#include <iostream>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>


namespace ba = boost::asio;




ba::io_service&  io_service() {
    static ba::io_service  svc;
    return svc;
}




char localData[ 1024 ]  = {};
char remoteData[ 1024 ] = {};




/**
* Вызывается всякий раз, когда данные получены.
*/
static void handleRead(
    ba::ip::tcp::socket&  readFrom,
    ba::ip::tcp::socket&  writeTo,
    char*   readBuffer,
    size_t  bytes,
    const boost::system::error_code&  e
) {
#ifdef _DEBUG
    const std::string  data( readBuffer, readBuffer + bytes );
    std::cout << data << std::endl;
#endif

    // отправляем полученные данные "другой стороне"
    writeTo.send( ba::buffer( readBuffer, bytes ) );

    // читаем ещё данные с "этой стороны"
    readFrom.async_read_some(
        ba::buffer( readBuffer, 1024 ),
        boost::bind(
            handleRead,
            boost::ref( readFrom ),
            boost::ref( writeTo),
            readBuffer,
            ba::placeholders::bytes_transferred,
            ba::placeholders::error
    ) );
}




static void run( int argc, char** argv );




int main( int argc, char** argv ) {

    if (argc == 5) {
        try {
            run( argc, argv );
        } catch ( const std::exception& ex ) {
            std::cerr << ex.what() << std::endl;
        }

    } else {
        std::cout << "server-boost-asio.exe"
            " LOCAL_IP LOCAL_PORT"
            " REMOTE_IP REMOTE_PORT"
            "\n";
    }


    std::cout << "\n^\n";
    std::cin.ignore();

    return 0;
}




void run( int argc, char** argv ) {

    setlocale( LC_ALL, "Russian" );
    setlocale( LC_NUMERIC, "C" );


    ba::io_service::work  work( io_service() );

    boost::thread  thread( boost::bind(
        &ba::io_service::run,
        &io_service()
    ) );


    // параметры подключения
    const auto localAddress =
        ba::ip::address::from_string( argv[ 1 ] );
    const auto localPort =
        boost::lexical_cast< size_t >( argv[ 2 ] );
    const auto remoteAddress =
        ba::ip::address::from_string( argv[ 3 ] );
    const auto remotePort =
        boost::lexical_cast< size_t >( argv[ 4 ] );
    std::cout <<
        "local \t" << localAddress  << ":" << localPort  << std::endl <<
        "remote\t" << remoteAddress << ":" << remotePort << std::endl;


    // слушатель локального адреса
    const ba::ip::tcp::endpoint  localEP( localAddress, localPort);
    ba::ip::tcp::acceptor  listen( io_service(), localEP );
    ba::ip::tcp::socket    localSocket( io_service() );
    listen.accept( localSocket );

    // удалённое соединение
    const ba::ip::tcp::endpoint  remoteEP( remoteAddress, remotePort);
    ba::ip::tcp::socket  remoteSocket( io_service() );
    remoteSocket.open( remoteEP.protocol());
    remoteSocket.connect( remoteEP );

    // слушаем локальное соединение
    localSocket.async_receive(
        ba::buffer( localData, 1024 ),
        boost::bind(
            handleRead,
            boost::ref( localSocket ),
            boost::ref( remoteSocket),
            localData,
            ba::placeholders::bytes_transferred,
            ba::placeholders::error
    ) );

    // слушаем удалённое соединение
    remoteSocket.async_receive(
        boost::asio::buffer( remoteData, 1024 ),
        boost::bind(
            handleRead,
            boost::ref( remoteSocket),
            boost::ref( localSocket ),
            remoteData,
            ba::placeholders::bytes_transferred,
            ba::placeholders::error
    ) );


    // отправляем тестовые данные
    {
        ba::io_service  io;
        ba::ip::tcp::socket  sa( io );
        const std::string request = "test data";
        ba::write( sa, boost::asio::buffer( request.c_str(), request.length() ) );
    }


    thread.join();
}
