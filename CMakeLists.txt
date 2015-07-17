cmake_minimum_required(VERSION 2.8)

project(nss-etcd)

set(CMAKE_BUILD_TYPE Debug)

include_directories(
    PROJECT_SOURCE_DIR
    src
    test
)

add_library(nss_etcd SHARED src/nss-etcd.c)
set_target_properties(nss_etcd PROPERTIES SOVERSION 2)
target_link_libraries(nss_etcd etcd-nss-api)
install(TARGETS nss_etcd DESTINATION lib)

add_executable(etc-test test/etd-test.c)
target_link_libraries(etc-test etcd-nss-api)

# Libraries
add_library(etcd-nss-api STATIC
    src/nss-etcd-api.c
)
set_target_properties(etcd-nss-api PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
target_link_libraries(etcd-nss-api etcd-api)

add_library(etcd-api STATIC
    src/etcd-api/etcd-api2.c
)
set_target_properties(etcd-api PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
target_link_libraries(etcd-api ghttp yajl)

add_library(ghttp STATIC
    src/ghttp/ghttp.c
    src/ghttp/http_base64.c
    src/ghttp/http_date.c
    src/ghttp/http_hdrs.c
    src/ghttp/http_req.c
    src/ghttp/http_resp.c
    src/ghttp/http_trans.c
    src/ghttp/http_uri.c
)
set_target_properties(ghttp PROPERTIES POSITION_INDEPENDENT_CODE TRUE)