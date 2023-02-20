#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"

#include <iostream>

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;

// camera
gps::Camera myCamera(
    glm::vec3(383.63f, 10.25f, -336.082f),
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 2.00000000000000000000000000007f;

GLboolean pressedKeys[1024];

// models
gps::Model3D myScene;
gps::Model3D millBlades;
gps::Model3D schoolBus;
gps::Model3D snowFlake;
GLfloat angle;

glm::mat4 millModel;
glm::mat4 busModel;
glm::mat4 snowModel[100];
// shaders
gps::Shader myBasicShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;

int glWindowWidth = 800;
int glWindowHeight = 600;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

GLuint shadowMapFBO;
GLuint depthMapTexture;

bool showDepthMap;

float angleY = 0.0f;
GLfloat lightAngle;

//fog terms
int foginit = 0;
float fogDensity = 0.0005f;
int fogDensityLoc;

float busX = 59.008f, busY = 3.1896f, busZ = -12.886f;
float coordBus = -20.008f;
float turnRight = false;
bool moveBus=false;
glm::vec3 pointTR;
bool snow = false;
float snowSpeed = 0.001f;
float xlim[1000], ylim[1000], zlim[1000];
int xmax, xmin, ymax, ymin, zmax, zmin;

GLenum glCheckError_(const char *file, int line)
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
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_STACK_OVERFLOW:
                error = "STACK_OVERFLOW";
                break;
            case GL_STACK_UNDERFLOW:
                error = "STACK_UNDERFLOW";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
	//TODO
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }

    if (pressedKeys[GLFW_KEY_M])
        showDepthMap = !showDepthMap;
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    glm::vec3 up(0.f, 1.f, 0.f);
    glm::vec3 cameraForward = myCamera.cameraFrontDirection;
    int width, height, centerX, centerY;
    double sens = 0.1;
    glfwGetFramebufferSize(window, &width, &height);
    centerX = width / 2;
    centerY = height / 2;
    double deltaX = xpos - centerX;
    double deltaY = ypos - centerY;
    glfwSetCursorPos(window, centerX, centerY);
    glm::mat4 matYaw = glm::mat4(1.0f);
    matYaw = glm::rotate<float>(matYaw, glm::radians(-sens * deltaX), up);
    glm::mat4 matPitch = glm::mat4(1.0f);
    matPitch = glm::rotate<float>(matYaw, glm::radians(-sens * deltaY), glm::cross<float>(cameraForward, up));
    cameraForward = matPitch * matYaw * glm::vec4(cameraForward, 0.f);
    myCamera.cameraFrontDirection = cameraForward;
}

