add_library(cleo INTERFACE
    ring_buf.hpp
    error.hpp
)

target_include_directories(cleo INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
