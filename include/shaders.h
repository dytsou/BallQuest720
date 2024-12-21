#ifndef SHADERS_H
#define SHADERS_H

#include <string>
#include <iostream>
#include <GL/glew.h>

// Vertex shader for basic rendering
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aNormal;
    
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    
    out vec3 Normal;
    out vec3 FragPos;
    
    void main() {
        FragPos = vec3(model * vec4(aPos, 1.0));
        Normal = mat3(transpose(inverse(model))) * aNormal;
        gl_Position = projection * view * model * vec4(aPos, 1.0);
    }
)";

// Fragment shader for basic rendering
const char* fragmentShaderSource = R"(
    #version 330 core
    in vec3 Normal;
    in vec3 FragPos;
    
    uniform vec3 objectColor;
    uniform vec3 lightPos;
    uniform vec3 viewPos;
    
    out vec4 FragColor;
    
    void main() {
        // Ambient light
        float ambientStrength = 0.2;
        vec3 ambient = ambientStrength * vec3(1.0);
        
        // Diffuse light
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * vec3(1.0);
        
        // Specular light
        float specularStrength = 0.5;
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = specularStrength * spec * vec3(1.0);
        
        vec3 result = (ambient + diffuse + specular) * objectColor;
        FragColor = vec4(result, 1.0);
    }
)";

// Rainbow ball fragment shader
const char* rainbowFragmentShaderSource = R"(
    #version 330 core
    in vec3 Normal;
    in vec3 FragPos;
    
    uniform float time;
    
    out vec4 FragColor;
    
    void main() {
        vec3 rainbow = vec3(
            sin(time * 2.0) * 0.5 + 0.5,
            sin(time * 2.0 + 2.094) * 0.5 + 0.5,
            sin(time * 2.0 + 4.189) * 0.5 + 0.5
        );
        FragColor = vec4(rainbow, 1.0);
    }
)";

// Shader compilation and program creation functions
GLuint createShader(const char* source, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    // Check for compilation errors
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "Shader compilation error: " << infoLog << std::endl;
    }
    
    return shader;
}

GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = createShader(vertexSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = createShader(fragmentSource, GL_FRAGMENT_SHADER);
    
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    // Check for linking errors
    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Shader program linking error: " << infoLog << std::endl;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return shaderProgram;
}

#endif // SHADERS_H 