//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#define OLC_PGE_APPLICATION

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <cstdlib>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"
#include "olcPixelGameEngine.h"
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "Smoke.hpp"

#include "LensFlare.hpp"

#include <iostream>

int glWindowWidth = 800;
int glWindowHeight = 600;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const unsigned int SHADOW_WIDTH = 4096;
const unsigned int SHADOW_HEIGHT = 4096;

float movementSpeed = 5.0f; 
float animatedAngle = 0.0f;
float animatedPositionX = 0.0f;

double lastTimeStamp = 0.0;


LensFlare* lensFlareEffect;


GLuint lightTextureID;
GLuint christmasTextureID;
GLuint musicID;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;
float currentTime = 0.0f;
glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

gps::Camera myCamera(
	glm::vec3(129.9f, 14.5f, 480.0f),
	glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 1.0f;

bool pressedKeys[1024];
float angleY = 0.05f;
GLfloat lightAngle;

gps::Model3D gilneasBigHouse;
gps::Model3D statue;
gps::Model3D house1;
gps::Model3D house2;
gps::Model3D house3;
gps::Model3D lightCube;
gps::Model3D screenQuad;
gps::Model3D watchTower;
gps::Model3D simpleTower;
gps::Model3D inn;
gps::Model3D catedrala;
gps::Model3D farmHouse;
gps::Model3D blacksmith;
gps::Model3D stoneHouse;
gps::Model3D lightHouse;
gps::Model3D day;
gps::Model3D lightCord;
gps::Model3D lightBulbs;
gps::Model3D lightGlass;
gps::Model3D lightFilament;
gps::Model3D lightPole;
gps::Model3D lightBulbsBig;
gps::Model3D lightGlassPole;
gps::Model3D tavernMusic;
gps::Model3D choirMusic;
gps::Model3D happyMusic;



gps::Model3D walls;
gps::Model3D snow;

gps::Model3D scene;
gps::Model3D moon;
gps::Model3D sun;


gps::SkyBox skybox;
gps::SkyBox skyBoxDay;



gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;
gps::Shader skyboxShader;
gps::Shader smokeShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;

bool showDepthMap;

struct SnowVertex {
	float x, y;     // Position
	float r, g, b;  // Color
};

std::vector<Smoke*> smokeList;

ma_engine engine;
ma_sound_group musicGroup;

ma_sound happyM;
ma_sound tavernM;
ma_sound choirM;

class Snow {
private:
	int grid_width;
	int grid_height;
	std::vector<int> flakes; 

	// Modern OpenGL handles
	GLuint VAO, VBO, shaderProgram;

	std::vector<SnowVertex> renderBuffer;

	float animate_elapsed = 0.0f;
	float animate_rate = 0.08f;

public:
	// Destructor to clean up GPU memory
	~Snow() {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteProgram(shaderProgram);
	}

	void Init(int width = 256, int height = 240) {
		grid_width = width;
		grid_height = height;
		flakes.resize(grid_width * grid_height, 0);

		// 1. Setup Buffers (VAO & VBO)
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		// Initialize VBO with null data, dynamic draw (since we update it every frame)
		// We allocate enough space for potentially filling the whole screen (safety)
		glBufferData(GL_ARRAY_BUFFER, sizeof(SnowVertex) * grid_width * grid_height, NULL, GL_DYNAMIC_DRAW);

		// Attribute 0: Position (2 floats)
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(SnowVertex), (void*)0);
		glEnableVertexAttribArray(0);

		// Attribute 1: Color (3 floats)
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SnowVertex), (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);

		// 2. compile Shaders
		InitShaders();
	}

	void Update(float deltaTime) {
		animate_elapsed += deltaTime;
		if (animate_elapsed >= animate_rate) {
			animate_elapsed -= animate_rate;
			SpawnSnow(2);
			MoveSnow();
		}
	}

	void Render(int windowWidth, int windowHeight) {
		// --- 1. BUILD GEOMETRY ON CPU ---
		renderBuffer.clear();

		// Calculate scaling factor
		float scaleX = (float)windowWidth / grid_width;
		float scaleY = (float)windowHeight / grid_height;

		for (int y = 0; y < grid_height; y++) {
			for (int x = 0; x < grid_width; x++) {
				int colorVal = flakes[y * grid_width + x];
				if (colorVal > 0) {
					float color = colorVal / 255.0f;

					// Map grid coordinate to Screen Coordinate
					float screenX = x * scaleX;
					float screenY = y * scaleY;

					renderBuffer.push_back({ screenX, screenY, color, color, color });
				}
			}
		}

		if (renderBuffer.empty()) return;

		// --- 2. UPLOAD TO GPU ---
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		// glBufferSubData replaces data in the buffer without reallocating
		glBufferSubData(GL_ARRAY_BUFFER, 0, renderBuffer.size() * sizeof(SnowVertex), renderBuffer.data());


		// --- 3. DRAW ---
		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);

		// Pass Projection Matrix to Shader
		// Note: Coordinates are Top-Left (0,0) to Bottom-Right (width, height)
		glm::mat4 projection = glm::ortho(0.0f, (float)windowWidth, (float)windowHeight, 0.0f, -1.0f, 1.0f);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		// Calculate Point Size based on scale
		float pSize = scaleX > scaleY ? scaleY : scaleX;
		if (pSize < 1.0f) pSize = 1.0f;

		// Send point size to shader (optional if using glPointSize, but safer for Core)
		glUniform1f(glGetUniformLocation(shaderProgram, "pointSize"), pSize);

		// Enable writing to point size in shader (for some drivers)
		glEnable(GL_PROGRAM_POINT_SIZE);

		glDrawArrays(GL_POINTS, 0, (GLsizei)renderBuffer.size());

		glBindVertexArray(0);
		glUseProgram(0);
	}

