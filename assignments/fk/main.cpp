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
#include <jsc/spline.h>

// hey
#include "../ImGuizmo.h"

typedef std::vector<jsc::PostProcessEffect*> EffectsList;
typedef std::vector<jsc::KeyFrame<glm::vec3>> KeysVec3;


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
	GLFWwindow* window = initWindow("Assignment 5 - Forward Kinematics", screenWidth, screenHeight);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

// Setup ----------------------------------------------------------------*/
	light.transform.position.y = 2.0;

	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;

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
	ew::Transform monkeyTransform = ew::Transform(glm::vec3(0, 5, 0), glm::quat(), glm::vec3(.2, .2, .2));
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

	// Shadow the Hedgehog
	const GLuint shadowWidth = 2048, shadowHeight = 2048;
	jsc::Framebuffer shadowFB = jsc::Framebuffer::createFramebufferDepth(shadowWidth, shadowHeight);

	// Animation
	jsc::Animator animator;

	animator.clip = new jsc::AnimationClip();
	animator.clip->duration = 2;

	animator.clip->posKeys.push_back(jsc::KeyFrame<glm::vec3>(0, glm::vec3(0,0,0)));
	animator.clip->posKeys.push_back(jsc::KeyFrame<glm::vec3>(5, glm::vec3(0,2,0)));

	animator.clip->scaleKeys.push_back(jsc::KeyFrame<glm::vec3>(0, glm::vec3(1,1,1)));
	animator.clip->scaleKeys.push_back(jsc::KeyFrame<glm::vec3>(5, glm::vec3(2,2,2)));

	animator.clip->rotKeys.push_back(jsc::KeyFrame<glm::vec3>(0, glm::vec3(0,0,0)));
	animator.clip->rotKeys.push_back(jsc::KeyFrame<glm::vec3>(5, glm::vec3(0,3,0)));

#pragma region Splines

	// Splines
	jsc::Spline spline1(
		ew::Shader("assets/unlit_line.vert", "assets/unlit_line.frag"), 
		ew::Shader("assets/unlit.vert", "assets/unlit.frag"),
		"Spline 1"
	);

	spline1.addPoint(
		ew::Transform(glm::vec3(0.0f, 0.0f, 0.0f),
			glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
			glm::vec3(1.0f, 1.0f, 1.0f))
	);

	//spline1.addPoint(
	//	ew::Transform(spline1.points.front().position + glm::eulerAngles(spline1.points.front().rotation) * spline1.points.front().scale,
	//		glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
	//		glm::vec3(1.0f, 1.0f, 1.0f)
	//	)
	//);

	//spline1.addPoint(
	//	ew::Transform(glm::vec3(0.0f, 0.0f, 0.0f), 
	//		glm::quat(1.0f, 0.0f, 0.0f, 0.0f), 
	//		glm::vec3(1.0f, 1.0f, 1.0f))
	//);

	spline1.addPoint(
		ew::Transform(glm::vec3(5.0f, 0.0f, 0.0f), 
		glm::quat(1.0f, 0.0f, 0.0f, 0.0f), 
		glm::vec3(1.0f, 1.0f, 1.0f))
	);

	//spline1.addPoint(
	//	ew::Transform(glm::vec3(10.0f, 3.0f, 0.0f), 
	//		glm::quat(1.0f, 0.0f, 0.0f, 0.0f), 
	//		glm::vec3(1.0f, 1.0f, 1.0f))
	//);

	jsc::Spline spline2(
		ew::Shader("assets/unlit_line.vert", "assets/unlit_line.frag"),
		ew::Shader("assets/unlit.vert", "assets/unlit.frag"),
		"Spline 2"
	);

	spline2.clr = glm::vec3(1.0, 1.0, 0);
	spline2.width = 1;

	spline2.addPoint(
		ew::Transform(glm::vec3(-3.0f, 0.0f, 0.0f),
			glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
			glm::vec3(1.0f, 1.0f, 1.0f))
	);

	spline2.addPoint(
		ew::Transform(glm::vec3(-5.0f, 1.0f, 0.0f),
			glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
			glm::vec3(1.0f, 1.0f, 1.0f))
	);

#pragma endregion

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
		
// Animation ------------------------------------------------------------*/
#pragma region Animation
		//animator.playbackTime = 1; // Debug

		//animator.update(deltaTime);
		
		//monkeyTransform.position = spline1.getValue(animator.playbackTime).position + glm::eulerAngles(spline1.getValue(animator.playbackTime).rotation);
		monkeyTransform.position = spline1.getValue(animator.playbackTime).position;
		monkeyTransform.rotation = glm::lookAt(
			monkeyTransform.position,
			monkeyTransform.position + glm::eulerAngles(spline1.getValue(animator.playbackTime).rotation),
			glm::vec3(0, 0, 1)
		);
			

		//monkeyTransform.position = 
		//	animator.getValue(animator.clip->posKeys, monkeyTransform.position);
		//monkeyTransform.scale = 
		//	animator.getValue(animator.clip->scaleKeys, monkeyTransform.scale);
		//monkeyTransform.rotation = 
		//	animator.getValue(animator.clip->rotKeys, glm::eulerAngles(monkeyTransform.rotation));

#pragma endregion

// Shadow Pass ----------------------------------------------------------*/
#pragma region Shadow Pass
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


#pragma endregion

// Lighting Pass --------------------------------------------------------*/
#pragma region Lighting Pass
		// Back to default FB
		//postProcessor.preRender();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

		// Splines
		spline1.draw(camera);
		spline1.debugDrawVelocity(camera, animator.playbackTime);
		spline2.draw(camera);

#pragma endregion

// Post Process Pass ----------------------------------------------------*/
#pragma region Post Process Pass
		//postProcessor.render();
		//postProcessor.draw();

#pragma endregion

// More -----------------------------------------------------------------*/
		//drawUI();

#pragma region UI

		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();

		//postProcessor.drawUIWindow();
		//postProcessor.drawDebugUIWindow();

		// Shadow Map UI
		//ImGui::SetNextWindowPos({ screenWidth - guiWidth * 2,0 });
		//ImGui::SetNextWindowSize({ guiWidth, guiWidth });
		//ImGui::Begin("Shadow Map", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);

		//float ratio = guiWidth / shadowWidth;
		//ImVec2 imageDrawSize = ImVec2(shadowWidth * ratio, shadowHeight * ratio);
		//ImGui::Image((ImTextureID)shadowFB.depthBuffer, imageDrawSize, ImVec2(0, 1), ImVec2(1, 0));
		//
		//ImGui::End();

		// Animator UI
		ImGui::SetNextWindowPos({ screenWidth - guiWidth,0 });
		ImGui::SetNextWindowSize({ guiWidth, (float)screenHeight });
		ImGui::Begin("Animation", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);

		animator.showUI();

		ImGui::End();

		// Spline UI. 
		ImGui::SetNextWindowPos({ screenWidth - guiWidth * 2,0 });
		ImGui::SetNextWindowSize({ guiWidth, (float)screenHeight });
		ImGui::Begin("Spline 1", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);

		//spline1.showUI();

		ImGui::End();

		// The Rest of the UI
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

		ImGui::Text(std::to_string(spline1.getValue(animator.playbackTime).position.y).c_str());

		if (ImGui::CollapsingHeader("Light & Shadow Settings"))
		{
			ImGui::Indent();

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
