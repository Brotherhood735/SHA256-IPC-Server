# Versione minima richiesta di CMake
cmake_minimum_required(VERSION 3.10)

project(SHA256 VERSION 1.0)

set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

find_package(OpenSSL REQUIRED)

include_directories(${OPENSSL_INCLUDE_DIR})

set(UTILITY_SOURCES src/utils.c src/utils.h)

# --- Creazione del Target Server ---
add_executable(server src/server.c ${UTILITY_SOURCES})

# Collega le librerie necessarie per il server
# OPENSSL_LIBRARIES include sia libssl che libcrypto
# La libreria 'rt' (real-time) è spesso richiesta per le IPC su alcuni sistemi
target_link_libraries(server PRIVATE ${OPENSSL_LIBRARIES} rt)


# --- Creazione del Target Client ---
add_executable(client src/client.c ${UTILITY_SOURCES})

# Collega le librerie necessarie per il clients
target_link_libraries(client PRIVATE ${OPENSSL_LIBRARIES} rt)
