#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif
#pragma comment(lib, "winmm.lib")

#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"
#include <windows.h>
#include <mmsystem.h>
#include <vector>

#include <iostream>

std::vector<glm::vec3> cameraPositions = {
    glm::vec3(1.0f, 3.0f, 3.0f),
    glm::vec3(1.0f, 2.5f, 3.0f),
    glm::vec3(-2.0f, 3.0f, 3.0f),
    glm::vec3(0.0f, 5.0f, 0.0f),

};

std::vector<glm::vec3> cameraTargets = {
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),

};
int currentTourPoint = 0;
bool autoTourActive = false;
float tourProgress = 0.0f;
const float TOUR_SPEED = 0.003f;
float lightIntensity = 1.0f;

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

glm::mat4 apamodel;
glm::mat4 apaview;
glm::mat4 apaprojection;
glm::mat3 apanormalMatrix;


// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

glm::vec3 apalightDir;
glm::vec3 apalightColor;


// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;

GLint apamodelLoc;
GLint apaviewLoc;
GLint apaprojectionLoc;
GLint apanormalMatrixLoc;


GLint lightDirLoc;
GLint lightColorLoc;
GLint apalightDirLoc;
GLint apalightColorLoc;
GLboolean dirLightEnabled = true;
GLint dirLightEnabledLoc;
GLint apadirLightEnabledLoc;
// camera
gps::Camera myCamera(
    glm::vec3(0.0f, 1.0f, 3.0f),
    glm::vec3(0.0f, 1.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.1f;

GLboolean pressedKeys[1024];

// models
gps::Model3D teapot;
gps::Model3D banner;

GLfloat angle;

gps::SkyBox mySkyBox;
struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    float life;
    float size;
};

// shaders
gps::Shader skyboxShader;
gps::Shader myBasicShader;
gps::Shader apa;

glm::vec3 pointLightPos;
glm::vec3 pointLightColor;
GLboolean pointLightEnabled = false;

GLint pointLightPosLoc;
GLint pointLightColorLoc;
GLint pointLightEnabledLoc;

glm::vec3 apapointLightPos;
glm::vec3 apapointLightColor;
GLboolean apapointLightEnabled = false;

GLint apapointLightPosLoc;
GLint apapointLightColorLoc;
GLint apapointLightEnabledLoc;


const float GRID_WIDTH = 0.25f;
const float GRID_HEIGHT = 0.25f;
const int GRID_NUM_POINTS_WIDTH = 50;
const int GRID_NUM_POINTS_HEIGHT = 50;

//VBO, EBO and VAO
GLuint gridPointsVBO;
GLuint gridTrianglesEBO;
GLuint gridVAO;

//texture
const float textureRepeatU = 1.0f; //number of times to repeat seamless texture on u axis
const float textureRepeatV = 1.0f; //number of times to repeat seamless texture on v axis
GLuint gridTexture;
GLint gridTextureLoc;

float simTime;
GLint simTimeLoc;

void initVBOs() {

    glGenVertexArrays(1, &gridVAO);
    glBindVertexArray(gridVAO);

    //prepare vertex data to send to shader
    static GLfloat vertexData[GRID_NUM_POINTS_WIDTH * GRID_NUM_POINTS_HEIGHT * 4];

    //for each vertex in grid
    for (unsigned int i = 0; i < GRID_NUM_POINTS_HEIGHT; i++) {
        for (unsigned int j = 0; j < GRID_NUM_POINTS_WIDTH; j++) {

            //tex coords
            vertexData[4 * (i * GRID_NUM_POINTS_WIDTH + j) + 0] = j * textureRepeatU / (float)(GRID_NUM_POINTS_WIDTH - 1);
            vertexData[4 * (i * GRID_NUM_POINTS_WIDTH + j) + 1] = textureRepeatV - i * textureRepeatV / (float)(GRID_NUM_POINTS_HEIGHT - 1);
            //xy position indices in grid (for computing sine function)
            vertexData[4 * (i * GRID_NUM_POINTS_WIDTH + j) + 2] = (float)j;
            vertexData[4 * (i * GRID_NUM_POINTS_WIDTH + j) + 3] = (float)i;
        }
    }

    glGenBuffers(1, &gridPointsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, gridPointsVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

    //prepare triangle indices to send to shader
    static GLuint triangleIndices[(GRID_NUM_POINTS_WIDTH - 1) * (GRID_NUM_POINTS_HEIGHT - 1) * 2 * 3];

    //for each square/rectangle in grid (each four neighboring points)
    for (unsigned int i = 0; i < GRID_NUM_POINTS_HEIGHT - 1; i++) {
        for (unsigned int j = 0; j < GRID_NUM_POINTS_WIDTH - 1; j++) {

            //lower triangle
            triangleIndices[6 * (i * (GRID_NUM_POINTS_WIDTH - 1) + j)] = i * GRID_NUM_POINTS_WIDTH + j;
            triangleIndices[6 * (i * (GRID_NUM_POINTS_WIDTH - 1) + j) + 1] = (i + 1) * GRID_NUM_POINTS_WIDTH + j;
            triangleIndices[6 * (i * (GRID_NUM_POINTS_WIDTH - 1) + j) + 2] = (i + 1) * GRID_NUM_POINTS_WIDTH + j + 1;

            //upper triangle
            triangleIndices[6 * (i * (GRID_NUM_POINTS_WIDTH - 1) + j) + 3] = i * GRID_NUM_POINTS_WIDTH + j;
            triangleIndices[6 * (i * (GRID_NUM_POINTS_WIDTH - 1) + j) + 4] = (i + 1) * GRID_NUM_POINTS_WIDTH + j + 1;
            triangleIndices[6 * (i * (GRID_NUM_POINTS_WIDTH - 1) + j) + 5] = i * GRID_NUM_POINTS_WIDTH + j + 1;
        }
    }

    glGenBuffers(1, &gridTrianglesEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gridTrianglesEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangleIndices), triangleIndices, GL_STATIC_DRAW);

    //split vertex attributes

    //tex coords
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    //grid XY indices
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

}

float fogDensity = 0.0f;
GLuint fogDensityLoc;
GLenum glCheckError_(const char* file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
        case GL_INVALID_ENUM:
            error = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "INVALID_VALUE";
            break;
           // case GL_INVALID_OPERATION:
           //     error = "INVALID_OPERATION";
            break;
        case GL_OUT_OF_MEMORY:
            error = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        }
       // std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
    glViewport(0, 0, width, height);
    projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 20.0f);
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        }
        else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

