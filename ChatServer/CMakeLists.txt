cmake_minimum_required(VERSION 3.14)

project(ChatServer LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Network)
# PostgreSQL 라이브러리 찾기
find_package(PostgreSQL REQUIRED)

# PostgreSQL 헤더 파일 디렉토리 포함
include_directories(${PostgreSQL_INCLUDE_DIRS})

add_executable(ChatServer
  main.cpp
  dbcon.h
)
target_link_libraries(ChatServer Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network ${PostgreSQL_LIBRARIES})

include(GNUInstallDirs)
install(TARGETS ChatServer
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
