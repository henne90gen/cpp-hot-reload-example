#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <chrono>
#include <dlfcn.h>
#include <filesystem>
#include <iostream>

#include "test_library.h"

struct Code {
  void *libraryHandle = nullptr;
  int64_t lastModified = std::numeric_limits<int64_t>::min();

  update_func *update = nullptr;
};

Code code = {};
Platform platform = {};

int64_t platform_last_modified(const std::string &file_path) {
  std::error_code ec;
  auto last_write_time = std::filesystem::last_write_time(file_path, ec);
  if (ec) {
    return 0;
  }
  return std::chrono::time_point_cast<std::chrono::milliseconds>(
             last_write_time)
      .time_since_epoch()
      .count();
}

void platform_log(const std::string &msg) { std::cout << msg << std::endl; }

void reload_code() {
  std::string libraryPath = "./libtest_library.so";
  auto lastModified = platform_last_modified(libraryPath);
  if (code.libraryHandle != nullptr && lastModified == code.lastModified) {
    return;
  }

  if (code.libraryHandle) {
    dlclose(code.libraryHandle);
  }

  std::cout << "Reloading code" << std::endl;

  code.lastModified = lastModified;
  code.libraryHandle = dlopen(libraryPath.c_str(), RTLD_NOW);
  if (!code.libraryHandle) {
    std::cout << "Couldn't load library: " << std::string(dlerror())
              << std::endl;
    return;
  }

  code.update = (update_func *)dlsym(code.libraryHandle, "update");
  if (!code.update) {
    std::cout << "Couldn't load 'update' function: " << std::string(dlerror())
              << std::endl;
    return;
  }

  auto dlsym_error = dlerror();
  if (dlsym_error) {
    std::cout << "Function loading failed" << dlsym_error << std::endl;
  }
}

int main() {
  std::cout << "Starting program" << std::endl;

  platform.log = platform_log;

  if (glfwInit() == 0) {
    return 1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  auto glfw_window =
      glfwCreateWindow(100, 100, "Test Window", nullptr, nullptr);
  if (glfw_window == nullptr) {
    std::cerr << "Failed to create window" << std::endl;
    glfwTerminate();
    return 1;
  }

  glfwMakeContextCurrent(glfw_window);

#if 1
    auto procAddress = reinterpret_cast<GLADloadproc>(glfwGetProcAddress);
    auto status = gladLoadGLLoader(procAddress);
    if (status == 0) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return 1;
    }
#else
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(err)
              << std::endl;
    return 1;
  }
#endif

  while (glfwWindowShouldClose(glfw_window) == 0) {
    glClearColor(0.1F, 0.1F, 0.1F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    reload_code();

    code.update(platform);

    glfwSwapBuffers(glfw_window);
    glfwPollEvents();
  }
}
