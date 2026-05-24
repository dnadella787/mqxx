from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout


class MqxxConan(ConanFile):
    name = "mqxx"
    version = "0.1.0"

    settings = "os", "arch", "compiler", "build_type"
    options = {
        "build_tests": [True, False],
        "enable_clang_tidy": [True, False],
        "enable_sanitizers": [True, False],
        "warnings_as_errors": [True, False],
    }
    default_options = {
        "build_tests": False,
        "enable_clang_tidy": True,
        "enable_sanitizers": False,
        "warnings_as_errors": False,
    }

    def requirements(self):
        if self.options.build_tests:
            self.requires("gtest/1.14.0")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()

        toolchain = CMakeToolchain(self)
        toolchain.cache_variables["MQXX_BUILD_TESTS"] = self.options.build_tests
        toolchain.cache_variables["MQXX_ENABLE_CLANG_TIDY"] = self.options.enable_clang_tidy
        toolchain.cache_variables["MQXX_ENABLE_SANITIZERS"] = self.options.enable_sanitizers
        toolchain.cache_variables["MQXX_WARNINGS_AS_ERRORS"] = self.options.warnings_as_errors
        toolchain.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
