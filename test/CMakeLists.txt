enable_testing()

include_directories(../include/)

find_package(GTest REQUIRED)

add_executable(test_result test_result.cpp)
target_link_libraries(test_result PRIVATE GTest::gtest_main)

set(TEST_LIBS
    GTest::gtest_main
    proxytest
    ${UV_LIBRARIES}
)

add_executable(test_udp test_udp.cpp)
target_link_libraries(test_udp PRIVATE ${TEST_LIBS})

add_executable(test_tcp test_tcp.cpp)
target_link_libraries(test_tcp PRIVATE ${TEST_LIBS})

add_executable(test_server test_server.cpp)
target_link_libraries(test_server PRIVATE ${TEST_LIBS})

add_executable(test_proxy test_proxy.cpp)
target_link_libraries(test_proxy PRIVATE ${TEST_LIBS})

add_executable(test_forward test_forward.cpp)
target_link_libraries(test_forward PRIVATE ${TEST_LIBS})

include(GoogleTest)

gtest_discover_tests(test_result test_udp test_tcp)