float lastX = 400, lastY = 300; // center of the window
float yaw = -90.0f, pitch = 0.0f;
GLboolean firstMouse = true;

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {  // First mouse movement
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed
    lastX = xpos;
    lastY = ypos;

    const float sensitivity = 0.5f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Constrain pitch angle to avoid flipping
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    myCamera.rotate(pitch, yaw); // Update
}
bool gridAnimationActive = false;

void processMovement() {
    if (pressedKeys[GLFW_KEY_W]) {
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_S]) {
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_A]) {
        myCamera.move(gps::MOVE_LEFT, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_D]) {
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_Q]) {
        angle -= 1.0f;
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_E]) {
        angle += 1.0f;
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_SPACE]) {
        myCamera.move(gps::MOVE_UP, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_LEFT_SHIFT]) {
        myCamera.move(gps::MOVE_DOWN, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_1]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    if (pressedKeys[GLFW_KEY_2]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }
    if (pressedKeys[GLFW_KEY_3]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }


    if (pressedKeys[GLFW_KEY_K]) {
        pointLightEnabled = !pointLightEnabled; // Toggle light state
        myBasicShader.useShaderProgram();
        glUniform1i(pointLightEnabledLoc, pointLightEnabled);

        if (pointLightEnabled == true)
        {
            lightIntensity = 1.4f;
        }
        else lightIntensity = 0.05f;


        apa.useShaderProgram();
        lightColor = glm::vec3(1.0f, 1.0f, 1.0f) * lightIntensity;
        glUniform3fv(apalightColorLoc, 1, glm::value_ptr(lightColor));
        pressedKeys[GLFW_KEY_K] = false;
    }

    if (pressedKeys[GLFW_KEY_I]) {
        lightIntensity += 0.015f;
        if (lightIntensity > 2.0f) lightIntensity = 2.0f;
        pressedKeys[GLFW_KEY_I] = false;

        myBasicShader.useShaderProgram();
        lightColor = glm::vec3(1.0f, 1.0f, 1.0f) * lightIntensity;
        glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

        apa.useShaderProgram();
        lightColor = glm::vec3(1.0f, 1.0f, 1.0f) * lightIntensity;
        glUniform3fv(apalightColorLoc, 1, glm::value_ptr(lightColor));
    }

    if (pressedKeys[GLFW_KEY_J]) {
        lightIntensity -= 0.01f;
        if (lightIntensity < 0.0f) lightIntensity = 0.0f;

        myBasicShader.useShaderProgram();
        lightColor = glm::vec3(1.0f, 1.0f, 1.0f) * lightIntensity;
        glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

        apa.useShaderProgram();
        lightColor = glm::vec3(1.0f, 1.0f, 1.0f) * lightIntensity;
        glUniform3fv(apalightColorLoc, 1, glm::value_ptr(lightColor));

    }


    

    if (pressedKeys[GLFW_KEY_O] && !autoTourActive && !cameraPositions.empty()) {
        PlaySound(TEXT("UCL2.wav"), NULL, SND_ASYNC);
        gridAnimationActive = true; 
        autoTourActive = true;
        currentTourPoint = 0;
        tourProgress = 0.0f;
        pressedKeys[GLFW_KEY_O] = false;
    }

    if (autoTourActive) {

        if (currentTourPoint < cameraPositions.size() - 1) {
            tourProgress += TOUR_SPEED;

            if (currentTourPoint == 2)
                if (tourProgress >= 0.9f)
                {
                    autoTourActive = false;
                    gridAnimationActive = false;
                }
                    

            if (tourProgress >= 1.0f) {
                tourProgress = 0.0f;
                currentTourPoint++;

            }

            //interpolare

            glm::vec3 p0 = cameraPositions[currentTourPoint];
            glm::vec3 p1 = cameraPositions[currentTourPoint + 1];
            glm::vec3 newCameraPosition = p1 * tourProgress + p0 * (1.0f - tourProgress);

            glm::vec3 t0 = cameraTargets[currentTourPoint];
            glm::vec3 t1 = cameraTargets[currentTourPoint + 1];
            glm::vec3 newCameraTarget = t1 * (tourProgress + 0.1f) + t0 * (0.9f - tourProgress);

            myCamera.setPosition(newCameraPosition);
            myCamera.setTarget(newCameraTarget);

        }
        else {
            autoTourActive = false;
        }
    }
    if (pressedKeys[GLFW_KEY_G]) {//Disable fog
        fogDensity = 0;
        myBasicShader.useShaderProgram();
        glUniform1f(fogDensityLoc, fogDensity);
    }
    if (pressedKeys[GLFW_KEY_F]) {//Enable fog
        fogDensity = 0.08f;
        myBasicShader.useShaderProgram();
        glUniform1f(fogDensityLoc, fogDensity);
    }

    // Update the view matrix after any movement or rotation
    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // Update normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

}


