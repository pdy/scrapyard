#include "include.h"
#include "shaders.h"


namespace pdy {

std::tuple<bool, std::string> isShaderProgramCompSuccess(unsigned int programId)
{
  int success;
  char infoLog[512];
  glGetProgramiv(programId, GL_LINK_STATUS, &success);
  if(!success)
    glGetProgramInfoLog(programId, 512, nullptr, infoLog);
 
  return std::make_tuple(success, std::string(infoLog));
}

std::tuple<bool, std::string> isShaderCompSuccess(unsigned int shaderId)
{
  int  success;
  char infoLog[512];
  glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
  if(!success)
    glGetShaderInfoLog(shaderId, 512, nullptr, infoLog);
  
  return std::make_tuple(success, std::string(infoLog));
}

std::optional<unsigned int> compileShader(const char *shaderSource, unsigned int shaderType)
{
  const unsigned int shaderId = glCreateShader(shaderType);
  glShaderSource(shaderId, 1/*source count*/, &shaderSource, nullptr);
  glCompileShader(shaderId); 
  if(const auto[status, msg] = isShaderCompSuccess(shaderId); !status)
  {
    log << "[ERROR] shader compilation error: " << msg;
    return std::nullopt;
  }

  return shaderId;
}


std::optional<Result> createRect(float ndcOrigoX, float ndcOrigoY, float ndcWidth, float ndcHeight, Color color)
{

  const auto vertexShader = compileShader(vertexShader_Loc_0.c_str(), GL_VERTEX_SHADER);
  if(!vertexShader)
    return std::nullopt; 

  log << "[INFO] compiled vertex shader";

  const std::string fragColorShaderSource = [&]{
    if(color == Color::orange)
      return fragColorOrangeShader; 

    return fragColorYellowShader; 
  }();


  const auto colorShader = compileShader(fragColorShaderSource.c_str(), GL_FRAGMENT_SHADER);
  if(!colorShader)
    return std::nullopt;
    
  log << "[INFO] fragment color shader";

  
  unsigned int shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, *vertexShader);
  glAttachShader(shaderProgram, *colorShader);
  glLinkProgram(shaderProgram);
  glDeleteShader(*vertexShader);
  glDeleteShader(*colorShader);

  if(const auto[status, msg] = isShaderProgramCompSuccess(shaderProgram); !status)
  {
    log << "[ERROR] Shader program compilation failed " << msg;
    return std::nullopt;
  }

  log << "[INFO] compiled shader program";
 
  const float vertices[] {
    ndcOrigoX,             ndcOrigoY,             .0, // left high - ORIGO
    ndcOrigoX,             ndcOrigoY - ndcHeight, .0, // left low
    ndcOrigoX + ndcWidth,  ndcOrigoY - ndcHeight, .0, // right low
    ndcOrigoX + ndcWidth,  ndcOrigoY,             .0  // right high
   };

  const unsigned int indices []{
    0,1,2,
    0,3,2
  };

  const unsigned int VAO = []{ unsigned int ret; glGenVertexArrays(1, &ret); return ret;}();
  const unsigned int VBO = []{ unsigned int ret; glGenBuffers(1, &ret); return ret; }();
  const unsigned int EBO = []{ unsigned int ret; glGenBuffers(1, &ret); return ret; }();

  // 1. bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
  glBindVertexArray(VAO); 

  // 2. copy our vertices array in a buffer for OpenGL to use
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // GL_STATIC_DRAW cause we're setting position which does not change

  // 3. copy our index array in a element buffer for OpenGL to use
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  // 3. then set our vertex attributes pointers
  glVertexAttribPointer(
      0,                          // location = 0 in vertexShader
      3,                          // vec3 in vertexShader - size of vertex attribute
      GL_FLOAT,                   // type of data
      GL_FALSE,                   // normalize
      3 * sizeof(float),          // stride - 3x of float we have next set of data
      reinterpret_cast<void*>(0));// offset of where the position data begins in buffer

  glEnableVertexAttribArray(0);   // enable vertex attribute at location 0

