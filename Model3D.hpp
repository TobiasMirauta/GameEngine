#ifndef Model3D_hpp
#define Model3D_hpp

#include "Mesh.hpp"

#include "tiny_obj_loader.h"
#include "stb_image.h"

#include <iostream>
#include <string>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace gps {

    class Model3D {

    public:
		Model3D() : position(0.0f), rotation(0.0f), scale(1.0f), modelMatrix(1.0f) {}
        ~Model3D();

		void LoadModel(std::string fileName);

		void LoadModel(std::string fileName, std::string basePath);

		void Draw(gps::Shader shaderProgram);

		void SetPosition(const glm::vec3& pos) {position = pos; UpdateModelMatrix();}
		void SetRotation(const glm::vec3& rot) { rotation = rot; UpdateModelMatrix(); }
		void SetScale(const glm::vec3& s) { scale = s; UpdateModelMatrix(); }

		glm::vec3 GetPosition() const { return position; }
		glm::mat4 GetModelMatrix() const { return modelMatrix; }

    private:
		// Component meshes - group of objects
        std::vector<gps::Mesh> meshes;
		// Associated textures
        std::vector<gps::Texture> loadedTextures;

		glm::vec3 position;
		glm::vec3 rotation; // in degrees
		glm::vec3 scale;
		glm::mat4 modelMatrix;

		void UpdateModelMatrix() {
			glm::mat4 mat(1.0f);
			mat = glm::translate(mat, position);
			mat = glm::rotate(mat, glm::radians(rotation.x), glm::vec3(1, 0, 0));
			mat = glm::rotate(mat, glm::radians(rotation.y), glm::vec3(0, 1, 0));
			mat = glm::rotate(mat, glm::radians(rotation.z), glm::vec3(0, 0, 1));
			mat = glm::scale(mat, scale);
			modelMatrix = mat;
		}

		// Does the parsing of the .obj file and fills in the data structure
		void ReadOBJ(std::string fileName, std::string basePath);

		// Retrieves a texture associated with the object - by its name and type
		gps::Texture LoadTexture(std::string path, std::string type);

		// Reads the pixel data from an image file and loads it into the video memory
		GLuint ReadTextureFromFile(const char* file_name);
    };
}

#endif /* Model3D_hpp */
