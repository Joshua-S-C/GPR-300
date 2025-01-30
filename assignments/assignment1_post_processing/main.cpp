#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <ew/shader.h>
#include <ew/model.h>
#include <ew/camera.h>
#include <ew/transform.h>
#include <ew/cameraController.h>
#include <ew/texture.h>
#include <ew/procGen.h>

#include <jsc/light.h>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();
void drawTransformUI(ew::Transform &transform);
void resetCamera(ew::Camera* camera, ew::CameraController* controller);

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

ew::CameraController cameraController;
ew::Camera camera;

ew::Transform planeTransform;

jsc::Material material(1, .5, .5, 128);
jsc::Light light(glm::vec3(1.0));
jsc::DirectionalLight dirLight(glm::vec3(0.0, 1.0, 0.0));

struct AppSettings {
	bool useNormalMap = true;
	bool useDirectionalLight = true;

	int renderTypeIndex = 0;
	enum RenderTypes {
		Textured = 0,
		World_Normals = 1,
		Normal_Map = 2
	};
	const char* renderTypeNames[4] = { 
		"Textured", 
		"Unlit",
		"World Normals",
		"Normal Map"
	};
}settings;


int main() {
	GLFWwindow* window = initWindow("Assignment 1 - Post Processing", screenWidth, screenHeight);

// Setup ----------------------------------------------------------------*/
	light.transform.position.y = 2.0;

	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;

	ew::Shader shader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Shader screenShader = ew::Shader("assets/screen.vert", "assets/screen.frag");

	ew::Model monkeyModel = ew::Model("assets/suzanne.fbx");
	ew::Transform monkeyTransform;
	GLuint texture = ew::loadTexture("assets/Sand_Texture/Ground080_1K-PNG_Color.png");
	GLuint normalMap = ew::loadTexture("assets/Sand_Texture/Ground080_1K-PNG_NormalDX.png");

	ew::Mesh planeMesh(ew::createPlane(10, 10, 2));
	planeTransform.position = glm::vec3(0, -1.0, 0);

	shader.use();
	shader.setInt("_MainTex", 0);
	shader.setInt("_NormalMap", 1);

	screenShader.use();
	screenShader.setInt("_ScreenTexture", 1); // Cant be 0 cuz of normal map

// Framebuffer Setup ----------------------------------------------------*/

	// TODO Objectify this. 

	unsigned int dummyVAO;
	glCreateVertexArrays(1, &dummyVAO);

	// The actual frame buffer
	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// Create and attach Colour
	GLuint colourAttachment;
	glGenTextures(1, &colourAttachment);
	glBindTexture(GL_TEXTURE_2D, colourAttachment);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colourAttachment, 0);

	// Used for depth and stencil buffers. tbh still not fully sure what thedeal is
	GLuint rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	// Framebuffer validation
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		printf("[Error] Framebuffer is incomplete");

	// Bind it
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

// Render Loop ----------------------------------------------------------*/
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		camera.aspectRatio = (float)screenWidth / screenHeight;
		cameraController.move(window, &camera, deltaTime);

// Screen Shader Per Frame Setup ----------------------------------------*/
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.6f, 0.8f, 0.92f, 1.0f); // Literally random numbers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

// Uniforms & Draw ------------------------------------------------------*/
		monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));	

		shader.use();
		shader.setMat4("_Model", monkeyTransform.modelMatrix());
		shader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalMap);

		shader.setMaterial("_Material", material);
		shader.setLight("_Light", light);
		shader.setVec3("_DirectionalLight.dir", dirLight.dir);
		shader.setBool("_UseNormalMap", settings.useNormalMap);
		shader.setBool("_UseDirectionalLight", settings.useDirectionalLight);

		monkeyModel.draw();

		shader.setMat4("_Model", planeTransform.modelMatrix());
		planeMesh.draw();

// Draw Screen Shader ---------------------------------------------------*/

		// Bind to other fbo and disable depth testing
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);
		glClearColor(0.5f, 0.1f, 0.1f, 1.0f); // Different random numbers. If these show up, theres a problem lol
		glClear(GL_COLOR_BUFFER_BIT);

		screenShader.use();
		glBindVertexArray(dummyVAO);
		glBindTexture(GL_TEXTURE_2D, colourAttachment);	// Colour attachment -> screen quad
		glDrawArrays(GL_TRIANGLES, 0, 3);

// More -----------------------------------------------------------------*/

		drawUI();

		glfwSwapBuffers(window);
	}

	// Tutorial had these in here.
	// TODO Add these to the Framebuffer class destructor or something
	glDeleteVertexArrays(1, &dummyVAO);
	glDeleteRenderbuffers(1, &rbo);
	glDeleteFramebuffers(1, &fbo);

	printf("Sayonara!");
}

void drawUI() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Settings");

	ImGui::Text("Hai");

	if (ImGui::Button("Reset Camera"))
		resetCamera(&camera, &cameraController);

	ImGui::Checkbox("Use Normal Map", &settings.useNormalMap);
	
	ImGui::Combo("Render Type", &settings.renderTypeIndex, settings.renderTypeNames, IM_ARRAYSIZE(settings.renderTypeNames));

	ImGui::Checkbox("Use Directional Light", &settings.useDirectionalLight);
	if (settings.useDirectionalLight)
		dirLight.drawUI();
	else
		light.drawUI();
	
	material.drawUI();
	
	if (ImGui::CollapsingHeader("Plane Transform")) {
		drawTransformUI(planeTransform);
	}

	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void drawTransformUI(ew::Transform &transform) {
	ImGui::DragFloat3("Position", &transform.position.x, .05f,  -10.0f, 10.0f);
	ImGui::DragFloat4("Rotation", &transform.rotation.x, .05f, -10.0f, 10.0f);
	ImGui::DragFloat3("Scale", &transform.scale.x, .05f, -10.0f, 10.0f);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	screenWidth = width;
	screenHeight = height;
}

/// <summary>
/// Initializes GLFW, GLAD, and IMGUI
/// </summary>
/// <param name="title">Window title</param>
/// <param name="width">Window width</param>
/// <param name="height">Window height</param>
/// <returns>Returns window handle on success or null on fail</returns>
GLFWwindow* initWindow(const char* title, int width, int height) {
	printf("Salut!");
	if (!glfwInit()) {
		printf("GLFW failed to init!");
		return nullptr;
	}

	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (window == NULL) {
		printf("GLFW failed to create window");
		return nullptr;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGL(glfwGetProcAddress)) {
		printf("GLAD Failed to load GL headers");
		return nullptr;
	}

	//Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	return window;
}

// From Eric
void resetCamera(ew::Camera* camera, ew::CameraController* controller) {
	camera->position = glm::vec3(0, 0, 5.0f);
	camera->target = glm::vec3(0);
	controller->yaw = controller->pitch = 0;
}