  return Result{ VAO, shaderProgram, std::size(indices)};
}

std::optional<Result> createTwoRects()
{
  const auto vertexShader = compileShader(vertexShader_Loc_0.c_str(), GL_VERTEX_SHADER);
  if(!vertexShader)
    return std::nullopt; 

  log << "[INFO] compiled vertex shader";

  const auto colorShader = compileShader(fragColorOrangeShader.c_str(), GL_FRAGMENT_SHADER);
  if(!colorShader)
    return std::nullopt;
    
  log << "[INFO] fragment color shader";

  
  unsigned int shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, *vertexShader);
  glAttachShader(shaderProgram, *colorShader);
  glLinkProgram(shaderProgram);
  glDeleteShader(*vertexShader);
  glDeleteShader(*colorShader);

  if(const auto[status, msg] = isShaderProgramCompSuccess(shaderProgram); !status)
  {
    log << "[ERROR] Shader program compilation failed " << msg;
    return std::nullopt;
  }

  log << "[INFO] compiled shader program";
 
  const float vertices[] {
    // left
    -0.8f, -0.1f, .0, // left low
    -0.1f, -0.1f, .0, // right low
    -0.8f,  0.8f, .0, // left high
    -0.1f,  0.8f, .0,  // right high

    // right
     0.1f, -0.1f, .0, // left low
     0.8f, -0.1f, .0, // right low
     0.1f,  0.8f, .0, // left high
     0.8f,  0.8f, .0,  // right high
  };

//  const unsigned int indices []{0,1,2,3};
  const unsigned int indicesLeft []{
    // left
    0,1,3,
    0,2,3,

    // right
    4,5,7,
    4,6,7
  };

  const unsigned int VAO = []{ unsigned int ret; glGenVertexArrays(1, &ret); return ret;}();
  const unsigned int VBO = []{ unsigned int ret; glGenBuffers(1, &ret); return ret; }();
  const unsigned int EBO = []{ unsigned int ret; glGenBuffers(1, &ret); return ret; }();
  // const unsigned int EBO_RIGHT = []{ unsigned int ret; glGenBuffers(1, &ret); return ret; }();

  // 1. bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
  glBindVertexArray(VAO); 

  // 2. copy our vertices array in a buffer for OpenGL to use
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // GL_STATIC_DRAW cause we're setting position which does not change

  // 3. copy our index array in a element buffer for OpenGL to use
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesLeft), indicesLeft, GL_STATIC_DRAW);

 // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_RIGHT);
 // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesRight), indicesRight, GL_STATIC_DRAW);


  // 3. then set our vertex attributes pointers
  glVertexAttribPointer(
      0,                          // location = 0 in vertexShader
      3,                          // vec3 in vertexShader - size of vertex attribute
      GL_FLOAT,                   // type of data
      GL_FALSE,                   // normalize
      3 * sizeof(float),          // stride - 3x of float we have next set of data
      reinterpret_cast<void*>(0));// offset of where the position data begins in buffer

  glEnableVertexAttribArray(0);   // enable vertex attribute at location 0

  return Result{ VAO, shaderProgram, std::size(indicesLeft)};
}

std::optional<Result> createRectangleGLBuffer()
{
  const auto vertexShader = compileShader(vertexShader_Loc_0.c_str(), GL_VERTEX_SHADER);
  if(!vertexShader)
    return std::nullopt; 

  log << "[INFO] compiled vertex shader";

  const auto colorShader = compileShader(fragColorOrangeShader.c_str(), GL_FRAGMENT_SHADER);
  if(!colorShader)
    return std::nullopt;
    
  log << "[INFO] fragmend color shader";

  unsigned int shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, *vertexShader);
  glAttachShader(shaderProgram, *colorShader);
  glLinkProgram(shaderProgram);
  glDeleteShader(*vertexShader);
  glDeleteShader(*colorShader);

  if(const auto[status, msg] = isShaderProgramCompSuccess(shaderProgram); !status)
  {
    log << "[ERROR] Shader program compilation failed " << msg;
    return std::nullopt;
  }

  log << "[INFO] compiled shader program";

  const float vertices[] {
    -0.7f, -0.7f, .0, // left low
     0.7f, -0.7f, .0, // right low
    -0.7f,  0.7f, .0, // left high
     0.7f,  0.7f, .0  // right high
  };

