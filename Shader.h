#ifndef _SHADER_H_
#define _SHADER_H_

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

const GLchar* vShaderCode ="#version 300 es\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec2 texCoord;\n"
"out vec2 TexCoord;\n"
"void main()\n"
"{\n"
    "gl_Position = vec4(position, 1.0f);\n"
    "TexCoord = texCoord;\n"
"}\0" ;

const GLchar* fShaderCode ="#version 300 es\n"
"precision mediump float;\n"
"in vec2 TexCoord;\n"
"uniform sampler2D tex_y;\n"
"uniform sampler2D tex_u;\n"
"uniform sampler2D tex_v;\n"
"out vec4 fragcolor;\n"
"void main()\n"
"{\n"
    "vec3 yuv;\n"
    "vec3 rgb;\n"
    "yuv.x = texture(tex_y, TexCoord).r;\n"
    "yuv.y = texture(tex_u, TexCoord).r - 0.5;\n"
    "yuv.z = texture(tex_v, TexCoord).r - 0.5;\n"
    "rgb = mat3( 1,       1,         1,\n"
                "0,       -0.39465,  2.03211,\n"
                "1.13983, -0.58060,  0) * yuv;\n"
    "fragcolor = vec4(rgb, 1);\n"
"}\n\0" ;

class Shader
{
public:
    GLuint Program;
    // Constructor generates the shader on the fly
    Shader(int a)
    {
        // 2. Compile shaders
        GLuint vertex, fragment;
        GLint success;
        GLchar infoLog[512];
        // Vertex Shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        // Print compile errors if any
        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertex, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        // Fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        // Print compile errors if any
        glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragment, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        // Shader Program
        this->Program = glCreateProgram();
        glAttachShader(this->Program, vertex);
        glAttachShader(this->Program, fragment);
        glLinkProgram(this->Program);
        // Print linking errors if any
        glGetProgramiv(this->Program, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(this->Program, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
        // Delete the shaders as they're linked into our program now and no longer necessery
        glDeleteShader(vertex);
        glDeleteShader(fragment);

    }
    // Uses the current shader
    void Use()
    {
        glUseProgram(this->Program);
    }
    
    void Use_end()
    {
        glUseProgram(0);
    }
};

#endif
