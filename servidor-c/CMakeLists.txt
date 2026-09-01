cmake_minimum_required(VERSION 3.15)
project(ChatServer C)
#Estandar de C
set(CMake_C_STANDAR 11)
set(CMake_C_STANDAR_REQUIRED ON)
#Donde CMake busca los archivos de cabecera 
include_directories(include)
#Buscamos los archivos .c en el directorio src/
file(GLOB SOURCES "src/*.c")
#Generamos el ejecutable
add_executable(chat_server ${SOURCES})
#Activamos las advertencias del compilador
target_compile_options(chat_server PRIVATE -Wall -Wextra)