//  const unsigned int indices []{0,1,2,3};
  const unsigned int indices []{
    0,1,3,
    0,2,3
  };

  const unsigned int VAO = []{ unsigned int ret; glGenVertexArrays(1, &ret); return ret;}();
  const unsigned int VBO = []{ unsigned int ret; glGenBuffers(1, &ret); return ret; }();
  const unsigned int EBO = []{ unsigned int ret; glGenBuffers(1, &ret); return ret; }();

  // 1. bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
  glBindVertexArray(VAO); 

  // 2. copy our vertices array in a buffer for OpenGL to use
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // GL_STATIC_DRAW cause we're setting position which does not change

  // 3. copy our index array in a element buffer for OpenGL to use
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  // 3. then set our vertex attributes pointers
  glVertexAttribPointer(
      0,                          // location = 0 in vertexShader
      3,                          // vec3 in vertexShader - size of vertex attribute
      GL_FLOAT,                   // type of data
      GL_FALSE,                   // normalize
      3 * sizeof(float),          // stride - 3x of float we have next set of data
      reinterpret_cast<void*>(0));// offset of where the position data begins in buffer

  glEnableVertexAttribArray(0);   // enable vertex attribute at location 0

  return Result{ VAO, shaderProgram, std::size(indices) };
}


std::optional<ResultEx> createTwoTrianglesNextToEachOtherDifferentColors()
{
  const auto vertexShader = compileShader(vertexShader_Loc_0.c_str(), GL_VERTEX_SHADER);
  if(!vertexShader)
    return std::nullopt; 

  log << "[INFO] compiled vertex shader";

  const auto colorShader = compileShader(fragColorOrangeShader.c_str(), GL_FRAGMENT_SHADER);
  if(!colorShader)
    return std::nullopt;
    
  log << "[INFO] fragmend color shader";

  const auto colorYellowShader = compileShader(fragColorYellowShader.c_str(), GL_FRAGMENT_SHADER);
  if(!colorYellowShader)
    return std::nullopt;

  log << "[INFO] yellow fragment shader compiled";

  unsigned int shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, *vertexShader);
  glAttachShader(shaderProgram, *colorShader);
  glLinkProgram(shaderProgram);
  glDeleteShader(*colorShader);

  if(const auto[status, msg] = isShaderProgramCompSuccess(shaderProgram); !status)
  {
    log << "[ERROR] Shader program compilation failed " << msg;
    return std::nullopt;
  }

  log << "[INFO] compiled shader program";

  unsigned int shaderYellowProgram = glCreateProgram();
  glAttachShader(shaderYellowProgram, *vertexShader);
  glAttachShader(shaderYellowProgram, *colorYellowShader);
  glLinkProgram(shaderYellowProgram);
  glDeleteShader(*vertexShader);
  glDeleteShader(*colorYellowShader);

  if(const auto[status, msg] = isShaderProgramCompSuccess(shaderYellowProgram); !status)
  {
    log << "[ERROR] Yellow shader program compilation failed " << msg;
    return std::nullopt;
  }

  log << "[INFO] compiled yellow shader program";


  const float vertices[] {
    // first triangle - left; pointing top
    -0.5f,  0.8f, .0, // top 
    -0.8f,  0.2f, .0, // left 
    -0.2f,  0.2f, .0, // right 

    // second triangle - right; pointing down
     0.5f,  0.2f, .0, // bottom 
     0.2f,  0.8f, .0, // left 
     0.8f,  0.8f, .0, // right
  };


  const unsigned int VAO = []{ unsigned int ret; glGenVertexArrays(1, &ret); return ret;}();
  const unsigned int VBO = []{ unsigned int ret; glGenBuffers(1, &ret); return ret; }();

  // 1. bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
  glBindVertexArray(VAO); 

  // 2. copy our vertices array in a buffer for OpenGL to use
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // GL_STATIC_DRAW cause we're setting position which does not change

  // 3. then set our vertex attributes pointers
  glVertexAttribPointer(
      0,                          // location = 0 in vertexShader
      3,                          // vec3 in vertexShader - size of vertex attribute
      GL_FLOAT,                   // type of data
      GL_FALSE,                   // normalize
      3 * sizeof(float),          // stride - 3x of float we have next set of data
      reinterpret_cast<void*>(0));// offset of where the position data begins in buffer

  glEnableVertexAttribArray(0);   // enable vertex attribute at location 0

  return ResultEx{ VAO, shaderProgram, shaderYellowProgram, 3, 3};
}

