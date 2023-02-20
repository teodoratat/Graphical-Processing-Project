#ifndef WINDOW_H
#define WINDOW_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>

struct WindowDimensions {
    int width;
    int height;
};

namespace gps {

    class Window {

    public:
        void Create(int width=800, int height=600, const char *title="OpenGL Project");
        void Delete();

        GLFWwindow* getWindow();
        WindowDimensions getWindowDimensions();
        void setWindowDimensions(WindowDimensions dimensions);

    private:
        WindowDimensions dimensions;
        GLFWwindow *window;
    };
}

#endif //WINDOW_H
