from conans import ConanFile, CMake


class CoRedisConan(ConanFile):
    name = "co_redis"
    version = "0.1"
    license = "MIT"
    author = "Dmitry Khominich khdmitryi@gmail.com"
    description = "redis asynchronous client based on co_lib framework"
    topics = ("<Put some tag here>", "<here>", "<and here>")
    generators = "cmake"
    exports_sources = "include/*"
    requires="co_lib/0.1", "boost/1.75.0"

    def package(self):
        self.copy("*.hpp")
