#include "../ew/shader.h"
#include <imgui.h>

// o yea I was gonna make this a while ago, it doesn't need to be for post processing, its just there to draw ui functions and update the shader uniforms outside of main
namespace jsc {
	struct PostProcessEffect
	{
		ew::Shader shader;
		std::string name;

		PostProcessEffect(ew::Shader shader, int screenTexIndex) :
			shader(shader)
		{
			shader.use();
			shader.setInt("_ScreenTexture", screenTexIndex); // This may need to be changed later
		}

		~PostProcessEffect()
		{}
		
		// TODO this maybe idk
		//std::string relativePath; 

		void renderToTexture(GLuint texture) {
			shader.use();
			updateShader();
			glBindTexture(GL_TEXTURE_2D, texture);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}

		void updateTextureIndex(int screenTexIndex) {
			shader.setInt("_ScreenTexture", screenTexIndex); // This may need to be changed later
		}

		// TODO Make UI work when having 2 of the same effect (doesn't work cuz the UI parts have the exact same names)
		virtual void drawUI() = 0;

		virtual void updateShader() = 0;
	};

	struct TintShader : public PostProcessEffect
	{
		bool active = true;
		float tintStrength = 0.2;
		glm::vec3 tintColour = glm::vec3(1.0, 0.0, 1.0);

		TintShader(ew::Shader shader, int screenTexIndex) :
			PostProcessEffect(shader, screenTexIndex)
		{
			name = "Tint Shader";
			updateShader();
		}

		void drawUI() {
			ImGui::Text("Tint Shader");
			ImGui::Checkbox("Active (Tint)", &active);
			ImGui::DragFloat("Strength", &tintStrength, 0.001, 0, 1);
			ImGui::ColorPicker3("Tint Colour", &tintColour.x);
		}

		void updateShader() {
			shader.use();
			shader.setFloat("_TintStrength", tintStrength);
			shader.setVec3("_TintColour", tintColour);
			shader.setBool("_Active", active);
		}
	};

	struct NegativeShader : public PostProcessEffect
	{
		bool active = true;

		NegativeShader(ew::Shader shader, int screenTexIndex) :
			PostProcessEffect(shader, screenTexIndex)
		{
			name = "Negative Shader";
			updateShader();
		}

		void drawUI() {
			ImGui::Text("Negative Shader");
			ImGui::Checkbox("Active (Negative)", &active);
		}

		void updateShader() {
			shader.use();
			shader.setBool("_Active", active);
		}
	};

	struct BoxBlurShader : public PostProcessEffect
	{
		bool active = true;
		int kernelSize = 3;

		BoxBlurShader(ew::Shader shader, int screenTexIndex) :
			PostProcessEffect(shader, screenTexIndex)
		{
			name = "Box Blur Shader";
			updateShader();
		}

		void drawUI() {
			ImGui::Text("Blur Shader");
			ImGui::Checkbox("Active (Box Blur)", &active);
			
			if (ImGui::SliderInt("Kernel Size", &kernelSize, 0, 30)) {
				if (kernelSize % 2 == 0)
					kernelSize--;
			}
		}

		void updateShader() {
			shader.use();
			shader.setBool("_Active", active);
			shader.setInt("_KernelSize", kernelSize);
		}
	};

	struct GaussianBlurShader : public PostProcessEffect
	{
		bool active = true;
		int kernelSize = 3;
		float sigma = 5;

		GaussianBlurShader(ew::Shader shader, int screenTexIndex) :
			PostProcessEffect(shader, screenTexIndex)
		{
			name = "Gaussian Blur Shader";
			updateShader();
		}

		void drawUI() {
			ImGui::Text("Gaussian Blur Shader");
			ImGui::Checkbox("Active (Gaussian Blur)", &active);
			ImGui::DragInt("Kernel Size", &kernelSize);
			ImGui::DragFloat("Sigma", &sigma);
		}

		void updateShader() {
			shader.use();
			shader.setBool("_Active", active);
			shader.setInt("_KernelSize", kernelSize);
			shader.setInt("_Sigma", sigma);
		}
	};
}
