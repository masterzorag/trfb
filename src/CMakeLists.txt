INCLUDE_DIRECTORIES(.)

ADD_EXECUTABLE(trfb_test server.c trfb.c error.c connection.c protocol.c io.c fb.c xbm_print.c trfbtest.c starfield.c)

TARGET_LINK_LIBRARIES(trfb_test)

