cmake_minimum_required(VERSION 3.10.2)
project(DESKTOP_REMOTE)

set(TEST_SCREEN_X11_SRC
    ./test_x11.cpp
    ./screen_x11.h
    ./screen_x11.cpp
)

set(SCREEN_SERVER_SRC
    ./common.h
    ./screen_x11.h
    ./screen_x11.cpp
    ./server_.cpp
    ./server_.h
    ./main.cpp
)

add_executable(TEST_SCREEN_X11 ${TEST_SCREEN_X11_SRC})
target_link_libraries(TEST_SCREEN_X11 -lfmt -lX11 -lXext -lrt -lpng)

add_executable(SCREEN_SERVER ${SCREEN_SERVER_SRC})
target_link_libraries(SCREEN_SERVER -lfmt -lX11 -lXext -lrt -lpng -lpthread)