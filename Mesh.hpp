#ifndef Mesh_hpp
#define Mesh_hpp

#include <GL/glew.h>
#include "glm/glm.hpp"

#include "Shader.hpp"

#include <string>
#include <vector>


namespace gps {

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture
{
    GLuint id;
    //ambientTexture, diffuseTexture, specularTexture
    std::string type;
    std::string path;
};

struct Material
    {
        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;
    };

struct Buffers {
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
};

class Mesh
{
public:
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;

	Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures);

	Buffers getBuffers();

	void Draw(gps::Shader shader);

private:
    /*  Render data  */
    Buffers buffers;

	// Initializes all the buffer objects/arrays
	void setupMesh();

};

}
#endif /* Mesh_hpp */