std::optional<Result> createTwoTrianglesNextToEachOther()
{
  const auto vertexShader = compileShader(vertexShader_Loc_0.c_str(), GL_VERTEX_SHADER);
  if(!vertexShader)
    return std::nullopt; 

  log << "[INFO] compiled vertex shader";

  const auto colorShader = compileShader(fragColorOrangeShader.c_str(), GL_FRAGMENT_SHADER);
  if(!colorShader)
    return std::nullopt;
    
  log << "[INFO] fragmend color shader";

  unsigned int shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, *vertexShader);
  glAttachShader(shaderProgram, *colorShader);
  glLinkProgram(shaderProgram);
  glDeleteShader(*vertexShader);
  glDeleteShader(*colorShader);

  if(const auto[status, msg] = isShaderProgramCompSuccess(shaderProgram); !status)
  {
    log << "[ERROR] Shader program compilation failed " << msg;
    return std::nullopt;
  }

  log << "[INFO] compiled shader program";

  const float vertices[] {
    // first triangle - left; pointing top
    -0.3f,  0.3f, .0, // top 
    -0.6f, -0.3f, .0, // left 
     0.0f, -0.3f, .0, // right 

    // second triangle - right; pointing down
     0.3f, -0.3f, .0, // bottom 
     0.0f,  0.3f, .0, // left 
     0.6f,  0.3f, .0, // right
  };


  const unsigned int VAO = []{ unsigned int ret; glGenVertexArrays(1, &ret); return ret;}();
  const unsigned int VBO = []{ unsigned int ret; glGenBuffers(1, &ret); return ret; }();

  // 1. bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
  glBindVertexArray(VAO); 

  // 2. copy our vertices array in a buffer for OpenGL to use
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // GL_STATIC_DRAW cause we're setting position which does not change

  // 3. then set our vertex attributes pointers
  glVertexAttribPointer(
      0,                          // location = 0 in vertexShader
      3,                          // vec3 in vertexShader - size of vertex attribute
      GL_FLOAT,                   // type of data
      GL_FALSE,                   // normalize
      3 * sizeof(float),          // stride - 3x of float we have next set of data
      reinterpret_cast<void*>(0));// offset of where the position data begins in buffer

  glEnableVertexAttribArray(0);   // enable vertex attribute at location 0

  return Result{ VAO, shaderProgram, std::size(vertices) / 3 };
}

std::optional<Result> createTriangle_1()
{
  const std::string vertexShaderSource = R"(#version 330 core
layout (location = 0) in vec3 aPos;
void main()
{
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
})";

  const auto vertexShader = compileShader(vertexShaderSource.c_str(), GL_VERTEX_SHADER);
  if(!vertexShader)
    return std::nullopt; 

  log << "[INFO] compiled vertex shader";

  const std::string fragColorShaderSource = R"(#version 330 core
