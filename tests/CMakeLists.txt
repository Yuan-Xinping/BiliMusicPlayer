# 基础测试
add_executable(test_basic "test_basic.cpp")
target_link_libraries(test_basic common data Qt6::Core Qt6::Sql)

# Repository 测试
add_executable(test_repository "test_repository.cpp")
target_link_libraries(test_repository common data Qt6::Core Qt6::Sql)

# 二进制文件测试
add_executable(test_binaries "test_binaries.cpp")
target_link_libraries(test_binaries common Qt6::Core)

# 进程和下载测试
add_executable(test_downloader "test_downloader.cpp")
target_link_libraries(test_downloader common data infra Qt6::Core Qt6::Sql)

# 下载服务测试
add_executable(test_download_service "test_download_service.cpp")
target_link_libraries(test_download_service common data infra service Qt6::Core Qt6::Sql)

# 并发下载测试
add_executable(test_concurrent_download "test_concurrent_download.cpp")
target_link_libraries(test_concurrent_download common data infra service Qt6::Core Qt6::Sql)