private:
	void InitShaders() {
		// Simple Internal Shaders
		const char* vertexSrc = R"(
            #version 410 core
            layout (location = 0) in vec2 aPos;
            layout (location = 1) in vec3 aColor;
            
            out vec3 fragColor;
            
            uniform mat4 projection;
            uniform float pointSize;

            void main() {
                gl_Position = projection * vec4(aPos, 0.0, 1.0);
                fragColor = aColor;
                gl_PointSize = pointSize;
            }
        )";

		const char* fragmentSrc = R"(
            #version 410 core
            in vec3 fragColor;
            out vec4 finalColor;
            void main() {
                finalColor = vec4(fragColor, 1.0);
            }
        )";

		GLuint vs = CompileShader(vertexSrc, GL_VERTEX_SHADER);
		GLuint fs = CompileShader(fragmentSrc, GL_FRAGMENT_SHADER);

		shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vs);
		glAttachShader(shaderProgram, fs);
		glLinkProgram(shaderProgram);

		glDeleteShader(vs);
		glDeleteShader(fs);
	}

	GLuint CompileShader(const char* src, GLenum type) {
		GLuint shader = glCreateShader(type);
		glShaderSource(shader, 1, &src, NULL);
		glCompileShader(shader);
		// (Error checking omitted for brevity, but recommended)
		return shader;
	}

	// --- GAME LOGIC (Unchanged) ---
	void SpawnSnow(int number_of_flakes) {
		for (int i = 0; i < number_of_flakes; i++) {
			int x = rand() % grid_width;
			int y = 0;
			flakes[y * grid_width + x] = 126 + rand() % 128;
		}
	}

	void MoveSnow() {

		for (int x = 0; x < grid_width; x++) {
			flakes[(grid_height - 1) * grid_width + x] = 0;
		}

		// Iterate bottom-up (starting from second-to-last row)
		for (int y = grid_height - 2; y >= 0; y--) {
			for (int x = 0; x < grid_width; x++) {
				int currentIdx = y * grid_width + x;
				int val = flakes[currentIdx];

				if (val != 0) {
					int direction = (rand() % 3) - 1;
					int newX = x + direction;

					if (newX >= 0 && newX < grid_width) {
						int targetIdx = (y + 1) * grid_width + newX;

						// Because we clear the bottom row every frame, 
						// snow will always find space to fall into "the void"
						if (flakes[targetIdx] == 0) {
							flakes[targetIdx] = val;
							flakes[currentIdx] = 0;
						}
					}
				}
			}
		}
	}
};

GLenum glCheckError_(const char* file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);

	glWindowWidth = width;
	glWindowHeight = height;

	glfwGetFramebufferSize(window, &retina_width, &retina_height);

	glViewport(0, 0, retina_width, retina_height);

	if (retina_height == 0) retina_height = 1; 
	float aspectRatio = (float)retina_width / (float)retina_height;

	projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 3000.0f);



	myCustomShader.useShaderProgram();
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	skyboxShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
	//solid
	if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	//wireframe
	if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	//poligonal
	if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	
}

static double lastMouseX = 0.0;
static double lastMouseY = 0.0;
static bool firstMouseFrame = true;

void processMovement()
{

	if (pressedKeys[GLFW_KEY_F]) {
		angleY -= 1.0f;
	}
	if (pressedKeys[GLFW_KEY_Q]) {
		angleY -= 1.0f;
		lightAngle -= 1.0f;
	}

	if (pressedKeys[GLFW_KEY_E]) {
		angleY += 1.0f;
		lightAngle += 1.0f;
	}

	if (pressedKeys[GLFW_KEY_J]) {
		lightAngle -= 1.0f;
	}

	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle += 1.0f;
	}

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

}

// Helper function to load a texture and get its ID
GLuint loadTexture(const char* path) {
	GLuint textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);

	if (data) {
		GLenum format;
		if (nrComponents == 1) format = GL_RED;
		else if (nrComponents == 3) format = GL_RGB;
		else if (nrComponents == 4) format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// Parameters to make it look nice
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else {
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID; // <--- THIS IS THE ID YOU NEED
}


bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);


	//window scaling for HiDPI displays
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

	//for sRBG framebuffer
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

	//for antialising
	glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	//glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

