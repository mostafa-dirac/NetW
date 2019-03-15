cmake_minimum_required(VERSION 3.13)
project(cpp_cf_pa1)

set(CMAKE_CXX_STANDARD 14)

include_directories(base)
include_directories(user)

add_executable(cpp_cf_pa1
        base/cf.cpp
        base/cf.h
        base/frame.cpp
        base/frame.h
        base/interface.cpp
        base/interface.h
        base/machine.cpp
        base/machine.h
        base/main.cpp
        base/partovdef.h
        user/client_machine.cpp
        user/client_machine.h
        user/facade.cpp
        user/server_machine.cpp
        user/server_machine.h
        user/simpleMachine.cpp
        user/simpleMachine.h
        user/sm.cpp
        user/sm.h)
