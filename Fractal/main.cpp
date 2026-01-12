#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctime>
#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.hpp"
using namespace glm;

//__________________________________________________________________

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 700

int Num_Lados = 9;
int Depth = 100;
const float ratio = 0.1f;
const float Raio = 30.0f;
const float PI = 3.141592653589f;

//__________________________________________________________________

struct Color {
    float r, g, b;
    Color(float r, float g, float b) : r(r), g(g), b(b) {}
};

//__________________________________________________________________

Color goldColor(0.9f, 0.7f, 0.0f);
Color blackColor(0.0f, 0.0f, 0.0f);
Color whiteColor(1.0f, 1.0f, 1.0f);
Color redColor(1.0f, 0.0f, 0.0f);
Color greyColor(0.8f, 0.8f, 0.8f);
Color blueColor(0.0f, 0.0f, 1.0f);
Color coralColor(0.9f, 0.5f, 0.4f);
Color tealColor(0.0f, 0.4f, 0.4f);
Color brownColor(0.3f, 0.2f, 0.1f);
Color yellowColor(0.8f, 0.6f, 0.2f);
Color beigeColor(0.7f, 0.65f, 0.5f);
Color creamColor(0.9f, 0.9f, 0.8f);

GLuint VertexArrayID = 0;
GLuint vertexbuffer = 0;
GLuint colorbuffer = 0;
GLuint programID = 0;

GLFWwindow* window = nullptr;
std::vector<GLfloat> vertices;
std::vector<GLfloat> colors;

//__________________________________________________________________

void addVertex(float x, float y, const Color& color) {
    vertices.push_back(x);
    vertices.push_back(y);
    vertices.push_back(0.0f);

    colors.push_back(color.r);
    colors.push_back(color.g);
    colors.push_back(color.b);
}

void addLine(float x1, float y1, float x2, float y2, const Color& color) {
    addVertex(x1, y1, color);
    addVertex(x2, y2, color);
}

void generateInternalPattern(
    float x1, float y1,
    float x2, float y2,
    float x3, float y3,
    float x4, float y4,
    int depth) {

    if (depth <= 0) return;

    addLine(x1, y1, x2, y2, coralColor);
    addLine(x2, y2, x3, y3, yellowColor);
    addLine(x3, y3, x4, y4, creamColor);
    addLine(x4, y4, x1, y1, tealColor);

    float nx1 = x1 + ratio * (x2 - x1);
    float ny1 = y1 + ratio * (y2 - y1);

    float nx2 = x2 + ratio * (x3 - x2);
    float ny2 = y2 + ratio * (y3 - y2);

    float nx3 = x3 + ratio * (x4 - x3);
    float ny3 = y3 + ratio * (y4 - y3);

    float nx4 = x4 + ratio * (x1 - x4);
    float ny4 = y4 + ratio * (y1 - y4);

    generateInternalPattern(nx1, ny1, nx2, ny2, nx3, ny3, nx4, ny4, depth - 1);
}

void generateFractal(int numSides, int depth) {
    for (int i = 0; i < numSides; i++) {
        float first_angle = 2.0f * PI * i / numSides;
        float second_angle = 2.0f * PI * (i + 1) / numSides;
        float third_angle = 2.0f * PI * (i + 2) / numSides;

        float x1 = Raio * cos(first_angle);
        float y1 = Raio * sin(first_angle);
        float x2 = Raio * cos(second_angle);
        float y2 = Raio * sin(second_angle);
        float x3 = Raio * cos(third_angle);
        float y3 = Raio * sin(third_angle);

        float midpointX1 = (x1 + x2) / 2.0f;
        float midpointY1 = (y1 + y2) / 2.0f;
        float midpointX2 = (x2 + x3) / 2.0f;
        float midpointY2 = (y2 + y3) / 2.0f;

        addLine(x1, y1, x2, y2, goldColor);

        generateInternalPattern(midpointX1, midpointY1,
            x2, y2,
            midpointX2, midpointY2,
            0.0f, 0.0f,
            depth);
    }
}

//__________________________________________________________________

void transferDataToGPUMemory(int numSides, int depth) {
    vertices.clear();
    colors.clear();

    generateFractal(numSides, depth);

    // create VAO
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // create & upload vertex buffer
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    // create & upload color buffer
    glGenBuffers(1, &colorbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(GLfloat), colors.data(), GL_STATIC_DRAW);
}

void cleanupDataFromGPU() {
    if (vertexbuffer) glDeleteBuffers(1, &vertexbuffer);
    if (colorbuffer) glDeleteBuffers(1, &colorbuffer);
    if (VertexArrayID) glDeleteVertexArrays(1, &VertexArrayID);
    if (programID) glDeleteProgram(programID);
}

void draw(void) {
    glUseProgram(programID);

    glm::mat4 mvp = glm::ortho(-40.0f, 40.0f, -40.0f, 40.0f);
    int matrixLoc = glGetUniformLocation(programID, "mvp");
    if (matrixLoc >= 0) {
        glUniformMatrix4fv(matrixLoc, 1, GL_FALSE, glm::value_ptr(mvp));
    }

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // Draw lines
    glLineWidth(1.5f);
    glDrawArrays(GL_LINES, 0, (GLsizei)(vertices.size() / 3));

    // Draw points (optional)
    glPointSize(1.5f);
    glDrawArrays(GL_POINTS, 0, (GLsizei)(vertices.size() / 3));

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

void framebuffer_size_callback(GLFWwindow* win, int width, int height) {
    (void)win;
    if (width > 0 && height > 0) {
        glViewport(0, 0, width, height);
    }
}

int main(void) {
    if (!glfwInit()) {
        std::cerr << "glfwInit failed\n";
        return -1;
    }

    int numSides = Num_Lados;
    int depth = Depth;

    std::cout << "Enter the number of sides for the shape: ";
    if (!(std::cin >> numSides)) {
        std::cerr << "Invalid input for number of sides. Using default: " << Num_Lados << "\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        numSides = Num_Lados;
    }
    else {
        std::cout << "Generating shape with " << numSides << " sides.\n";
    }

    std::cout << "Enter the number for the depth of the fractal (positive integer, default 100): ";
    if (!(std::cin >> depth)) {
        std::cerr << "Invalid input for depth. Using default: " << Depth << "\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        depth = Depth;
    }

    if (depth <= 0) {
        std::cout << "Invalid depth. The depth must be a positive integer. Using default: " << Depth << std::endl;
        depth = Depth;
    }
    if (numSides < 3) {
        std::cout << "Invalid number of sides. The shape must have at least 3 sides. Using default: " << Num_Lados << std::endl;
        numSides = Num_Lados;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Shape with Internal Patterns", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to open GLFW window.\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        glfwTerminate();
        return -1;
    }

    int fbw, fbh;
    glfwGetFramebufferSize(window, &fbw, &fbh);
    glViewport(0, 0, fbw, fbh);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // background color

    // Load shaders AFTER context is valid
    programID = LoadShaders(
        "SimpleVertexShader.vertexshader",
        "SimpleFragmentShader.fragmentshader");

    transferDataToGPUMemory(numSides, depth);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        draw();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cleanupDataFromGPU();
    glfwTerminate();
    return 0;
}
