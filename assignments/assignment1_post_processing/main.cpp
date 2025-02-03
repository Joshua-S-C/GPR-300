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

	ew::Model monkeyModel = ew::Model("assets/suzanne.fbx");
	ew::Transform monkeyTransform;
	GLuint texture = ew::loadTexture("assets/Sand_Texture/Ground080_1K-PNG_Color.png");
	GLuint normalMap = ew::loadTexture("assets/Sand_Texture/Ground080_1K-PNG_NormalDX.png");

	ew::Mesh planeMesh(ew::createPlane(10, 10, 2));
	planeTransform.position = glm::vec3(0, -1.0, 0);

	shader.use();
	shader.setInt("_MainTex", 0);
	shader.setInt("_NormalMap", 1);

	std::string ppPath = "assets/post_processing_effects/";
	
	// Post Processing Setup
	//jsc::PostProcessEffect* tintShader = new jsc::TintShader(ew::Shader(ppPath + "screen.vert", ppPath + "tint.frag"), 1);
	//jsc::PostProcessEffect* negativeShader = new jsc::NegativeShader( ew::Shader(ppPath + "screen.vert", ppPath + "negative.frag"), 2);
	jsc::PostProcessEffect* depthShader = new jsc::DepthShader( ew::Shader(ppPath + "screen.vert", ppPath + "depth.frag"), 1);
	//jsc::PostProcessEffect* passthroughShader = new jsc::PassthroughShader( ew::Shader(ppPath + "screen.vert", ppPath + "screen.frag"), 1);

	EffectsList effects;
	//effects.push_back(tintShader);
	//effects.push_back(negativeShader);
	effects.push_back(depthShader);
	//effects.push_back(passthroughShader);

	//jsc::PostProcessor postProcessor(effects, screenWidth, screenHeight);

	GLuint fbo; 
	GLuint depthBuffer;
	glCreateFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(1, &depthBuffer);
	glBindTexture(GL_TEXTURE_2D, depthBuffer);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, screenWidth, screenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBuffer, 0);

	glDrawBuffer(GL_NONE);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GLuint dummyVAO;
	glCreateVertexArrays(1, &dummyVAO);

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
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glViewport(0, 0, screenWidth, screenHeight);
		glClearColor(1.0, 0.0, 0.0, 1.0f);
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

// Post Processing ------------------------------------------------------*/

		//postProcessor.render();
		//postProcessor.draw();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, screenWidth, screenHeight);
		glClearColor(1.0, 1.0, 0.0, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindTextureUnit(0, depthBuffer);
		effects[0]->shader.use();
		glBindVertexArray(dummyVAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		

// More -----------------------------------------------------------------*/
		//drawUI();

#pragma region UI

		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();

		ImGui::SetNextWindowPos({ 0,0 });
		ImGui::SetNextWindowSize({ guiWidth, (float)screenHeight });
		ImGui::Begin("Settings", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
		
		float ratio = guiWidth / screenWidth;

		ImGui::Image((ImTextureID)depthBuffer, ImVec2(screenWidth * ratio, screenHeight * ratio), ImVec2(0, 1), ImVec2(1, 0));


		//if (ImGui::CollapsingHeader("Post Processing"))
		//{
		//	ImGui::Indent();
		//	// Same thing as below but for more than 2 effects (ignore that its not scaled properly)
		//	postProcessor.drawDebuggingUI();

		//	ImGui::Text("Screen Texture");
		//	float ratio = guiWidth / postProcessor.getWidthHeight().x;
		//	ImVec2 imageDrawSize = ImVec2(postProcessor.getWidthHeight().x * ratio, postProcessor.getWidthHeight().y * ratio);
		//	ImGui::Image((ImTextureID)postProcessor.getColourTextures()[0], imageDrawSize, ImVec2(0, 1), ImVec2(1, 0));
		//	
		//	ImGui::Text("Screen Texture 1");
		//	ImGui::Image((ImTextureID)postProcessor.getColourTextures()[1], imageDrawSize, ImVec2(0, 1), ImVec2(1, 0));

		//	// TODO Reording
		//	// Look at for Drag and Drop
		//	// https://github.com/ocornut/imgui/issues/1931
		//	if (ImGui::Button("[TEMP] Swap Effect Order")) {
		//		EffectsList tempList = postProcessor.effects;
		//		tempList[0] = postProcessor.effects[1];
		//		tempList[1] = postProcessor.effects[0];
		//		postProcessor.effects = tempList;
		//		postProcessor.updateTextureIndex();
		//	}

		//	postProcessor.drawUI();
		//	ImGui::Unindent();
		//}


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
