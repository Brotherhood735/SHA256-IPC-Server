cmake_minimum_required(VERSION 3.10)

project(SHA256 VERSION 1.0)

# i file in output li mettiamo nella cartella build
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

# file che contengono dati e funzioni condivise tra il server e il client
set(UTILS src/utils.c src/utils.h)

add_executable(server src/server.c ${UTILS})
target_link_libraries(server PRIVATE ${OPENSSL_LIBRARIES} rt)

add_executable(client src/client.c ${UTILS})
target_link_libraries(client PRIVATE ${OPENSSL_LIBRARIES} rt)