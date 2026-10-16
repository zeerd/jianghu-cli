enable_testing()

file(GLOB_RECURSE TEST_SOURCES "*.cc")
add_executable(test_cases ${TEST_SOURCES})
target_include_directories(test_cases PRIVATE ../src)
target_link_libraries(test_cases PRIVATE jianghu_lib)

if(MSVC)
    # Windows/MSVC：通过 NuGet 引入 GTest。生成的 vcxproj 会带
    # <PackageReference>，MSBuild 在编译前自动 restore（联网时从
    # nuget.org 下载，离线时用仓库内 packages/ 目录）。包内的
    # .targets 会自动注入 include 目录与 .lib，无需显式链接。
    # 注意：该包为静态 CRT（rt-static），需与顶层 CMakeLists 的
    # CMAKE_MSVC_RUNTIME_LIBRARY 保持一致。
    set_property(TARGET test_cases PROPERTY
        VS_PACKAGE_REFERENCES
        "Microsoft.googletest.v140.windesktop.msvcstl.static.rt-static_1.8.1.8")
else()
    # 非 Windows 平台使用系统安装的 GTest
    find_package(GTest REQUIRED)
    target_link_libraries(test_cases PRIVATE
        GTest::gtest
        GTest::gtest_main
    )
endif()

add_test(NAME test_cases COMMAND test_cases)