#if not defined (__APPLE__)
	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();
#endif

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initAudio() {
	ma_result result;

	// Initialize the engine
	result = ma_engine_init(NULL, &engine);
	if (result != MA_SUCCESS) {
		printf("Failed to initialize audio engine.\n");
		return;
	}
}

void initObjects() {
	//nanosuit.LoadModel("models/scena/statue2.obj");
	moon.LoadModel("models/moon/moon.obj");
	sun.LoadModel("models/sun/sun.obj");
	gilneasBigHouse.LoadModel("models/gilneas-market-quarter-house-01/gilneas_house_2.obj");
	house1.LoadModel("models/stylized-town-tavern/center_house.obj");
	house2.LoadModel("models/gilneas-detached-house/gilneas_house1.obj");
	watchTower.LoadModel("models/medieval-watchtower-house/medievalWatchTower.obj");
	inn.LoadModel("models/gobble-inn/gobbleInn.obj");
	catedrala.LoadModel("models/stormwind-cathedral-quarter-house-01/catedrala.obj");
	simpleTower.LoadModel("models/medieval-tower/medievalTower.obj");
	walls.LoadModel("models/walls1/walls.obj");
	snow.LoadModel("models/snow/snow.obj");
	farmHouse.LoadModel("models/farmHouse/farmHouse.obj");
	blacksmith.LoadModel("models/blacksmith/blacksmith.obj");
	stoneHouse.LoadModel("models/fantasy-stone-manson/stoneHouse.obj");
	statue.LoadModel("models/statue/statue.obj");
	lightHouse.LoadModel("models/lightHouse/lightHouse.obj");
	house3.LoadModel("models/adventurers-house/advHouse.obj");
	//ground.LoadModel("models/ground/ground.obj");
	//lightCube.LoadModel("models/cube/cube.obj");
	screenQuad.LoadModel("models/quad/quad.obj");
	lightCord.LoadModel("models/lights/cord/lights_cord.obj");
	lightBulbs.LoadModel("models/lights/bulb/lights_bulb.obj");
	lightGlass.LoadModel("models/lights/glass/lights_glass.obj");
	lightFilament.LoadModel("models/lights/filament/lights_filament.obj");
	lightTextureID = loadTexture("models/lights/bulb/Lightcolor.png");
	christmasTextureID = loadTexture("models/lights/filament/colors.png");
	day.LoadModel("models/skyboxDay/day.obj");
	lightPole.LoadModel("models/vintage-streetlamp/streetLamp.obj");
	lightBulbsBig.LoadModel("models/vintage-streetlamp/streetBulbs.obj");
	lightGlassPole.LoadModel("models/vintage-streetlamp/streetGlass.obj");
	tavernMusic.LoadModel("music/tavern_music/tavern.obj");
	choirMusic.LoadModel("music/choir_music/choir.obj");
	happyMusic.LoadModel("music/happy_music/happy.obj");
	//blackSmith.LoadModel("models/blacksmith/blacksmith.obj");
	//arthas.LoadModel("models/arthas/arthas.obj");
	//scene.LoadModel("models/auxMainScene/mainScene.obj");s
	//skybox.LoadModel("models/skyboxDay/skybox.obj");
	
}

void initLensFlare() {
	lensFlareEffect = new LensFlare();


	lensFlareEffect->AddTexture("models/lens_flare/tex1.png");
	lensFlareEffect->AddTexture("models/lens_flare/tex2.png");
	lensFlareEffect->AddTexture("models/lens_flare/tex3.png");
	lensFlareEffect->AddTexture("models/lens_flare/tex4.png");
	lensFlareEffect->AddTexture("models/lens_flare/tex5.png");
	lensFlareEffect->AddTexture("models/lens_flare/tex6.png");
	lensFlareEffect->AddTexture("models/lens_flare/tex7.png");
	lensFlareEffect->AddTexture("models/lens_flare/tex8.png");
}

void initSkyBox() {
	std::vector<const GLchar*> faces;
	faces.push_back("models/skybox_night/px.png");
	faces.push_back("models/skybox_night/nx.png");
	faces.push_back("models/skybox_night/py.png");
	faces.push_back("models/skybox_night/ny.png");
	faces.push_back("models/skybox_night/nz.png");
	faces.push_back("models/skybox_night/pz.png");
	skybox.Load(faces);
}

void initSkyBoxDay() {
	std::vector<const GLchar*> faces;
	faces.push_back("models/skybox-skydays-3/px.png");
	faces.push_back("models/skybox-skydays-3/nx.png");
	faces.push_back("models/skybox-skydays-3/py.png");
	faces.push_back("models/skybox-skydays-3/ny.png");
	faces.push_back("models/skybox-skydays-3/nz.png");
	faces.push_back("models/skybox-skydays-3/pz.png");
	skyBoxDay.Load(faces);
}

