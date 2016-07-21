//
// main.cpp
// ~~~~~~~~
//
// Copyright (c) 2016 Vladimir Voinea (voineavladimir@gmail.com)
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "server.hpp"
#include "user_handler.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <string>

using http::server::request;
using http::server::reply;
using http::server::user_handler;

void handler(const request &, reply &rep) { rep.content = "asd"; }

int main(int argc, char *argv[]) {
    try {
        // Check command line arguments.
        if (argc != 5) {
            std::cerr << "Usage: http_server <address> <port> <threads> <doc_root>\n";
            std::cerr << "  For IPv4, try:\n";
            std::cerr << "    receiver 0.0.0.0 80 1 .\n";
            std::cerr << "  For IPv6, try:\n";
            std::cerr << "    receiver 0::0 80 1 .\n";
            return 1;
        }

        std::vector<user_handler> handlers{{"GET", std::regex{"^\\/hi(\\/)?$"}, handler}};

        // Initialise the server.
        std::size_t num_threads = boost::lexical_cast<std::size_t>(argv[3]);
        http::server::server s(argv[1], argv[2], argv[4], num_threads, handlers);

        // Run the server until stopped.
        s.run();
    } catch (std::exception &e) {
        std::cerr << "exception: " << e.what() << "\n";
    }

    return 0;
}
