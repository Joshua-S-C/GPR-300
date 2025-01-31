#include "../ew/shader.h"
#include <imgui.h>

// o yea I was gonna make this a while ago, it doesn't need to be for post processing, its just there to draw ui functions and update the shader uniforms outside of main
namespace jsc {
	struct PostProcessEffect
	{
		PostProcessEffect(ew::Shader shader, int screenTexIndex) :
			shader(shader)
		{
			shader.use();
			shader.setInt("_ScreenTexture", screenTexIndex); // This may need to be changed later
		}

		~PostProcessEffect()
		{}

		ew::Shader shader;
		
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
		float tintStrength = 0.5;
		glm::vec3 tintColour = glm::vec3(1.0, 0.0, 1.0);

		TintShader(ew::Shader shader, int screenTexIndex) :
			PostProcessEffect(shader, screenTexIndex)
		{
			updateShader();
		}

		void drawUI() {
			ImGui::Text("Tint Shader");
			ImGui::DragFloat("Strength", &tintStrength, 0.001, 0, 1);
			ImGui::ColorPicker3("Tint Colour", &tintColour.x);
		}

		void updateShader() {
			shader.use();
			shader.setFloat("_TintStrength", tintStrength);
			shader.setVec3("_TintColour", tintColour);
		}
	};

	struct NegativeShader : public PostProcessEffect
	{
		bool active = true;

		NegativeShader(ew::Shader shader, int screenTexIndex) :
			PostProcessEffect(shader, screenTexIndex)
		{
			updateShader();
		}

		void drawUI() {
			ImGui::Text("Negative Shader");
			ImGui::Checkbox("Active", &active);
		}

		void updateShader() {
			shader.use();
			shader.setBool("_Active", active);
		}
	};
}