void initSmoke() {
	smokeShader.loadShader("shaders/smoke.vert", "shaders/smoke.frag");
	smokeShader.useShaderProgram();
	glm::vec3 leftHorn = glm::vec3(59.36f, 12.32f, 116.3f);
	glm::vec3 rightHorn = glm::vec3(58.36f, 29.6f, 116.3f);
	glm::vec3 horn3 = glm::vec3(196.0f, 27.49f, -2.097f);
	glm::vec3 horn4 = glm::vec3(58.62f, 29.16f, 116.3f);
	glm::vec3 horn5 = glm::vec3(-12.71f, 28.59f, 110.1f);
	glm::vec3 horn6 = glm::vec3(255.4f, 26.39f, 4.696f);
	glm::vec3 horn7 = glm::vec3(241.4f, 51.96f, 111.4f);
	glm::vec3 horn8 = glm::vec3(54.71f, 18.97f, -110.8f);
	glm::vec3 horn9 = glm::vec3(-41.35f, 26.8f, -122.9f);
	glm::vec3 horn10 = glm::vec3(45.22f, 29.99f, -245.5f);
	glm::vec3 horn11 = glm::vec3(78.84f, 52.9f, -185.4f);
	glm::vec3 horn12 = glm::vec3(188.7f, 54.04f, -163.2f);
	glm::vec3 horn13 = glm::vec3(295.7f, 27.34f, -213.3f);
	glm::vec3 horn14 = glm::vec3(249.4f, 27.99f, -74.52f);
	glm::vec3 horn15 = glm::vec3(73.76f, 47.03f, -156.6f);
	glm::vec3 horn16 = glm::vec3(78.93f, 49.1f, -169.4f);






	smokeList.push_back(new Smoke(leftHorn, 40));
	smokeList.push_back(new Smoke(rightHorn, 40));
	smokeList.push_back(new Smoke(horn3, 40));
	smokeList.push_back(new Smoke(horn4, 40));
	smokeList.push_back(new Smoke(horn5, 40));
	smokeList.push_back(new Smoke(horn6, 40));
	smokeList.push_back(new Smoke(horn7, 40));
	smokeList.push_back(new Smoke(horn8, 40));
	smokeList.push_back(new Smoke(horn8, 40));
	smokeList.push_back(new Smoke(horn9, 40));
	smokeList.push_back(new Smoke(horn10, 40));
	smokeList.push_back(new Smoke(horn11, 40));
	smokeList.push_back(new Smoke(horn12, 40));
	smokeList.push_back(new Smoke(horn13, 40));
	smokeList.push_back(new Smoke(horn14, 40));
	smokeList.push_back(new Smoke(horn15, 40));
	smokeList.push_back(new Smoke(horn16, 40));
	//smokeList.push_back(new Smoke(horn5, 40));
	//smokeList.push_back(new Smoke(horn5, 40));



}

void initShaders() {
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	lightShader.useShaderProgram();
	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();
	depthMapShader.loadShader("shaders/depthMapShader.vert", "shaders/depthMapShader.frag");
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();


}



