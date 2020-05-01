#include "cmdline/cmdline.h"
#include "simplelog/simplelog.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui/bindings/imgui_impl_glfw.h"
#include "imgui/bindings/imgui_impl_opengl3.h"
#include "imgui/imgui.h"


static constexpr const char* programName = "GLFW window";

constexpr static int
  windowWidth = 1200,
  windowHeight = 800;

constexpr static float
      backgroundR = 0.1f,
      backgroundG = 0.3f,
      backgroundB = 0.2f;

static void framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height)
{
    glViewport(0, 0, width, height);
}

static void error_callback(int error, const char* description)
{
  log << "[ERROR] code " << error << " " << description;
}

static void teardown(GLFWwindow *window)
{
    if (window != nullptr) { glfwDestroyWindow(window); }
    glfwTerminate();
}

int main(int argc, char *argv[])
{
  cmdline::parser arg;
  arg.add("help", 'h', "Print help.");
  arg.add<std::string>("font", 'f', "Font", false);
    
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

  const std::string font = [&]{
    if(!arg.exist("font"))
      return std::string();
    
    return arg.get<std::string>("font");
  }();
   

  if (!glfwInit())
  {
    log << "[ERROR] Couldn't initialize GLFW";
    return -1;
  }
  else
    log << "[INFO] GLFW initialized";
  

  glfwSetErrorCallback(error_callback); 

  // setup GLFW window

  glfwWindowHint(GLFW_DOUBLEBUFFER , 1);
  glfwWindowHint(GLFW_DEPTH_BITS, 24);
  glfwWindowHint(GLFW_STENCIL_BITS, 8);

  glfwWindowHint(
      GLFW_OPENGL_PROFILE,
      GLFW_OPENGL_CORE_PROFILE
      );


  // GL 3.2 + GLSL 150
  const std::string glslVersion = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  GLFWwindow *window = glfwCreateWindow(
      windowWidth,
      windowHeight,
      programName,
      nullptr,
      nullptr
      );
  

  if (!window)
  {
    log << "[ERROR] Couldn't create a GLFW window";
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

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  if(!font.empty())
  {
    ImGuiIO &io = ImGui::GetIO(); 
    io.Fonts->AddFontFromFileTTF(font.c_str(), 18.0f, nullptr, nullptr);
  }
  else
    log << "[WARNING] No font set";
  
  //  ImGui::StyleColorsDark();
  ImGui::StyleColorsLight();

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glslVersion.c_str());

  // --- rendering loop
  while (!glfwWindowShouldClose(window))
  {
    glfwPollEvents();

    // feed inputs to dear imgui, start new frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow();
   

    // Render dear imgui into screen
    ImGui::Render();

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    //glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }
 

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext(); 

  teardown(window);
  return 0;
}
