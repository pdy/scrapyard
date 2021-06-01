#include "cmdline/cmdline.h"
#include "simplelog/simplelog.h"

#include "include.h"


static constexpr const char* programName = "GLFW window";

constexpr static int
  windowWidth = 1600,
  windowHeight = 1200;

constexpr static float
  backgroundR = 0.2f,
  backgroundG = 0.3f,
  backgroundB = 0.3f;

//static void keyCallback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
//{
//  if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
//    glfwSetWindowShouldClose(window, GLFW_TRUE);
//}

static std::string getGLString(unsigned int strId)
{
  const unsigned char *str = glGetString(strId);
  const unsigned char *it = str;
  
  std::string ret;
  while(it && *it)
  {
    ret.push_back(static_cast<char>(*it));
    ++it;
  }

  return ret;
}

static std::string glVerStr()
{
  return getGLString(GL_VERSION);
}

static std::string glShadingLangVerStr()
{
  return getGLString(GL_SHADING_LANGUAGE_VERSION);
}

static void processInput(GLFWwindow *window)
{
  
  if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

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
   

  GLFWwindow *window = pdy::initGLFW(programName, windowWidth, windowHeight);
  if(!window)
   return -1; 

  if (!gladLoadGLLoader( reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
  {
    log << "[ERROR] Couldn't initialize GLAD";
    pdy::terminateGLFW(window);
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

  log << "GL_VERSION: " << glVerStr();
  log << "GL_SHADING_LANGUAGE_VERSION: " << glShadingLangVerStr();

  ImGui::StyleColorsDark();
  // ImGui::StyleColorsLight();

  const std::string glslVersion = "#version 150";
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glslVersion.c_str());

/*
  const auto rect = pdy::createRectangleGLBuffer();
  if(!rect)
    return -1;


  const auto triang = pdy::createTwoTrianglesNextToEachOther();
  if(!triang)
    return -1;



  const auto triang_1 = pdy::createTriangle_1();
  if(!triang_1)
    return -1;

  const auto triang_2 = pdy::createTriangle_2();
  if(!triang_2)
    return -1;


  const auto twoTriangs = pdy::createTwoTrianglesNextToEachOtherDifferentColors();
  if(!twoTriangs)
    return -1;

  const auto twoRects = pdy::createTwoRects();
  if(!twoRects)
    return -1;
*/

  const auto leftRect = pdy::createRect(-0.8f, 0.8f, 0.7f, 0.7f);
  if(!leftRect)
  {
    log << "[ERROR] Failed left rect";
    return -1;
  }

  const auto rightRect = pdy::createRect(0.1f, 0.8f, 0.7f, 0.7f, pdy::Color::yellow);
  if(!leftRect)
  {
    log << "[ERROR] Failed right rect";
    return -1;
  }

  // uncomment this call to draw in wireframe polygons.
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // --- rendering loop
  while (!glfwWindowShouldClose(window))
  {
    glfwPollEvents();
    processInput(window);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //----Render--------------------------
    //************************************************
 
    // feed inputs to dear imgui, start new frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    //ImGui::ShowDemoWindow();
    ImGui::ShowMetricsWindow();



    //-----Frame---------------------------
    //**************************************************************************
    glUseProgram(leftRect->shaderProgram);
    glBindVertexArray(leftRect->VAO);
//    glDrawArrays(GL_TRIANGLES, 0, twoTriangs->shader1Count);
    glDrawElements(GL_TRIANGLES, leftRect->count, GL_UNSIGNED_INT, nullptr);

    glUseProgram(rightRect->shaderProgram);
    glBindVertexArray(rightRect->VAO);
    glDrawElements(GL_TRIANGLES, rightRect->count, GL_UNSIGNED_INT, nullptr);

//    glBindVertexArray(triang_2->VAO);
//    glDrawArrays(GL_TRIANGLES, twoTriangs->shader1Count, twoTriangs->shader2Count);

    //-----End Frame---------------------------
    //**************************************************************************

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

  pdy::terminateGLFW(window);
  return 0;
}