void initUniforms() {
	myCustomShader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 3000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = moon.GetPosition();
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(1.0f, 0.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glUniform1f(glGetUniformLocation(lightShader.shaderProgram, "alphaMultiplier"), 2.0f);
}

void initFBO() {
	//TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
	glGenFramebuffers(1, &shadowMapFBO);
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


}

glm::mat4 computeLightSpaceTrMatrix(const glm::vec3 &lightPosWorld) {
	//TODO - Return the light-space transformation matrix
	glm::vec3 upvector = glm::vec3(0.0f, 1.0f, 0.0f);
	if (abs(lightPosWorld.x) < 0.1f && abs(lightPosWorld.z) < 0.1f) {
		upvector - glm::vec3(0.0f, 0.0f, 1.0f);
	}
	glm::mat4 lightView = glm::lookAt(lightPosWorld, glm::vec3(0.0f), upvector);
	const GLfloat near_plane = 0.1f, far_plane = 500.0f;
	glm::mat4 lightProjection = glm::ortho(-200.0f, 200.0f, -200.0f, 200.0f, near_plane, far_plane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

	return lightSpaceTrMatrix;
}

void sendPointLights(gps::Shader shader) {
	shader.useShaderProgram();

	struct LightData {
		glm::vec3 pos;
		glm::vec3 color;
	};

	std::vector<LightData> allLights;
	glm::vec3 lampColor = glm::vec3(10.0f, 6.0f, 3.0f);
	glm::vec3 windowColor = glm::vec3(10.0f, 6.0f, 3.0f);
	
	allLights.push_back({ glm::vec3(106.9f, 29.35f, 19.66f), lampColor });
	allLights.push_back({ glm::vec3(106.9f, 29.35f, 79.4f), lampColor });
	allLights.push_back({ glm::vec3(99.28f, 29.44f, -86.17f), lampColor });
	allLights.push_back({ glm::vec3(167.5f,  28.69f, -86.58f), lampColor });
	allLights.push_back({ glm::vec3(183.7f,  29.35f, 32.62f), lampColor });
	allLights.push_back({ glm::vec3(180.0f, 29.35f, 87.96f), lampColor });
	
	allLights.push_back({ glm::vec3(194.3f, 16.42f, 9.997f), windowColor });
	allLights.push_back({ glm::vec3(181.9f, 12.31f, 6.903f), windowColor });
	allLights.push_back({ glm::vec3(58.8f, 11.67f, 105.2f), windowColor });
	allLights.push_back({ glm::vec3(20.55f, 21.33f, 57.74f), windowColor });
	allLights.push_back({ glm::vec3(8.762f, 20.88f, 68.95f), windowColor });
	allLights.push_back({ glm::vec3(183.4f, 10.39f, -113.3f), windowColor });
	allLights.push_back({ glm::vec3(182.8f, 19.87f, -109.5f), windowColor });
	allLights.push_back({ glm::vec3(137.8f, 23.89f, -228.7f), windowColor });
	allLights.push_back({ glm::vec3(83.16f, 8.516f, -98.37f), windowColor });
	allLights.push_back({ glm::vec3(271.0f, 13.52f, -165.9f), windowColor });
	for (int i = 0; i < 16; i++) {
		std::string number = std::to_string(i);

		// If we have data for this light, use it. If not, send a "Black" light (OFF)
		if (i < allLights.size()) {
			glm::vec3 posView = glm::vec3(view * glm::vec4(allLights[i].pos, 1.0f));
			glUniform3fv(glGetUniformLocation(shader.shaderProgram, ("pointLights[" + number + "].position").c_str()), 1, glm::value_ptr(posView));
			glUniform3fv(glGetUniformLocation(shader.shaderProgram, ("pointLights[" + number + "].color").c_str()), 1, glm::value_ptr(allLights[i].color));

			// Physics for Windows (shorter range than street lamps)
			glUniform1f(glGetUniformLocation(shader.shaderProgram, ("pointLights[" + number + "].constant").c_str()), 1.0f);
			glUniform1f(glGetUniformLocation(shader.shaderProgram, ("pointLights[" + number + "].linear").c_str()), 0.14f);
			glUniform1f(glGetUniformLocation(shader.shaderProgram, ("pointLights[" + number + "].quadratic").c_str()), 0.07f);
		}
		else {
			// Turn off unused lights
			glUniform3fv(glGetUniformLocation(shader.shaderProgram, ("pointLights[" + number + "].color").c_str()), 1, glm::value_ptr(glm::vec3(0.0f)));
		}
	}
}


void drawObjects(gps::Shader shader, bool depthPass) {

	// 1. DRAW MAIN SCENE
	shader.useShaderProgram();

	// Standard Scene Rotation
	model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	// -- Draw Houses, Ground, etc. --

	gilneasBigHouse.Draw(shader);
	house1.Draw(shader);
	statue.Draw(shader);
	house2.Draw(shader);
	house3.Draw(shader);
	inn.Draw(shader);
	watchTower.Draw(shader);
	simpleTower.Draw(shader);
	catedrala.Draw(shader);
	farmHouse.Draw(shader);
	blacksmith.Draw(shader);
	stoneHouse.Draw(shader);
	lightHouse.Draw(shader);
	snow.Draw(shader);
	happyMusic.Draw(shader);
	tavernMusic.Draw(shader);
	choirMusic.Draw(shader);
	walls.Draw(shader);
	// =========================================================
	// STREET LAMPS (Static Lights)
	// =========================================================

	// 1. Poziționarea Lămpii
	// (Dacă le-ai pus în Blender, lasă matricea Identity, altfel pune translate/scale)
	glm::mat4 lampModel = glm::mat4(1.0f);
	// lampModel = glm::translate(lampModel, ...); // Dacă e nevoie

	// -----------------------------------------------------------
	// PASUL 1: DESENEAZĂ CORPUL LĂMPII (Metal + Sticla Mată)
	// -----------------------------------------------------------
	myCustomShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(lampModel));

	// Matricea normală pentru lumină pe metal
	if (!depthPass) {
		moon.Draw(shader);
		sun.Draw(shader);
		glm::mat3 lampNormalMatrix = glm::mat3(glm::inverseTranspose(view * lampModel));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(lampNormalMatrix));
	}

	// Dacă ai o textură pentru lampă (metal ruginit etc), o legi aici la GL_TEXTURE0
	// glActiveTexture(GL_TEXTURE0); ...

	lightPole.Draw(myCustomShader);


	// -----------------------------------------------------------
	// PASUL 2 & 3: BECUL ȘI AURA (Folosim lightShader)
	// -----------------------------------------------------------
	lightShader.useShaderProgram();

	// Setări standard shader
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(lampModel));
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	// IMPORTANT: Trimitem Timpul (pentru ca becul să pâlpâie ușor, ca un bec vechi de stradă)
	float currentTime = glfwGetTime();
	glUniform1f(glGetUniformLocation(lightShader.shaderProgram, "time"), currentTime);

	// TEXTURA CULOARE:
	// Folosește aceeași textură de "Christmas Lights" dacă ai galben pe ea,
	// SAU leagă o textură mică portocalie/galbenă pentru efectul de Sodium/Vapori.
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, lightTextureID);
	glUniform1i(glGetUniformLocation(lightShader.shaderProgram, "diffuseTexture"), 1);


	// --- A. BECUL INTERIOR (Sursa mică și fierbinte) ---
	// Chiar dacă sticla e mată, asta va da un punct alb în centru dacă te uiți atent.

	// Alpha 15.0 = Alb Nuclear
	glUniform1f(glGetUniformLocation(lightShader.shaderProgram, "alphaMultiplier"), 15.0f);

	lightBulbsBig.Draw(lightShader);


	// --- B. AURA (Efectul de lumină prin ceață/sticlă mată) ---
	// Asta rezolvă problema vizibilității! Desenăm un "nor" de lumină în jurul lămpii.

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive Blending (Strălucire)
	glDepthMask(GL_FALSE); // Nu blochează alte obiecte


	glUniform1f(glGetUniformLocation(lightShader.shaderProgram, "alphaMultiplier"), 0.3f);

	lightGlassPole.Draw(lightShader);

	glDisable(GL_CULL_FACE);

	// Mărim sfera becului de 6 ori (sau cât e nevoie să fie mai mare decât sticla lămpii)
	glm::mat4 glowModel = glm::scale(lampModel, glm::vec3(6.0f, 6.0f, 6.0f));
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(glowModel));

	// Intensitate medie (0.8) - să arate ca o ceață luminoasă gălbuie
	glUniform1f(glGetUniformLocation(lightShader.shaderProgram, "alphaMultiplier"), 0.8f);

	lightBulbsBig.Draw(lightShader); // Desenăm din nou becul, dar URIAȘ


	// -----------------------------------------------------------
	// CLEANUP
	// -----------------------------------------------------------
	glUniform1f(glGetUniformLocation(lightShader.shaderProgram, "alphaMultiplier"), 1.0f);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	// =========================================================
	// CHRISTMAS LIGHTS: THE "CARTOON GLOW" EDITION
	// =========================================================

	// Base Transform (Position & Base Scale)
	glm::mat4 lightsModel = glm::mat4(1.0f);
	lightsModel = glm::translate(lightsModel, glm::vec3(10.0f, 5.0f, -2.0f));
	lightsModel = glm::scale(lightsModel, glm::vec3(50.0f, 50.0f, 50.0f));

	// -----------------------------------------------------------
	// PART A: CORD & SOCKETS (Standard Opaque)
	// -----------------------------------------------------------
	myCustomShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(lightsModel));

	if (!depthPass) {
		glm::mat3 lightsNormalMatrix = glm::mat3(glm::inverseTranspose(view * lightsModel));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(lightsNormalMatrix));
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, lightTextureID); // Or cordTextureID
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "diffuseTexture"), 0);

	lightCord.Draw(myCustomShader);


	// -----------------------------------------------------------
	// SETUP LIGHT SHADER (For Filament, Glass, and Aura)
	// -----------------------------------------------------------
	lightShader.useShaderProgram();

	// Basic Uniforms
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(lightsModel));
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	// Twinkling Time
	float currentTimeLight = glfwGetTime();
	glUniform1f(glGetUniformLocation(lightShader.shaderProgram, "time"), currentTimeLight);

	// Bind Color Texture
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, lightTextureID);
	glUniform1i(glGetUniformLocation(lightShader.shaderProgram, "diffuseTexture"), 1);


	// -----------------------------------------------------------
	// PART B: FILAMENT (The Tiny White Source)
	// -----------------------------------------------------------
	// Alpha 20.0 = Pure White Hot
	glUniform1f(glGetUniformLocation(lightShader.shaderProgram, "alphaMultiplier"), 20.0f);
	lightFilament.Draw(lightShader);


	// -----------------------------------------------------------
	// PART C: THE INNER BULB (Solid Colored Glass)
	// -----------------------------------------------------------
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive
	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);

	// Alpha 2.0 = Bright and Solid
	glUniform1f(glGetUniformLocation(lightShader.shaderProgram, "alphaMultiplier"), 2.0f);
	lightGlass.Draw(lightShader);


	// -----------------------------------------------------------
	// PART D: THE CARTOON AURA (Massive Glow Cloud)
	// -----------------------------------------------------------

	glm::mat4 auraModel = lightsModel;

	// SCALE THIS UP! 
	// 8.0f means the glow is 8 times bigger than the physical bulb.
	// If you want it even crazier, change 8.0f to 12.0f or 15.0f.
	auraModel = glm::scale(auraModel, glm::vec3(8.0f, 8.0f, 8.0f));

	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(auraModel));

	// Alpha 0.4: We keep this lower so it looks like a soft, magical fog.
	// Because they overlap, they will naturally get brighter where lights are clustered.
	glUniform1f(glGetUniformLocation(lightShader.shaderProgram, "alphaMultiplier"), 0.4f);

	lightGlass.Draw(lightShader);


	// -----------------------------------------------------------
	// CLEANUP
	// -----------------------------------------------------------
	// Reset uniforms
	glUniform1f(glGetUniformLocation(lightShader.shaderProgram, "alphaMultiplier"), 1.0f);

	// Restore OpenGL state
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
void renderScene() {

	// =========================================================
	// 1. UPDATE POSITIONS (Orbit Logic)
	// =========================================================
	float orbitRadius = 800.0f;
	glm::vec3 orbitCenter(0.0f, 0.0f, 0.0f);
	float angleRad = glm::radians(lightAngle);

	// MOON Position (Positive Sin/Cos)
	glm::vec3 moonPos;
	moonPos.x = orbitCenter.x + orbitRadius * sin(angleRad);
	moonPos.y = orbitCenter.y + orbitRadius * cos(angleRad);
	moonPos.z = orbitCenter.z + orbitRadius * cos(angleRad);

	// SUN Position (Negative - Opposite side)
	glm::vec3 sunPos;
	sunPos.x = orbitCenter.x - orbitRadius * sin(angleRad);
	sunPos.y = orbitCenter.y - orbitRadius * cos(angleRad);
	sunPos.z = orbitCenter.z - orbitRadius * cos(angleRad);

	moon.SetPosition(moonPos);
	sun.SetPosition(sunPos);


	// =========================================================
	// 2. DAY / NIGHT CYCLE LOGIC
	// =========================================================
	// Decide who casts the shadows and what color the light is
	glm::vec3 lightPosForShadows;
	glm::vec3 currentLightColor;

	// If Sun is above horizon (-50.0f buffer allows for twilight shadows)
	if (sunPos.y > -50.0f) {
		lightPosForShadows = sunPos;
		currentLightColor = glm::vec3(1.0f, 1.0f, 0.9f); // Warm Sun White
	}
	else {
		lightPosForShadows = moonPos;
		currentLightColor = glm::vec3(0.1f, 0.1f, 0.3f); // Cold Blue Night
	}


	// =========================================================
	// 3. SHADOW MAPPING PASS
	// =========================================================
	glm::mat4 lightSpace = computeLightSpaceTrMatrix(lightPosForShadows);

	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpace));

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	// Draw scene for shadow map (BUT SKIP SUN/MOON inside drawObjects)
	glCullFace(GL_FRONT);
	drawObjects(depthMapShader, true);
	glCullFace(GL_BACK);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	// =========================================================
	// 4. MAIN SCENE RENDERING
	// =========================================================

	// DEBUG: Show Depth Map (Press M key)
	if (showDepthMap) {
		glViewport(0, 0, retina_width, retina_height);
		glClear(GL_COLOR_BUFFER_BIT);
		screenQuadShader.useShaderProgram();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);
		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);
		return;
	}

	// --- NORMAL RENDER START ---
	glViewport(0, 0, retina_width, retina_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	myCustomShader.useShaderProgram();

	// 1. Update View Matrix
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	// 2. Send Directional Light (Sun/Moon)
	glm::vec3 lightDirGlobal = glm::normalize(lightPosForShadows);
	glm::vec3 lightDirViewSpace = glm::vec3(view * glm::vec4(lightDirGlobal, 0.0f));
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirViewSpace));
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(currentLightColor));

	// 3. Send Shadow Map
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpace));

	// 4. Send Point Lights (Street Lamps)
	// Only turn them on if it's Night (Sun is below 0)
	if (sunPos.y < 10.0f) {
		sendPointLights(myCustomShader);
	}
	else {
		// Optional: Turn them off during day by sending 0 color, 
		// or just don't call the function (if uniforms retain value, be careful)
		// Ideally, have a 'ResetLights' function or just leave them on (they look faint in day).
		sendPointLights(myCustomShader); // Leaving them on is usually fine
	}

	// --- A. Draw Opaque Objects (Houses, Ground, Lamps) ---
	drawObjects(myCustomShader, false);


	// --- B. Draw Sun & Moon (Emission Objects) ---
	myCustomShader.useShaderProgram();
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "isObjectEmission"), 1);

	// Draw Moon
	model = glm::mat4(1.0f);
	model = glm::translate(model, moonPos);
	model = glm::scale(model, glm::vec3(2.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	moon.Draw(myCustomShader);

	// Draw Sun
	model = glm::mat4(1.0f);
	model = glm::translate(model, sunPos);
	model = glm::scale(model, glm::vec3(10.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	sun.Draw(myCustomShader);

	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "isObjectEmission"), 0);


	// --- C. Draw Skybox ---
	if (sunPos.y > 0.0f) {
		// DAY: Draw Hemisphere Mesh
		skyboxShader.useShaderProgram();
		glDepthFunc(GL_LEQUAL);
		skyBoxDay.Draw(skyboxShader, view, projection);
		glDepthFunc(GL_LESS);
	}
	else {
		// NIGHT: Draw Cubemap
		skyboxShader.useShaderProgram();
		glDepthFunc(GL_LEQUAL);
		skybox.Draw(skyboxShader, view, projection);
		glDepthFunc(GL_LESS);
	}


	// --- D. Draw Smoke (Transparent) ---
	smokeShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(smokeShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(smokeShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);

	for (Smoke* s : smokeList) {
		s->Update(0.016f);
		s->Render(smokeShader);
	}

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);


	// --- E. Draw Lens Flare ---
	// Only if Sun is roughly visible
	if (sunPos.y > -20.0f) {
		lensFlareEffect->Render(sun.GetPosition(), view, projection);
	}
}

void startMusicZones() {
	ma_result result;

	result = ma_sound_init_from_file(&engine, "music\\tavern_music\\Two Hornpipes (Tortuga).mp3", MA_SOUND_FLAG_STREAM, NULL, NULL, &tavernM);
	if (result != MA_SUCCESS) std::cout << "ERROR: Failed to load Tavern Music!" << std::endl;

	result = ma_sound_init_from_file(&engine, "music\\happy_music\\Nat King Cole   Chestnuts roasting on an open fire.mp3", MA_SOUND_FLAG_STREAM, NULL, NULL, &happyM);
	if (result != MA_SUCCESS) std::cout << "ERROR: Failed to load Happy Music!" << std::endl;

	result = ma_sound_init_from_file(&engine, "music\\choir_music\\O Holy Night  Carols from King's 2017.mp3", MA_SOUND_FLAG_STREAM, NULL, NULL, &choirM);
	if (result != MA_SUCCESS) std::cout << "ERROR: Failed to load Choir Music!" << std::endl;

	// --- 4. SET POSITIONS ---
	
	// Tavern Position
	ma_sound_set_position(&tavernM, 54.5263f, 2.0f, 23.2267f);

	// Happy Position
	ma_sound_set_position(&happyM, 209.2f, 2.0f, 50.2267f);

	// Choir Position
	ma_sound_set_position(&choirM, 129.7f, 24.1f, -135.7f);
	
	// --- 5. SET LOOPING ---
	ma_sound_set_looping(&tavernM, MA_TRUE);
	ma_sound_set_looping(&happyM, MA_TRUE);
	ma_sound_set_looping(&choirM, MA_TRUE);

	// --- 6. SET ATTENUATION (Physics) ---

	// Tavern
	ma_sound_set_attenuation_model(&tavernM, ma_attenuation_model_linear);
	ma_sound_set_min_distance(&tavernM, 10.0f); // Loud within 10 meters
	ma_sound_set_max_distance(&tavernM, 60.0f); // Heard up to 60 meters

	// Happy
	ma_sound_set_attenuation_model(&happyM, ma_attenuation_model_linear);
	ma_sound_set_min_distance(&happyM, 10.0f);
	ma_sound_set_max_distance(&happyM, 60.0f);

	// Choir
	ma_sound_set_attenuation_model(&choirM, ma_attenuation_model_linear);
	ma_sound_set_min_distance(&choirM, 10.0f);
	ma_sound_set_max_distance(&choirM, 60.0f);

	// --- 7. START PLAYING ---
	ma_sound_start(&tavernM);
	ma_sound_start(&happyM);
	ma_sound_start(&choirM);
}

void updateAudioListener() {
	glm::vec3 camPos = myCamera.getCameraPosition();
	//glm::vec3 camDir = myCamera.getCameraTarget(); // Or forward vector

	// ma_engine_listener_set_position(engine, listenerIndex, x, y, z)
	ma_engine_listener_set_position(&engine, 0, camPos.x, camPos.y, camPos.z);



}

void cleanup() {
	glDeleteTextures(1, &depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);
	glfwTerminate();
	ma_engine_uninit(&engine);
	for (auto s : smokeList) delete s;
	smokeList.clear();
}

int main(int argc, const char* argv[]) {

	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}

	initOpenGLState();
	initObjects();
	initSkyBox();
	initSkyBoxDay();
	initShaders();
	initUniforms();
	initFBO();
	initAudio();
	initSmoke();
	initLensFlare();
	// --- SNOW INITIALIZATION (Step 1) ---
	Snow snow;
	// Keep resolution low (256x240) for the NES-style pixel effect, 
	// it will scale up automatically in Render()
	snow.Init(256, 240);

	// Timing variables for Snow physics
	float lastFrame = 0.0f;

	glCheckError();

	startMusicZones();
	while (!glfwWindowShouldClose(glWindow)) {

		// --- CALCULATE DELTA TIME (Step 2) ---
		float currentFrame = glfwGetTime();
		float deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();

		processMovement();
		if (glfwGetMouseButton(glWindow, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
			double mouseX, mouseY;
			glfwGetCursorPos(glWindow, &mouseX, &mouseY);
			myCamera.MouseLook(mouseX, mouseY);
		}
		else {
			myCamera.mouseFirstLook = true;
		}

		// 1. Render the 3D Scene first
		updateAudioListener();
		renderScene();

		

		snow.Update(deltaTime);

		int width, height;
		glfwGetWindowSize(glWindow, &width, &height);


		snow.Render(width, height);

		glfwSwapBuffers(glWindow);
	}

	cleanup();

	return 0;
}