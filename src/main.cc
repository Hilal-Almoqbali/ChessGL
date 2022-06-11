#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <cmath>


#define STB_IMAGE_IMPLEMENTATION
#include "stbi_image.h"

struct ShadderSource
{
    std::string vertexSource;
    std::string fragmentSource;
};

static ShadderSource ParseShader(const char* filePath);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
static unsigned int CompileShadder(const char* source,unsigned int type);
static unsigned int LinkShader(unsigned int vertexShader,unsigned int fragmentShader);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;


int main (void)
{
        // glfw: initialize and configure
    // ------------------------------
    if (!glfwInit())
        {return -1;}
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    


    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    printf ("Connecting to hello world server…\n");
    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_REQ);
    zmq_connect (requester, "tcp://localhost:5555");

    int request_nbr;

 
        const char* vertexShaderSource = //source.vertexSource.c_str();
    "#version 330 core\n"
    "layout (location = 0) in vec3 vertices;\n"
    "void main()\n"
    "{\n"
     "  gl_Position = vec4(vertices, 1.0);\n"
    "};\n";
    const char* fragmentShaderSource = //source.fragmentSource.c_str();  
    "#version 330 core\n"
    "out vec4 color;\n"
    "uniform vec4 u_Color;\n"
    "void main()\n"
    "{\n"
    "  color = u_Color;\n"
    "};\n";
    // build and compile our shader program

    unsigned int vertexShader = CompileShadder(vertexShaderSource,GL_VERTEX_SHADER);

    unsigned int fragmentShader = CompileShadder(fragmentShaderSource,GL_FRAGMENT_SHADER);

    unsigned int shaderProgram = LinkShader(vertexShader,fragmentShader);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = 
    {
                // positions          // colors           // texture coords
        -1.0f, -1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
         1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
         1.0f,  1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
        -1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
    };

    unsigned int indices[] = {  // note that we start from 0!
    0, 1, 3,   // first triangle
    1, 2, 3    // second triangle
    }; 


    unsigned int vertexbuffer, vertexattrib;

    glGenBuffers(1, &vertexbuffer);
    glGenVertexArrays(1, &vertexattrib);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);


    unsigned int ibo;
    glGenBuffers(1, &ibo);
    glGenVertexArrays(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


    glBindVertexArray(vertexattrib);

 // be sure to activate the shader before any calls to glUniform
    glUseProgram(shaderProgram);

    glUseProgram(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    int locations = glGetUniformLocation(shaderProgram,"u_Color");
    //ASSERT(locations != -1);
    glUniform4f(locations,0.2f,0.3f,0.8f,1.0f);

        float r = 0.0f;
        float incresment =0.5f;

        unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
    unsigned char *board = stbi_load("./res/chessboard.png", &width, &height, &nrChannels, 0);
    if (board)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(board);


    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        
        char buffer [10];
        printf ("Sending Hello \n");
        zmq_send (requester, "Hello", 5, 0);
        zmq_recv (requester, buffer, 10, 0);
        printf ("Received World \n");
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.1f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if( r > 1.0f){incresment = -0.1f;}
        else if( r < 0.0f){incresment = 0.1f;}
        r += incresment;
       
        
        glUniform4f(locations,0.2f,r,0.3f,1.0f);

        glUseProgram(shaderProgram);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        
        glBindTexture(GL_TEXTURE_2D, texture);

        //glDrawArrays(GL_TRIANGLES, 0, 3);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT,nullptr);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &vertexattrib);
    glDeleteBuffers(1, &vertexbuffer);
    //glDeleteProgram(shaderProgram);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
    zmq_close (requester);
    zmq_ctx_destroy (context);
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

static unsigned int CompileShadder(const char* source,unsigned int type)
{
    unsigned int id = glCreateShader(type);
    const char* src = source;
    glShaderSource(id, 1,& src, NULL);
    glCompileShader(id);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(id, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::"<<(type == GL_VERTEX_SHADER ? "vertex" : "fragment")<<"::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    return id;
}

static unsigned int LinkShader(unsigned int vertexShader,unsigned int fragmentShader)
{
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    /*glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }*/
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
}

static ShadderSource ParseShader(const char* filePath)
{
    std::ifstream stream(filePath);

    enum class ShaderType
    {
        NONE =-1,VERTEX = 0,FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    while(getline(stream,line))
    {
        if(line.find("#shader") != std::string::npos)
        {
            if(line.find("vertex") != std::string::npos)
            {
                //set mode to vertex
                type = ShaderType::VERTEX;
            }
            if(line.find("#fragment") != std::string::npos)
            {
                //set mode to fragment
                type = ShaderType::FRAGMENT;
            }
            else
            {
                ss[(int)type] << line <<"\n";
            }
        }
    }
    return {ss[0].str(),ss[1].str()};
}