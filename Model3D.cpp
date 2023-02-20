#include "Model3D.hpp"

namespace gps {

	void Model3D::LoadModel(std::string fileName)
	{
        std::string basePath = fileName.substr(0, fileName.find_last_of('/')) + "/";
		ReadOBJ(fileName, basePath);
	}

    void Model3D::LoadModel(std::string fileName, std::string basePath)
	{
		ReadOBJ(fileName, basePath);
	}

	// Draw each mesh from the model
	void Model3D::Draw(gps::Shader shaderProgram)
	{
		for (int i = 0; i < meshes.size(); i++)
			meshes[i].Draw(shaderProgram);
	}

	// Does the parsing of the .obj file and fills in the data structure
	void Model3D::ReadOBJ(std::string fileName, std::string basePath){

        std::cout << "Loading : " << fileName << std::endl;
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		int materialId;

		std::string err;
		bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, fileName.c_str(), basePath.c_str(), GL_TRUE);

		if (!err.empty()) { // `err` may contain warning message.
			std::cerr << err << std::endl;
		}

		if (!ret) {
			exit(1);
		}

		std::cout << "# of shapes    : " << shapes.size() << std::endl;
		std::cout << "# of materials : " << materials.size() << std::endl;

		// Loop over shapes
		for (size_t s = 0; s < shapes.size(); s++) {
			std::vector<gps::Vertex> vertices;
			std::vector<GLuint> indices;
			std::vector<gps::Texture> textures;

			// Loop over faces(polygon)
			size_t index_offset = 0;
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
				int fv = shapes[s].mesh.num_face_vertices[f];

				//gps::Texture currentTexture = LoadTexture("index1.png", "ambientTexture");
				//textures.push_back(currentTexture);

				// Loop over vertices in the face.
				for (size_t v = 0; v < fv; v++) {
					// access to vertex
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

					float vx = attrib.vertices[3 * idx.vertex_index + 0];
					float vy = attrib.vertices[3 * idx.vertex_index + 1];
					float vz = attrib.vertices[3 * idx.vertex_index + 2];
					float nx = attrib.normals[3 * idx.normal_index + 0];
					float ny = attrib.normals[3 * idx.normal_index + 1];
					float nz = attrib.normals[3 * idx.normal_index + 2];
					float tx = 0.0f;
					float ty = 0.0f;
					if (idx.texcoord_index != -1) {
						tx = attrib.texcoords[2 * idx.texcoord_index + 0];
						ty = attrib.texcoords[2 * idx.texcoord_index + 1];
					}

					glm::vec3 vertexPosition(vx, vy, vz);
					glm::vec3 vertexNormal(nx, ny, nz);
					glm::vec2 vertexTexCoords(tx, ty);

					gps::Vertex currentVertex;
					currentVertex.Position = vertexPosition;
					currentVertex.Normal = vertexNormal;
					currentVertex.TexCoords = vertexTexCoords;

					vertices.push_back(currentVertex);

					indices.push_back(index_offset + v);
				}

				index_offset += fv;
			}

			// get material id
			// Only try to read materials if the .mtl file is present
			int a = shapes[s].mesh.material_ids.size();
			if (a > 0 && materials.size()>0) {
				materialId = shapes[s].mesh.material_ids[0];
				if (materialId != -1) {
					gps::Material currentMaterial;
					currentMaterial.ambient = glm::vec3(materials[materialId].ambient[0], materials[materialId].ambient[1], materials[materialId].ambient[2]);
					currentMaterial.diffuse = glm::vec3(materials[materialId].diffuse[0], materials[materialId].diffuse[1], materials[materialId].diffuse[2]);
					currentMaterial.specular = glm::vec3(materials[materialId].specular[0], materials[materialId].specular[1], materials[materialId].specular[2]);

					//ambient texture
					std::string ambientTexturePath = materials[materialId].ambient_texname;
					if (!ambientTexturePath.empty())
					{
						gps::Texture currentTexture;
						currentTexture = LoadTexture(basePath + ambientTexturePath, "ambientTexture");
						textures.push_back(currentTexture);
					}

					//diffuse texture
					std::string diffuseTexturePath = materials[materialId].diffuse_texname;
					if (!diffuseTexturePath.empty())
					{
						gps::Texture currentTexture;
						currentTexture = LoadTexture(basePath + diffuseTexturePath, "diffuseTexture");
						textures.push_back(currentTexture);
					}

					//specular texture
					std::string specularTexturePath = materials[materialId].specular_texname;
					if (!specularTexturePath.empty())
					{
						gps::Texture currentTexture;
						currentTexture = LoadTexture(basePath + specularTexturePath, "specularTexture");
						textures.push_back(currentTexture);
					}
				}
			}

			meshes.push_back(gps::Mesh(vertices, indices, textures));
		}
	}

	// Retrieves a texture associated with the object - by its name and type
	gps::Texture Model3D::LoadTexture(std::string path, std::string type) {

			for (int i = 0; i < loadedTextures.size(); i++) {
				if (loadedTextures[i].path == path)
				{
					//already loaded texture
					return loadedTextures[i];
				}
			}

			gps::Texture currentTexture;
			currentTexture.id = ReadTextureFromFile(path.c_str());
			currentTexture.type = std::string(type);
			currentTexture.path = path;

			loadedTextures.push_back(currentTexture);

			return currentTexture;
		}

	// Reads the pixel data from an image file and loads it into the video memory
	GLuint Model3D::ReadTextureFromFile(const char* file_name) {
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
		unsigned char *top = NULL;
		unsigned char *bottom = NULL;
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

	Model3D::~Model3D() {
        for (size_t i = 0; i < loadedTextures.size(); i++) {
            glDeleteTextures(1, &loadedTextures.at(i).id);
        }

        for (size_t i = 0; i < meshes.size(); i++) {
            GLuint VBO = meshes.at(i).getBuffers().VBO;
            GLuint EBO = meshes.at(i).getBuffers().EBO;
            GLuint VAO = meshes.at(i).getBuffers().VAO;
            glDeleteBuffers(1, &VBO);
            glDeleteBuffers(1, &EBO);
            glDeleteVertexArrays(1, &VAO);
        }
	}
}
