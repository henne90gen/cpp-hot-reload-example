#include <glad/glad.h>

#if WIN32
#include <Windows.h>
#undef min
#else
#include <dlfcn.h>
#endif

#include <GLFW/glfw3.h>
#include <chrono>
#include <filesystem>
#include <iostream>

#include "test_library.h"

struct Code {
#if WIN32
  HMODULE gameCodeDLL = nullptr;
#else
  void *libraryHandle = nullptr;
#endif

  std::string sourceDllPath;
  std::string tmpDllPath;
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

#if WIN32
std::string GetLastErrorAsString() {
  // Get the error message ID, if any.
  DWORD errorMessageID = ::GetLastError();
  if (errorMessageID == 0) {
    return {}; // No error message has been recorded
  }

  LPSTR messageBuffer = nullptr;

  // Ask Win32 to give us the string version of that message ID.
  // The parameters we pass in, tell Win32 to create the buffer that holds the
  // message for us (because we don't yet know how long the message string will
  // be).
  size_t size = FormatMessageA(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
          FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPSTR)&messageBuffer, 0, NULL);

  // Copy the error message into a std::string.
  std::string message(messageBuffer, size);

  // Free the Win32's string's buffer.
  LocalFree(messageBuffer);

  return message;
}

void reload_code() {
  if (code.sourceDllPath.empty()) {
    char filename[1000];
    GetModuleFileNameA(nullptr, filename, sizeof(filename));
    auto exePath = std::filesystem::path(std::string(filename))
                       .parent_path()
                       .generic_string();
    code.sourceDllPath = exePath + "/test_library.dll";
    code.tmpDllPath = exePath + "/test_library-tmp.dll";
  }

  if (!std::filesystem::exists(code.sourceDllPath)) {
    return;
  }

  auto lastModified = platform_last_modified(code.sourceDllPath);
  if (code.gameCodeDLL != nullptr && lastModified == code.lastModified) {
    return;
  }

  if (code.gameCodeDLL) {
    FreeLibrary(code.gameCodeDLL);
    code.gameCodeDLL = nullptr;
    code.update = nullptr;
  }

  platform_log("Loading game code");

  code.lastModified = lastModified;

  CopyFile(code.sourceDllPath.c_str(), code.tmpDllPath.c_str(), FALSE);
  code.gameCodeDLL = LoadLibraryA(code.tmpDllPath.c_str());
  if (!code.gameCodeDLL) {
    std::cout << "Failed to load code: " << GetLastErrorAsString() << std::endl;
    exit(1);
  }

  code.update = (update_func *)GetProcAddress(code.gameCodeDLL, "update");
  if (!code.update) {
    std::cout << "Failed to load function 'update': " << GetLastErrorAsString()
              << std::endl;
    exit(1);
  }
}
#else
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
    exit(1);
  }

  code.update = (update_func *)dlsym(code.libraryHandle, "update");
  if (!code.update) {
    std::cout << "Couldn't load 'update' function: " << std::string(dlerror())
              << std::endl;
    exit(1);
  }

  auto dlsym_error = dlerror();
  if (dlsym_error) {
    std::cout << "Function loading failed" << dlsym_error << std::endl;
    exit(1);
  }
}
#endif

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
