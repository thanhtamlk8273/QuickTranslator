# Created by and for Qt Creator This file was created for editing the project sources only.
# You may attempt to use it for building too, by modifying this file here.

cmake_minimum_required(VERSION 3.2)
project(QuickTranslator)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_PREFIX_PATH "/home/xemanon/Qt/6.3.1/gcc_64")
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
find_package(ICU 70.0 COMPONENTS uc i18n  REQUIRED)

add_subdirectory(fmt)

include_directories(
    src
)

set (SRCS
    src/Common.h
    src/Dictionary.cpp
    src/Dictionary.h
    src/main.cpp
    src/MyTextEdit.cpp
    src/MyTextEdit.h
    src/Translator.cpp
    src/Translator.h
)

set (ASAN_COMPILE_FLAGS -fsanitize=address -fno-omit-frame-pointer -fsanitize-recover=address)
set (ASAN_LINKING_FLAGS -fsanitize=address)

add_executable(${CMAKE_PROJECT_NAME} ${SRCS})

target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Werror -g -O0 -fsanitize=address -fno-omit-frame-pointer -fsanitize-recover=address)
target_link_options(${CMAKE_PROJECT_NAME} PRIVATE -fsanitize=address)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ICU::uc ICU::i18n)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE fmt)


FILE(COPY src/VietPhrase.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
FILE(COPY src/Names.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
FILE(COPY src/ChinesePhienAmWords.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
