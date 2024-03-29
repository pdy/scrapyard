#ifndef PDY_INCLUDE_H_
#define PDY_INCLUDE_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui.h"

#include "simplelog/simplelog.h"
#include <tuple>
#include <optional>

namespace pdy {

struct Color
{
  float r{0.0};
  float g{0.0};
  float b{0.0};
  float a{1.0};

  inline bool operator ==(const Color &other) const
  {
    constexpr float EPS = 0.001f;
    const auto eq = [&](float lhs, float rhs) { return std::abs(lhs - rhs) <= EPS; };

    return eq(r, other.r) && eq(g, other.g) && eq(b, other.b) && eq(a, other.a);
  }

  inline bool operator !=(const Color &other) const { return !(*this == other); }
};

static const auto YELLOW = Color{1.0f, 1.0f, 0.0f};
static const auto ORANGE = Color{1.0f, 0.5f, 0.2f};

inline void frameBufferSizeCallback(GLFWwindow* /*window*/, int width, int height)
{
  glViewport(0, 0, width, height);
}

inline void errorCallback(int error, const char* description)
{
  log << "[ERROR] code " << error << " " << description;
}


inline void terminateGLFW(GLFWwindow *window = nullptr)
{
  if (window)
    glfwDestroyWindow(window);

  glfwTerminate();
}

template<typename ErrorCallback, typename FrameBufferSizeCallback>
inline GLFWwindow* initGLFW(
    const std::string &programName,
    int windowWidth,
    int windowHeight,
    ErrorCallback errorCallback_,
    FrameBufferSizeCallback frameBufferSizeCallback_)
{
  if (!glfwInit())
  {
    log << "[ERROR] Couldn't initialize GLFW";
    return nullptr;
  }
  else
    log << "[INFO] GLFW initialized";
  

  if(errorCallback_)
    glfwSetErrorCallback(errorCallback_); 

  // setup GLFW window
  glfwWindowHint(GLFW_DOUBLEBUFFER , 1);
  glfwWindowHint(GLFW_DEPTH_BITS, 24);
  glfwWindowHint(GLFW_STENCIL_BITS, 8);

  glfwWindowHint(
      GLFW_OPENGL_PROFILE,
      GLFW_OPENGL_CORE_PROFILE
      );


  // GL 3.3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  GLFWwindow *window = glfwCreateWindow(
      windowWidth,
      windowHeight,
      programName.c_str(),
      nullptr,
      nullptr
      );
  

  if (!window)
  {
    log << "[ERROR] Couldn't create a GLFW window";
    terminateGLFW(nullptr);
    return nullptr;
  }
 
//  glfwSetKeyCallback(window, keyCallback); 
  
  // watch window resizing
  if(frameBufferSizeCallback_)
    glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback_);

  glfwMakeContextCurrent(window);
  
  // VSync
  glfwSwapInterval(1);

  return window;
}

inline GLFWwindow* initGLFW(const std::string &programName, int windowWidth, int windowHeight)
{
  return initGLFW(programName, windowWidth, windowHeight, ::pdy::errorCallback, ::pdy::frameBufferSizeCallback);
}

std::tuple<bool, std::string> isShaderProgramCompSuccess(unsigned int programId);
std::tuple<bool, std::string> isShaderCompSuccess(unsigned int shaderId);
std::optional<unsigned int> compileShader(const char *shaderSource, unsigned int shaderType);
struct Result
{
  unsigned int VAO;
  unsigned int shaderProgram;
  unsigned int count;
};

struct ResultEx
{
  unsigned int VAO;
  unsigned int shaderProgram_1;
  unsigned int shaderProgram_2;
  unsigned int shader1Count;
  unsigned int shader2Count;
};

std::optional<Result> createRect(float ndcOrigoX, float ndcOrigoY, float ndcWidth, float ndcHeight, Color color = ORANGE);

std::optional<Result> createRectangleGLBuffer();
std::optional<Result> createTwoRects();
std::optional<Result> createTwoTrianglesNextToEachOther();
std::optional<ResultEx> createTwoTrianglesNextToEachOtherDifferentColors();

std::optional<Result> createTriangle_1();
std::optional<Result> createTriangle_2();

} // namespace pdy

#endif