void initOpenGLWindow() {
    myWindow.Create(1024, 768, "UEFA CHAMPIONS LEAGUE");
}

void setWindowCallbacks() {
    glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() {
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
    glEnable(GL_CULL_FACE); // cull face
    glCullFace(GL_BACK); // cull back face
    glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
    teapot.LoadModel("models/teapot/13500_autosave.obj");
    banner.LoadModel("models/teapot/banner.obj");

}

void initSkybox() {
    std::vector<const GLchar*> faces;
    faces.push_back("skybox/right.tga");
    faces.push_back("skybox/left.tga");
    faces.push_back("skybox/top.tga");
    faces.push_back("skybox/bottom.tga");
    faces.push_back("skybox/back.tga");
    faces.push_back("skybox/front.tga");
    mySkyBox.Load(faces);
}

void initShaders() {

    apa.loadShader(
        "shaders/basicWave.vert",
        "shaders/basicWave.frag");



    myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");

    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyboxShader.useShaderProgram();
}

void initUniforms() {
    myBasicShader.useShaderProgram();



    // create model matrix for teapot
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

    // get view matrix for current camera
    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    // send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

    // create projection matrix
    projection = glm::perspective(glm::radians(45.0f),
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
        0.1f, 20.0f);
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    // send projection matrix to shader
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //set the light direction (direction towards the light)
    lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
    lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
    // send light dir to shader
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

    //set light color
    lightColor = glm::vec3(1.0f, 1.0f, 1.0f) * lightIntensity; //white light
    lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
    // send light color to shader
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    dirLightEnabledLoc = glGetUniformLocation(myBasicShader.shaderProgram, "dirLightEnabled");
    glUniform1i(dirLightEnabledLoc, dirLightEnabled);

    pointLightPos = glm::vec3(0.0f, -0.5f, 0.0f);
    pointLightPosLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightPos");
    glUniform3fv(pointLightPosLoc, 1, glm::value_ptr(pointLightPos));

    pointLightColor = glm::vec3(2.0f, 2.0f, 2.0f);
    pointLightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightColor");
    glUniform3fv(pointLightColorLoc, 1, glm::value_ptr(pointLightColor));

    pointLightEnabledLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightEnabled");
    glUniform1i(pointLightEnabledLoc, pointLightEnabled);

    fogDensityLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogDensity");
    glUniform1f(fogDensityLoc, fogDensity);

}


void initGridUniforms() {

    apa.useShaderProgram();

    // create model matrix for grid
    apamodel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.33f, 0.11f));
    apamodelLoc = glGetUniformLocation(apa.shaderProgram, "apamodel");

    // get view matrix for current camera
    apaview = myCamera.getViewMatrix();
    apaviewLoc = glGetUniformLocation(apa.shaderProgram, "apaview");
    // send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(apaview));

    // compute normal matrix grid
    apanormalMatrix = glm::mat3(glm::inverseTranspose(apaview * apamodel));
    apanormalMatrixLoc = glGetUniformLocation(apa.shaderProgram, "apanormalMatrix");

    // create projection matrix
    apaprojection = glm::perspective(glm::radians(45.0f),
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
        0.1f, 50.0f);
    apaprojectionLoc = glGetUniformLocation(apa.shaderProgram, "apaprojection");
    // send projection matrix to shader
    glUniformMatrix4fv(apaprojectionLoc, 1, GL_FALSE, glm::value_ptr(apaprojection));
    //set the light direction (direction towards the light)
    apalightDir = glm::vec3(0.0f, 1.0f, 1.0f);
    apalightDirLoc = glGetUniformLocation(apa.shaderProgram, "apalightDir");
    // send light dir to shader
    glUniform3fv(apalightDirLoc, 1, glm::value_ptr(apalightDir));

    //set light color
    apalightColor = glm::vec3(1.0f, 1.0f, 1.0f) * lightIntensity; //white light
    apalightColorLoc = glGetUniformLocation(apa.shaderProgram, "apalightColor");
    // send light color to shader
    glUniform3fv(apalightColorLoc, 1, glm::value_ptr(apalightColor));

    apadirLightEnabledLoc = glGetUniformLocation(apa.shaderProgram, "dirLightEnabled");
    glUniform1i(apadirLightEnabledLoc, dirLightEnabled);



    gridTextureLoc = glGetUniformLocation(apa.shaderProgram, "diffuseTexture");
    glm::vec2 gridSize{ GRID_NUM_POINTS_WIDTH, GRID_NUM_POINTS_HEIGHT };
    glUniform2fv(glGetUniformLocation(apa.shaderProgram, "gridSize"), 1, glm::value_ptr(gridSize));
    glm::vec2 gridDimensions{ GRID_WIDTH, GRID_HEIGHT };
    glUniform2fv(glGetUniformLocation(apa.shaderProgram, "gridDimensions"), 1, glm::value_ptr(gridDimensions));

    simTimeLoc = glGetUniformLocation(apa.shaderProgram, "time");
}
GLuint initTexture(const char* file_name) {

    int x, y, n;
    int force_channels = 4;
    unsigned char* image_data = stbi_load(file_name, &x, &y, &n, force_channels);
    if (!image_data) {
        fprintf(stderr, "ERROR: could not load %s\n", file_name);
        return false;
    }
    // NPOT check
    if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) {
        fprintf(
            stderr, "WARNING: texture %s is not power-of-2 dimensions\n", file_name
        );
    }

    int width_in_bytes = x * 4;
    unsigned char* top = NULL;
    unsigned char* bottom = NULL;
    unsigned char temp = 0;
    int half_height = y / 2;

    for (int row = 0; row < half_height; row++) {
        top = image_data + row * width_in_bytes;
        bottom = image_data + (y - row - 1) * width_in_bytes;
        for (int col = 0; col < width_in_bytes; col++) {
            temp = *top;
            *top = *bottom;
            *bottom = temp;
            top++;
            bottom++;
        }
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_SRGB, //GL_SRGB,//GL_RGBA,
        x,
        y,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        image_data
    );
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}

