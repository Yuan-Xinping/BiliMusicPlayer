# 基础功能测试
add_executable(test_basic test_basic.cpp)
target_link_libraries(test_basic PRIVATE common data Qt6::Core Qt6::Sql)

# Repository 层测试  
add_executable(test_repository test_repository.cpp)
target_link_libraries(test_repository PRIVATE common data Qt6::Core Qt6::Sql)
