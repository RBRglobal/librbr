# Sample application using RBR library
cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(rbr_sample)

target_sources(app PRIVATE src/main.c)