void renderGrid(gps::Shader shader) {

    // select active shader program
    shader.useShaderProgram();

    //update view matrix
    apaview = myCamera.getViewMatrix();
    glUniformMatrix4fv(apaviewLoc, 1, GL_FALSE, glm::value_ptr(apaview));

    // send light dir to shader
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::vec3(glm::inverseTranspose(view) * glm::vec4(lightDir, 1.0f))));

    //send grid model matrix data to shader
    glUniformMatrix4fv(apamodelLoc, 1, GL_FALSE, glm::value_ptr(apamodel));

    // compute normal matrix for grid
    apanormalMatrix = glm::mat3(glm::inverseTranspose(apaview * apamodel));

    //send grid normal matrix data to shader
    glUniformMatrix3fv(apanormalMatrixLoc, 1, GL_FALSE, glm::value_ptr(apanormalMatrix));

    //send texture to shader
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gridTexture);
    glUniform1i(gridTextureLoc, 0);

    //send sim time
    glUniform1f(simTimeLoc, simTime);

    glBindVertexArray(gridVAO);

    // draw grid
    glDrawElements(GL_TRIANGLES, (GRID_NUM_POINTS_WIDTH - 1) * (GRID_NUM_POINTS_HEIGHT - 1) * 2 * 3, GL_UNSIGNED_INT, 0);
    //glDrawArrays(GL_POINTS, 0, GRID_NUM_POINTS_WIDTH * GRID_NUM_POINTS_HEIGHT);

}

void renderAdPanel(gps::Shader shader) {
    shader.useShaderProgram();

    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw teapot
    banner.Draw(shader);
}



void renderTeapot(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw teapot
    teapot.Draw(shader);
}

void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderGrid(apa);
    renderTeapot(myBasicShader);
    renderAdPanel(myBasicShader);

    mySkyBox.Draw(skyboxShader, view, projection);

}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}



int main(int argc, const char* argv[]) {

    try {
        initOpenGLWindow();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    initOpenGLState();
    initVBOs();
    gridTexture = initTexture("textures/ucl.png");
    initModels();
    initShaders();
    initGridUniforms();
    initUniforms();

    setWindowCallbacks();
    // initSkybox();


    glCheckError();
    // application loop
    while (!glfwWindowShouldClose(myWindow.getWindow())) {


        processMovement();
        renderScene();
        if (gridAnimationActive) {
            simTime += 0.04f;
        }

        glfwPollEvents();
        glfwSwapBuffers(myWindow.getWindow());

        glCheckError();
    }

    cleanup();

    return EXIT_SUCCESS;
}
