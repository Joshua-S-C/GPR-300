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
#include <jsc/postProcessor.h>

typedef std::vector<jsc::PostProcessEffect*> EffectsList;

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

float guiWidth = 300;

ew::CameraController cameraController;
ew::Camera camera;

ew::Transform planeTransform;

jsc::Material material(1, .5, .5, 128);
jsc::Light light(glm::vec3(1.0));
jsc::DirectionalLight dirLight(glm::vec3(0.0, 1.0, 0.0), light.clr);

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
	GLFWwindow* window = initWindow("Assignment 2 - Shadow Mapping", screenWidth, screenHeight);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

// Setup ----------------------------------------------------------------*/
	light.transform.position.y = 2.0;

	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;

	ew::Shader shader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Shader depthShader = ew::Shader("assets/shadows/depth.vert", "assets/shadows/depth.frag");
	ew::Shader depthDebugShader = ew::Shader("assets/shadows/depth.vert", "assets/shadows/depth.frag");

	ew::Model monkeyModel = ew::Model("assets/suzanne.fbx");
	ew::Transform monkeyTransform;
	GLuint texture = ew::loadTexture("assets/Sand_Texture/Ground080_1K-PNG_Color.png");
	GLuint normalMap = ew::loadTexture("assets/Sand_Texture/Ground080_1K-PNG_NormalDX.png");

	ew::Mesh planeMesh(ew::createPlane(10, 10, 2));
	planeTransform.position = glm::vec3(0, -1.0, 0);

	shader.use();
	//shader.setInt("_MainTex", 1);
	//shader.setInt("_NormalMap", 2);
	
	depthShader.use();
	depthShader.setInt("_DepthMap", 0);

	// Post Processing
	if (false)
	{
		jsc::PostProcessEffect* tintShader = new jsc::TintShader(ew::Shader("assets/post_processing_effects/screen.vert", "assets/post_processing_effects/tint.frag"), 1);
		jsc::PostProcessEffect* negativeShader = new jsc::NegativeShader( ew::Shader("assets/post_processing_effects/screen.vert", "assets/post_processing_effects/negative.frag"), 2);
		jsc::PostProcessEffect* blurShader = new jsc::BoxBlurShader( ew::Shader("assets/post_processing_effects/screen.vert", "assets/post_processing_effects/boxBlur.frag"), 2);

		EffectsList effects;
		effects.push_back(tintShader);
		effects.push_back(negativeShader);
		effects.push_back(blurShader);

		jsc::PostProcessor postProcessor(effects, screenWidth, screenHeight);
	}

	// Depth buffer creation
	GLuint depthFBO;
	glGenFramebuffers(1, &depthFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);

	const GLuint shadowWidth = 1024, shadowHeight = 1024;

	// Depth texture creation
	GLuint depthMap;
	glGenTextures(1, &depthMap);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depthMap);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Attach them
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		printf("ERROR::FRAMEBUFFER:: Framebuffer is not complete!");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GLuint depthVAO;
	glCreateVertexArrays(1, &depthVAO);

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

// Render Loop ----------------------------------------------------------*/
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		camera.aspectRatio = (float)screenWidth / screenHeight;
		cameraController.move(window, &camera, deltaTime);

		//postProcessor.preRender();

		glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));

// Render Scene to Depth ------------------------------------------------*/

		glm::mat4 lightProj, lightView, lightSpaceMatrix;
		float nearPlane = 1.0f, farPlane = 7.5f;

		lightProj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);
		// TODO Change this to use dir lights direction
		lightView = glm::lookAt(
			glm::vec3(-2.0f, 4.0f, -1.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f)); 

		lightSpaceMatrix = lightProj * lightView;

		glViewport(0, 0, shadowWidth, shadowHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
		glClear(GL_DEPTH_BUFFER_BIT);

		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, texture);

		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, normalMap);

		depthShader.use();
		depthShader.setMat4("_LightSpaceMatrix", lightSpaceMatrix);

		//shader.use();
		//shader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());

		depthShader.setMat4("_Model", monkeyTransform.modelMatrix());
		monkeyModel.draw();

		depthShader.setMat4("_Model", planeTransform.modelMatrix());
		planeMesh.draw();

// Uniforms & Draw ------------------------------------------------------*/
		// Back to default
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, screenWidth, screenHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//glBindTexture(GL_TEXTURE_2D, depthMap);

		//shader.use();
		//shader.setMat4("_Model", monkeyTransform.modelMatrix());
		//shader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());

		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, texture);

		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, normalMap);

		//monkeyModel.draw();

		//shader.setMat4("_Model", planeTransform.modelMatrix());
		//planeMesh.draw();

// Post Processing ------------------------------------------------------*/

		//postProcessor.render();
		//postProcessor.draw();

		glViewport(0, 0, screenWidth, screenHeight);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Render Depth to tri
		depthDebugShader.use();
		depthDebugShader.setInt("_DepthMap", 0);
		depthDebugShader.setFloat("_NearPlane", nearPlane);
		depthDebugShader.setFloat("_FarPlane", farPlane);
		
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, depthMap);

		glBindVertexArray(depthVAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);

// More -----------------------------------------------------------------*/
		//drawUI();

#pragma region UI

		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();

		//postProcessor.drawUIWindow();

		ImGui::SetNextWindowPos({ 0,0 });
		ImGui::SetNextWindowSize({ guiWidth, (float)screenHeight });
		ImGui::Begin("Settings", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);

		ImGui::Text("Shadow Map");
		float ratio = guiWidth / shadowWidth;
		ImVec2 imageDrawSize = ImVec2(shadowWidth * ratio, shadowHeight * ratio);

		ImGui::Image((ImTextureID)depthMap, imageDrawSize, ImVec2(0,1), ImVec2(1,0));

		/*
		if (ImGui::CollapsingHeader("View Ping Pong Textures"))
		{
			ImGui::Indent();

			ImGui::Text("Ping Pong 1");
			float ratio = guiWidth / postProcessor.getWidthHeight().x;
			ImVec2 imageDrawSize = ImVec2(postProcessor.getWidthHeight().x * ratio, postProcessor.getWidthHeight().y * ratio);
			ImGui::Image((ImTextureID)postProcessor.getColourTextures()[0], imageDrawSize, ImVec2(0, 1), ImVec2(1, 0));
			
			ImGui::Text("Ping Pong 2");
			ImGui::Image((ImTextureID)postProcessor.getColourTextures()[1], imageDrawSize, ImVec2(0, 1), ImVec2(1, 0));

			ImGui::Unindent();
		}
		*/

		if (ImGui::CollapsingHeader("Scene Settings"))
		{
			ImGui::Indent();

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

			ImGui::Unindent();
		}

		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

#pragma endregion

		glfwSwapBuffers(window);
	}
	
	glDeleteVertexArrays(1, &depthVAO);

	printf("Sayonara!");
}

// I think about this later
void drawUI() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::SetNextWindowPos({ 0,0 });
	ImGui::SetNextWindowSize({ 300, (float)screenHeight });
	ImGui::Begin("Settings", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);

	if (ImGui::CollapsingHeader("Post Processing")) 
	{

	}


	if (ImGui::CollapsingHeader("Scene Settings")) 
	{

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
