cmake_minimum_required(VERSION 3.18)

project(Cyanite
  LANGUAGES C CXX Swift)

add_library(GameScripts SHARED Test.swift)

target_compile_options(GameScripts PRIVATE
  -parse-as-library)
