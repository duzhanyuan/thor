//
// ssl_connection.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2016 Vladimir Voinea (voineavladimir@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef SSL_CONNECTION
#define SSL_CONNECTION
#include "connection.hpp"
#include <boost/asio/ssl.hpp>
#include <boost/bind.hpp>
#include <iostream>

namespace http {
namespace server {

/// Represents a single connection from a client.
class ssl_connection : public connection {
    public:
    typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket;
    /// Construct a connection with the given io_service.
    explicit ssl_connection(boost::asio::io_service &io_service, boost::asio::ssl::context &context,
                            request_handler &handler)
        : connection(io_service, handler), strand_(io_service), socket_(io_service, context),
          request_handler_(handler) {}

    virtual ~ssl_connection() {}

    ssl_socket::lowest_layer_type &lowest_layer__socket() { return socket_.lowest_layer(); }

    /// Start the first asynchronous operation for the connection.
    void start() override {
        socket_.async_handshake(
            boost::asio::ssl::stream_base::server,
            strand_.wrap(std::bind(&ssl_connection::handle_handshake, shared_from_this(), std::placeholders::_1)));
    }

    void handle_handshake(const boost::system::error_code &error) {
        if (!error) {
            socket_.async_read_some(boost::asio::buffer(buffer_),
                                    strand_.wrap(std::bind(&ssl_connection::handle_read, shared_from_this(),
                                                           std::placeholders::_1, std::placeholders::_2)));
        }
    }

    protected:
    /// Handle completion of a read operation.
    void handle_read(const boost::system::error_code &e, std::size_t bytes_transferred) override {
        if (!e) {
            boost::tribool result;
            boost::tie(result, boost::tuples::ignore) =
                request_parser_.parse(request_, buffer_.data(), buffer_.data() + bytes_transferred);

            if (result) {
                request_handler_.handle_request<request_handler::protocol_type::https>(request_, reply_);
                boost::asio::async_write(
                    socket_, reply_.to_buffers(),
                    strand_.wrap(std::bind(&ssl_connection::handle_write, shared_from_this(), std::placeholders::_1)));
            } else if (!result) {
                reply_ = reply::stock_reply(reply::status_type::bad_request);
                boost::asio::async_write(
                    socket_, reply_.to_buffers(),
                    strand_.wrap(std::bind(&ssl_connection::handle_write, shared_from_this(), std::placeholders::_1)));
            } else {
                socket_.async_read_some(boost::asio::buffer(buffer_),
                                        strand_.wrap(std::bind(&ssl_connection::handle_read, shared_from_this(),
                                                               std::placeholders::_1, std::placeholders::_2)));
            }
        }

        // If an error occurs then no new asynchronous operations are started. This
        // means that all shared_ptr references to the connection object will
        // disappear and the object will be destroyed automatically after this
        // handler returns. The connection class's destructor closes the socket.
    }

    /// Handle completion of a write operation.
    void handle_write(const boost::system::error_code &) override {

        // No new asynchronous operations are started. This means that all shared_ptr
        // references to the connection object will disappear and the object will be
        // destroyed automatically after this handler returns. The connection class's
        // destructor closes the socket.
    }

    void shutdown() override {
        socket_.async_shutdown(
            strand_.wrap(std::bind(&ssl_connection::handle_shutdown, shared_from_this(), std::placeholders::_1)));
    }

    void handle_shutdown(const boost::system::error_code &) {}

    void print_err(boost::system::error_code error) {
        std::string err = error.message();
        if (error.category() == boost::asio::error::get_ssl_category()) {
            err = std::string(" (") + std::to_string(ERR_GET_LIB(error.value())) + "," +
                  std::to_string(ERR_GET_FUNC(error.value())) + "," + std::to_string(ERR_GET_REASON(error.value())) +
                  ") ";
            // ERR_PACK /* crypto/err/err.h */
            char buf[128];
            ::ERR_error_string_n(error.value(), buf, sizeof(buf));
            err += buf;
        }
        std::cout << err << std::endl;
    }

    boost::shared_ptr<ssl_connection> shared_from_this() {
        return boost::dynamic_pointer_cast<ssl_connection>(connection::shared_from_this());
    }

    /// Strand to ensure the connection's handlers are not called concurrently.
    boost::asio::io_service::strand strand_;

    private:
    /// Socket for the connection.
    ssl_socket socket_;

    protected:
    /// The handler used to process the incoming request.
    request_handler &request_handler_;

    /// Buffer for incoming data.
    boost::array<char, 8192> buffer_;

    /// The incoming request.
    request request_;

    /// The parser for the incoming request.
    request_parser request_parser_;

    /// The reply to be sent back to the client.
    reply reply_;

    sendfile_op sendfile_;
};

typedef boost::shared_ptr<ssl_connection> ssl_connection_ptr;

} // namespace server3
} // namespace http

#endif // SSL_CONNECTION
