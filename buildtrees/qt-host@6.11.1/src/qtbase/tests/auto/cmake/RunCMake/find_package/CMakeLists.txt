cmake_minimum_required(VERSION 3.16)
project(${RunCMake_TEST} LANGUAGES CXX)

# Setting AUTOMOC to true to make sure Qt package is used
set(CMAKE_AUTOMOC ON)

include(${RunCMake_TEST}.cmake)
