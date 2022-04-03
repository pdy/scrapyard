#ifndef PDY_SHADERS_H_
#define PDY_SHADERS_H_

#include <string>

const std::string fragColorOrangeShader = R"(#version 330 core
out vec4 FragColor;
void main()
{
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
})"; 

const std::string fragColorYellowShader = R"(#version 330 core
out vec4 FragColor;
void main()
{ 
    FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);
})";

const std::string vertexShader_Loc_0 = R"(#version 330 core
layout (location = 0) in vec3 aPos;
void main()
{
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
})";

#endif
