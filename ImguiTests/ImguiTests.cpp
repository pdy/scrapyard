#include "cmdline/cmdline.h"
#include "simplelog/simplelog.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui.h"


static constexpr const char* programName = "GLFW window";

constexpr static int
  windowWidth = 1200,
  windowHeight = 800;

constexpr static float
  backgroundR = 0.2f,
  backgroundG = 0.3f,
  backgroundB = 0.3f;

static void frameBufferSizeCallback(GLFWwindow* /*window*/, int width, int height)
{
  glViewport(0, 0, width, height);
}

static void errorCallback(int error, const char* description)
{
  log << "[ERROR] code " << error << " " << description;
}

static void keyCallback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
{
  if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void close(GLFWwindow *window)
{
  if (window)
    glfwDestroyWindow(window);

  glfwTerminate();
}

/*
static void processInput(GLFWwindow *window)
{
  if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}
*/

static GLFWwindow* initGLFW()
{
  if (!glfwInit())
  {
    log << "[ERROR] Couldn't initialize GLFW";
    return nullptr;
  }
  else
    log << "[INFO] GLFW initialized";
  

  glfwSetErrorCallback(errorCallback); 

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
      programName,
      nullptr,
      nullptr
      );
  

  if (!window)
  {
    log << "[ERROR] Couldn't create a GLFW window";
    close(nullptr);
    return nullptr;
  }
 
  glfwSetKeyCallback(window, keyCallback); 
  
  // watch window resizing
  glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);
  glfwMakeContextCurrent(window);
  
  // VSync
  glfwSwapInterval(1);

  return window;
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
   

  GLFWwindow *window = initGLFW();
  if(!window)
   return -1; 

  if (!gladLoadGLLoader( reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
  {
    log << "[ERROR] Couldn't initialize GLAD";
    close(window);
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

  const std::string glslVersion = "#version 150";
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glslVersion.c_str());

  // --- rendering loop
  while (!glfwWindowShouldClose(window))
  {
    glfwPollEvents();
//  processInput(window);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //----Render--------------------------
    //************************************************


    // feed inputs to dear imgui, start new frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
//    ImGui::ShowDemoWindow();
    ImGui::ShowMetricsWindow();
   
    // Render dear imgui into screen
    ImGui::Render();

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
 
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }
 

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext(); 

  close(window);
  return 0;
}
