#include "cmdline/cmdline.h"
#include "simplelog/simplelog.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui/bindings/imgui_impl_glfw.h"
#include "imgui/bindings/imgui_impl_opengl3.h"
#include "imgui/imgui.h"


std::string programName = "GLFW window";
int windowWidth = 1200,
    windowHeight = 800;
float backgroundR = 0.1f,
      backgroundG = 0.3f,
      backgroundB = 0.2f;

static void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

static void teardown(GLFWwindow *window)
{
    if (window != nullptr) { glfwDestroyWindow(window); }
    glfwTerminate();
}

static void logGlfwErr()
{
  const char *desc;
  if(const int code = glfwGetError(&desc); code != GLFW_NO_ERROR)
    log << "[ERROR] code " << code << " " << desc;
}

int main(int argc, char *argv[])
{
  cmdline::parser arg;
  arg.add("help", 'h', "Print help.");
//  arg.add<std::string>("file", 'f', "Example file argument.", true);
    
  if(!arg.parse(argc, const_cast<const char* const*>(argv)))
  {
    const auto fullErr = arg.error_full();
    if(!fullErr.empty())
      log << fullErr;
     
    log << arg.usage();
    return 0;
  }
  
  if(arg.exist("help"))
  {
    log << arg.usage();
    return 0;
  } 

  /* Example file arg check and get
  if(!arg.exist("file"))
  {
    log << "--file or -f argument is mandatory!";
    log << arg.usage();
    return 0;
  }
  
  const std::string file = arg.get<std::string>("file");
  */
     

  if (!glfwInit())
  {
      log << "[ERROR] Couldn't initialize GLFW";
      return -1;
  }
  else
  {
      log << "[INFO] GLFW initialized";
  }

  // setup GLFW window

  glfwWindowHint(GLFW_DOUBLEBUFFER , 1);
  glfwWindowHint(GLFW_DEPTH_BITS, 24);
  glfwWindowHint(GLFW_STENCIL_BITS, 8);

  glfwWindowHint(
      GLFW_OPENGL_PROFILE,
      GLFW_OPENGL_CORE_PROFILE
      );

  std::string glsl_version = "";

  // GL 3.2 + GLSL 150
  glsl_version = "#version 150";
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
    logGlfwErr();
    teardown(nullptr);
    return -1;
  }
  // watch window resizing
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwMakeContextCurrent(window);
  // VSync
  glfwSwapInterval(1);


  if (!gladLoadGLLoader( reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
  {
    log << "[ERROR] Couldn't initialize GLAD";
    teardown(window);
    return -1;
  }
  else
  {
    log << "[INFO] GLAD initialized";
  }

  log << "[INFO] OpenGL from glad " << GLVersion.major << "." << GLVersion.minor;

  int actualWindowWidth, actualWindowHeight;
  glfwGetWindowSize(window, &actualWindowWidth, &actualWindowHeight);
  glViewport(0, 0, actualWindowWidth, actualWindowHeight);

  glClearColor(backgroundR, backgroundG, backgroundB, 1.0f);
  // --- rendering loop
  while (!glfwWindowShouldClose(window))
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  teardown(window);
/*
   // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui_ImplOpenGL3_Init();
  ImGui_ImplGlfw_NewFrame();
  //ImGuiIO &io = ImGui::GetIO();
    
  ImGui::StyleColorsDark(); 
    
  ImGui::ShowDemoWindow();

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext(); 
*/
  return 0;
}
