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
#include <jsc/framebuffer.h>

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
ew::Camera shadowCamera;

ew::Transform planeTransform;

jsc::Material material(1, .5, .5, 128);
jsc::Light light(glm::vec3(1.0));
jsc::DirectionalLight dirLight(glm::vec3(1.0, 1.0, 0.0), light.clr);

struct AppSettings {
	bool useNormalMap = true;
	bool useDirectionalLight = true;
	bool useCustomAmbientClr = true;

	glm::vec4 clearClr = glm::vec4(0.6f, 0.8f, 0.92f, 1.0f);


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

	// Shadow Cam Setup
	float minShadowBias = 0.005, maxShadowBias = 0.05;
 	float shadowCamDistance = 3;
	shadowCamera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	shadowCamera.position = glm::normalize(dirLight.dir) * shadowCamDistance;
	shadowCamera.aspectRatio = (float)screenWidth / screenHeight;
	shadowCamera.fov = 60.0f;

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
	shader.setInt("_MainTex", 1);
	shader.setInt("_NormalMap", 2);
	shader.setInt("_ShadowMap", 3);
	
	depthShader.use();
	depthShader.setInt("_DepthMap", 3);

	// Post Processing
	jsc::PostProcessEffect* tintShader = new jsc::TintShader(ew::Shader("assets/post_processing_effects/screen.vert", "assets/post_processing_effects/tint.frag"), 1);

	EffectsList effects;
	effects.push_back(tintShader);

	jsc::PostProcessor postProcessor(effects, screenWidth, screenHeight);
	postProcessor.updateTextureIndex(0);

	// Shadows
	const GLuint shadowWidth = 2048, shadowHeight = 2048;
	jsc::Framebuffer shadowFB = jsc::Framebuffer::createFramebufferDepth(shadowWidth, shadowHeight);

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

		cameraController.move(window, &camera, deltaTime);

		camera.aspectRatio = (float)screenWidth / screenHeight;

		shadowCamera.aspectRatio = (float)screenWidth / screenHeight;
		shadowCamera.position = glm::normalize(dirLight.dir) * shadowCamDistance;

		glClearColor(settings.clearClr.x, settings.clearClr.y, settings.clearClr.z, settings.clearClr.w);
		
		monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));

// Shadow Pass ----------------------------------------------------------*/

		glm::mat4 lightProj, lightView, lightSpaceMatrix;
		float nearPlane = 1.0f, farPlane = 7.5f;

		lightProj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);
		lightView = glm::lookAt(
			glm::vec3(shadowCamera.position.x, shadowCamera.position.y, shadowCamera.position.z),
			glm::vec3(shadowCamera.target.x, shadowCamera.target.y, shadowCamera.target.z),
			glm::vec3(0.0f, 1.0f, 0.0f)); 

		lightSpaceMatrix = lightProj * lightView;

		glViewport(0, 0, shadowWidth, shadowHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, shadowFB.fbo);
		glClear(GL_DEPTH_BUFFER_BIT);

		glCullFace(GL_FRONT);

		depthShader.use();
		depthShader.setMat4("_LightSpaceMatrix", lightSpaceMatrix);

		depthShader.setMat4("_Model", monkeyTransform.modelMatrix());
		monkeyModel.draw();

		depthShader.setMat4("_Model", planeTransform.modelMatrix());
		planeMesh.draw();
		
		glCullFace(GL_BACK);

// Lighting Pass --------------------------------------------------------*/

		// Back to default FB
		postProcessor.preRender();
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, screenWidth, screenHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();

		// Lighting
		shader.setMaterial("_Material", material);
		shader.setLight("_Light", light);
		shader.setVec3("_DirectionalLight.dir", dirLight.dir);
		shader.setBool("_UseNormalMap", settings.useNormalMap);
		shader.setBool("_UseDirectionalLight", settings.useDirectionalLight);
		shader.setBool("_UseAmbientClr", settings.useCustomAmbientClr);

		shader.setFloat("_MinShadowBiasK", minShadowBias);
		shader.setFloat("_MaxShadowBiasK", maxShadowBias);

		// Textures
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, normalMap);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, shadowFB.depthBuffer);

		shader.setInt("_MainTex", 1);
		shader.setInt("_NormalMap", 2);
		shader.setInt("_ShadowMap", 3);

		// Models
		shader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
		shader.setMat4("_LightSpaceMatrix", lightSpaceMatrix);
		
		shader.setMat4("_Model", monkeyTransform.modelMatrix());
		monkeyModel.draw();

		shader.setMat4("_Model", planeTransform.modelMatrix());
		planeMesh.draw();

// Post Process Pass ----------------------------------------------------*/
		postProcessor.render();
		postProcessor.draw();

// More -----------------------------------------------------------------*/
		//drawUI();

#pragma region UI

		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();

		postProcessor.drawUIWindow();
		//postProcessor.drawDebugUIWindow();

		ImGui::SetNextWindowPos({ 0,0 });
		ImGui::SetNextWindowSize({ guiWidth, (float)screenHeight });
		ImGui::Begin("Settings", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);

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

		if (ImGui::CollapsingHeader("Light & Shadow Settings"))
		{
			ImGui::Indent();

			float ratio = guiWidth / shadowWidth;
			ImVec2 imageDrawSize = ImVec2(shadowWidth * ratio, shadowHeight * ratio);
			ImGui::Image((ImTextureID)shadowFB.depthBuffer, imageDrawSize, ImVec2(0, 1), ImVec2(1, 0));

			if (ImGui::Button("Reset Camera"))
				resetCamera(&camera, &cameraController);

			ImGui::SliderFloat("Light Distance", &shadowCamDistance, 0, 10);

			ImGui::Checkbox("Use Directional Light", &settings.useDirectionalLight);
			if (settings.useDirectionalLight)
				dirLight.drawUI();
			else
				light.drawUI();

			ImGui::DragFloat("Min Shadow Bias", &minShadowBias, 0.001f, 0, 1);
			ImGui::DragFloat("Max Shadow Bias", &maxShadowBias, 0.001f, 0, 1);

			ImGui::Checkbox("Use Custom Ambient Clr", &settings.useCustomAmbientClr);

			if (settings.useCustomAmbientClr) {
				ImGui::Text("Too Lazy to update Material struct rn");
			}

			ImGui::Combo("Render Type", &settings.renderTypeIndex, settings.renderTypeNames, IM_ARRAYSIZE(settings.renderTypeNames));


			ImGui::Unindent();
		}

		if (ImGui::CollapsingHeader("Scene Settings"))
		{
			ImGui::Indent();

			ImGui::Checkbox("Use Normal Map", &settings.useNormalMap);


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