out vec4 FragColor;
void main()
{
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
})";


  const auto colorShader = compileShader(fragColorShaderSource.c_str(), GL_FRAGMENT_SHADER);
  if(!colorShader)
    return std::nullopt;
    
  log << "[INFO] fragmend color shader";

  unsigned int shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, *vertexShader);
  glAttachShader(shaderProgram, *colorShader);
  glLinkProgram(shaderProgram);
  glDeleteShader(*vertexShader);
  glDeleteShader(*colorShader);

  if(const auto[status, msg] = isShaderProgramCompSuccess(shaderProgram); !status)
  {
    log << "[ERROR] Shader program compilation failed " << msg;
    return std::nullopt;
  }

  log << "[INFO] compiled shader program";

  const float vertices[] {
    -0.3f,  0.3f, .0, // top 
    -0.6f, -0.3f, .0, // left 
     0.0f, -0.3f, .0, // right 
  };


  const unsigned int VAO = []{ unsigned int ret; glGenVertexArrays(1, &ret); return ret;}();
  const unsigned int VBO = []{ unsigned int ret; glGenBuffers(1, &ret); return ret; }();

  // 1. bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
  glBindVertexArray(VAO); 

  // 2. copy our vertices array in a buffer for OpenGL to use
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // GL_STATIC_DRAW cause we're setting position which does not change

  // 3. then set our vertex attributes pointers
  glVertexAttribPointer(
      0,                          // location = 0 in vertexShader
      3,                          // vec3 in vertexShader - size of vertex attribute
      GL_FLOAT,                   // type of data
      GL_FALSE,                   // normalize
      3 * sizeof(float),          // stride - 3x of float we have next set of data
      reinterpret_cast<void*>(0));// offset of where the position data begins in buffer

  glEnableVertexAttribArray(0);   // enable vertex attribute at location 0

  return Result{ VAO, shaderProgram, std::size(vertices) / 3 };
}

std::optional<Result> createTriangle_2()
{
  const std::string vertexShaderSource = R"(#version 330 core
layout (location = 0) in vec3 aPos;
void main()
{
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
})";

  const auto vertexShader = compileShader(vertexShaderSource.c_str(), GL_VERTEX_SHADER);
  if(!vertexShader)
    return std::nullopt; 

  log << "[INFO] compiled vertex shader";

  const std::string fragColorShaderSource = R"(#version 330 core
out vec4 FragColor;
void main()
{
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
})";


  const auto colorShader = compileShader(fragColorShaderSource.c_str(), GL_FRAGMENT_SHADER);
  if(!colorShader)
    return std::nullopt;
    
  log << "[INFO] fragmend color shader";

  unsigned int shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, *vertexShader);
  glAttachShader(shaderProgram, *colorShader);
  glLinkProgram(shaderProgram);
  glDeleteShader(*vertexShader);
  glDeleteShader(*colorShader);

  if(const auto[status, msg] = isShaderProgramCompSuccess(shaderProgram); !status)
  {
    log << "[ERROR] Shader program compilation failed " << msg;
    return std::nullopt;
  }

  log << "[INFO] compiled shader program";

  const float vertices[] {
     0.3f, -0.3f, .0, // bottom 
     0.0f,  0.3f, .0, // left 
     0.6f,  0.3f, .0, // right
  };


  const unsigned int VAO = []{ unsigned int ret; glGenVertexArrays(1, &ret); return ret;}();
  const unsigned int VBO = []{ unsigned int ret; glGenBuffers(1, &ret); return ret; }();

  // 1. bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
  glBindVertexArray(VAO); 

  // 2. copy our vertices array in a buffer for OpenGL to use
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // GL_STATIC_DRAW cause we're setting position which does not change

  // 3. then set our vertex attributes pointers
  glVertexAttribPointer(
      0,                          // location = 0 in vertexShader
      3,                          // vec3 in vertexShader - size of vertex attribute
      GL_FLOAT,                   // type of data
      GL_FALSE,                   // normalize
      3 * sizeof(float),          // stride - 3x of float we have next set of data
      reinterpret_cast<void*>(0));// offset of where the position data begins in buffer

  glEnableVertexAttribArray(0);   // enable vertex attribute at location 0

  return Result{ VAO, shaderProgram, std::size(vertices) / 3 };
}

}// namespace pdy
