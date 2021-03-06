//
// char_memory_mapping_cache.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2016 Vladimir Voinea (voineavladimir@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef CHAR_MEMORY_MAPPING_CACHE_H
#define CHAR_MEMORY_MAPPING_CACHE_H

#include "file_descriptor_cache.hpp"
#include "memory_mapping.hpp"

namespace http {
namespace server {
struct char_memory_mapping_cache {
    static std::shared_ptr<char_memory_mapping> get(const std::string &path, int mode);
};
}
}

#endif /* CHAR_MEMORY_MAPPING_CACHE_H */
