#include "test_library.h"

#include <glad/glad.h>

#include "bla.h"

#if WIN32
#define LIBRARY_API extern "C" __declspec(dllexport)
#else
#define LIBRARY_API extern "C"
#endif

LIBRARY_API void update(Platform &platform) {
  glClearColor(1.0F, 0.0F, 1.0F, 1.0F);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  static unsigned int VAO;
  if (!VAO) {
    glGenVertexArrays(1, &VAO);
  }
  glBindVertexArray(VAO);

  // platform.log("updating test");

  bla(platform);
}
