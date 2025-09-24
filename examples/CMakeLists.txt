# 并发下载示例
add_executable(concurrent_download_example "concurrent_download_example.cpp")
target_link_libraries(concurrent_download_example common data infra service Qt6::Core Qt6::Sql)