void processMovement() {
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		//update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for myScene
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for myScene
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}


	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for myScene
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for myScene
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

    if (pressedKeys[GLFW_KEY_Q]) {
        angle += 0.01f;
        // update model matrix for myScene
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for myScene
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    }

    if (pressedKeys[GLFW_KEY_E]) {
        angle -= 0.01f;
        // update model matrix for myScene
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for myScene
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    }

    //point view
    if (pressedKeys[GLFW_KEY_1]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }

    //line view
    if (pressedKeys[GLFW_KEY_2]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    //normal view
    if (pressedKeys[GLFW_KEY_3]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    //smooth view
    if (pressedKeys[GLFW_KEY_4]) {
        glEnable(GL_MULTISAMPLE);
    }

    if (pressedKeys[GLFW_KEY_5]) {
        glDisable(GL_MULTISAMPLE);
    }
    
    //move bus front and forward
    if (pressedKeys[GLFW_KEY_UP]) {
        coordBus += 0.5f;
        busX += 0.5f;
        //busY -= 0.1f;
    }

    if (pressedKeys[GLFW_KEY_DOWN]) {
        coordBus -= 0.5f;
        busX -= 0.5f;
        //busY += 0.1f;
    }
    if (pressedKeys[GLFW_KEY_C]) {
        moveBus = true;
    }
    
    if (pressedKeys[GLFW_KEY_LEFT])
    {
        turnRight = true;
    }
    else
    {
        turnRight = false;
    }

    //start fog
    if (pressedKeys[GLFW_KEY_F]) {
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        fogDensity = 0.01f;
        glUniform1f(fogDensityLoc, fogDensity);
    }

    //stop fog
    if (pressedKeys[GLFW_KEY_G]) {
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        fogDensity = 0.0f;
        glUniform1f(fogDensityLoc, fogDensity);
    }

    // increase the intensity of fog
    if (pressedKeys[GLFW_KEY_M])
    {
        fogDensity = glm::min(fogDensity + 0.0001f, 1.0f);
    }

    // decrease the intensity of fog
    if (pressedKeys[GLFW_KEY_N])
    {
        fogDensity = glm::max(fogDensity - 0.0001f, 0.0f);
    }

    if (pressedKeys[GLFW_KEY_P]) {
        snow = true;
    }
    if(pressedKeys[GLFW_KEY_I])
        snow = false;
    if (pressedKeys[GLFW_KEY_J]) {
        lightAngle -= 1.0f;
    }

    if (pressedKeys[GLFW_KEY_L]) {
        lightAngle += 1.0f;
    }
}

void initOpenGLWindow() {

    myWindow.Create(1024, 768, "OpenGL Project Core");
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
    myScene.LoadModel("thetextures/scena1.obj");
    millBlades.LoadModel("thetextures/mill.obj");
    schoolBus.LoadModel("thetextures/schoolBus.obj");
    snowFlake.LoadModel("thetextures/snowFlakesBig.obj");
}


void initShaders() {
	myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");
}

void initUniforms() {
	myBasicShader.useShaderProgram();

    // create model matrix for myScene
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");


	// get view matrix for current camera
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	// send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for myScene
    normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

	// create projection matrix
	projection = glm::perspective(glm::radians(90.0f),
                               (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
                               0.1f, 50000.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	// send light dir to shader
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

    
    fogDensityLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogDensity");
    glUniform1f(fogDensityLoc, fogDensity);


	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	// send light color to shader
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

}

void renderMyScene(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(myCamera.getViewMatrix()));
    //send myScene model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send myScene normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw myScene
    myScene.Draw(shader);


    xmax = 324;
    xmin = 82;
    zmax = -470;
    zmin = -98;
    ymin = 70;
    ymax = 80;
    int yminl = -20;

    //float ymaxl = 40.0f;
    float ymaxl = 40.0f;
    if (snow == true) {
        for (int i = 0; i < 2; i++) {
            float x1 = rand() % xmax + xmin;
            float y1 = rand() % ymax + ymin;
            float z1 = rand() % zmax +zmin;
            glm::vec3 position = glm::vec3(x1, y1, z1);
            glm::mat4 snowF = glm::translate(position);
            glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(snowF)); //sends the current value of the millModel matrix to the "model" uniform in the vertex shader.
            normalMatrix = glm::mat3(glm::inverseTranspose(view * snowF));
            glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix)); //sends the current value of the normalMatrix to the "normalMatrix" uniform in the vertex shader.
            snowFlake.Draw(shader);

            while (y1 > yminl) {
                y1 -= snowSpeed;
                std::cout << y1 << std::endl;
                snowF = glm::translate(position);
            }
            y1 = rand() % ymax + ymin;
            snowF = glm::translate(position);
        }

    }

    model = glm::mat4(1.0f);
    //draw schoool bus

    busModel = glm::mat4(1.0f);
    glm::vec3 init = glm::vec3(135.27f, 2.7827f, -172.67f);
    if (turnRight == true) {
        glm::vec3 currentPos = glm::vec3(busX, busY, busZ);
        std::cout << busX << " "<< busY <<" " << busZ << std::endl;
        busModel = glm::translate(busModel, currentPos); //move back
        busModel = glm::rotate(busModel, 40.0f, glm::vec3(0.0f, 1.0f, 0.0f)); // applies a rotation transformation
        busModel = glm::translate(busModel, -currentPos); //move to origin
    }
    //busModel = glm::translate(glm::mat4(1.0f), init);
    while (moveBus == true && coordBus <=-33.0f) {
        coordBus += 0.3f;
        busModel = glm::translate(glm::mat4(1.0f), glm::vec3(coordBus, 0.0f, 0.0f));
        std::cout << coordBus << std::endl;
    }
     //sends the current value of the paletaModel matrix to the "model" uniform in the vertex shader.
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(busModel));
    //normalMatrix = busModel * view;
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    schoolBus.Draw(shader);


    // draw the mill blades and rotate them
    //model = glm::mat4(1.0f); //initializes the model matrix as an identity matrix
    angle += 0.01f; //increments the rotation angle variable by 0.01
    millModel = glm::mat4(1.0f); // initializes the millModel matrix as an identity matrix
    glm::vec3 centerOfMillBlade = glm::vec3(129.457f, 33.557f, -586.43f); //defines the original position of the millModel object in 3D space as a vector3
    millModel = glm::translate(millModel, centerOfMillBlade); //move back
    millModel = glm::rotate(millModel, angle, glm::vec3(1.0f, 0.0f, 0.0f)); // applies a rotation transformation
    millModel = glm::translate(millModel, -centerOfMillBlade); //move to origin
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(millModel)); //sends the current value of the millModel matrix to the "model" uniform in the vertex shader.
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix)); //sends the current value of the normalMatrix to the "normalMatrix" uniform in the vertex shader.
    millBlades.Draw(shader);
    
    // draw snow flakes
    /*xmax = 10000;
    xmin = -96;
    ymax = 40;
    ymin = -174;
    zmax = -593;
    zmin = 1000;*/
 
    //for (int i = 0; i < 2; i++)
      //snowFlake.Draw(shader);
}

void renderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//render the scene

	// render the myScene
	renderMyScene(myBasicShader);


}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
	initModels();
    //teapot = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));
	initShaders();
	initUniforms();
    setWindowCallbacks();

	glCheckError();
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
	    renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}

	cleanup();

    return EXIT_SUCCESS;
}
