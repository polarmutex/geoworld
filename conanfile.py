from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps


class GeoWorldConan(ConanFile):
    name = "geoworld"
    version = "1.0.0"
    
    # Package metadata
    license = "MIT"
    author = "GeoWorld Team"
    url = "https://github.com/user/geoworld"
    description = "Geospatial data visualization platform"
    topics = ("gis", "mapping", "geospatial", "qt6")
    
    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}
    
    # Sources
    exports_sources = "CMakeLists.txt", "src/*", "qml/*"
    
    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC
    
    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")
        # Force Qt to build from source on Linux since binaries aren't available
        if self.settings.os == "Linux":
            self.options["qt"].shared = True
    
    def layout(self):
        cmake_layout(self)
    
    def requirements(self):
        self.requires("qt/6.8.3")
        self.requires("qt-advanced-docking-system/4.3.1")
    
    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()
    
    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
    
    def package(self):
        cmake = CMake(self)
        cmake.install()