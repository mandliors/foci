cmake_minimum_required(VERSION 3.11)
project(foci)

add_executable(foci)

target_sources(foci PRIVATE
    main.c
)

target_compile_options(foci PRIVATE
    #-Werror -Wall -Wextra -Wpedantic -Wconversion
    -fsanitize=address -fno-omit-frame-pointer
)

target_link_options(foci PRIVATE -lm -fsanitize=address)