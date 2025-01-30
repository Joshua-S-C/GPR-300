#include "../ew/shader.h"
#include <imgui.h>

// o yea I was gonna make this a while ago, it doesn't need to be for post processing, its just there to draw ui functions and update the shader uniforms outside of main
namespace jsc {
	struct PostProcessEffect
	{
		PostProcessEffect(ew::Shader shader) :
			shader(shader)
		{
			shader.use();
			shader.setInt("_ScreenTexture", 1); // This may need to be changed later
		}

		~PostProcessEffect()
		{}

		ew::Shader shader;

		//std::string relativePath; // TODO this maybe idk

		virtual void drawUI() = 0;

		virtual void updateShader() = 0;
	};

	struct TintShader : public PostProcessEffect
	{
		float tintStrength = 0.5;
		glm::vec3 tintColour = glm::vec3(1.0, 0.0, 1.0);

		TintShader(ew::Shader shader) :
			PostProcessEffect(shader)
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

		NegativeShader(ew::Shader shader) :
			PostProcessEffect(shader)
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

namespace jsc {
	class PostProcessor
	{
	public:
		PostProcessor(ew::Shader shader, unsigned int width, unsigned int height);
		PostProcessor(PostProcessEffect* effect, unsigned int width, unsigned int height);
		~PostProcessor();

		//ew::Shader shader; // The power is in our hands // jk
			
		PostProcessEffect* effect = nullptr;

		/// <summary>
		/// Called at the start of Render Loop
		/// </summary>
		void preRender();

		/// <summary>
		/// Called after objects have been drawn
		/// </summary>
		void postRender();

		/// <summary>
		/// Actually draws the effect
		/// </summary>
		void draw();

		void createColourAttachment(unsigned int width, unsigned int height);

		GLuint getColourAttachment() {
			return colourAttachment;
		}

		glm::vec2 getWidthHeight() {
			return glm::vec2(width, height);
		}

	private:
		unsigned int width, height;
		GLuint FBO, RBO, VAO;

		GLuint colourAttachment;	// The texture
	};

	// cringe constructor that Im too lazy to move even though I couldve done it in the time I typed this comment
	PostProcessor::PostProcessor(ew::Shader shader, unsigned int width, unsigned int height) :
		/*shader(shader),*/ width(width), height(height)
	{
		// Dummy VAO
		glCreateVertexArrays(1, &VAO);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// FBO
		glGenFramebuffers(1, &FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

		// RBO : Used for depth and stencil buffers. tbh still not fully sure what thedeal is
		glGenRenderbuffers(1, &RBO);
		glBindRenderbuffer(GL_RENDERBUFFER, RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);


		// Framebuffer validation
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			printf("[Error] Framebuffer is incomplete");
	}

	PostProcessor::PostProcessor(PostProcessEffect* effect, unsigned int width, unsigned int height) :
		PostProcessor(effect->shader, width, height)
	{
		this->effect = effect;
	}

	PostProcessor::~PostProcessor()
	{
		glDeleteVertexArrays(1, &VAO);
		glDeleteRenderbuffers(1, &RBO);
		glDeleteFramebuffers(1, &FBO);
	}

	void PostProcessor::preRender()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.6f, 0.8f, 0.92f, 1.0f); // Literally random numbers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void PostProcessor::postRender()
	{
		// Bind to other fbo and disable depth testing
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);
		glClearColor(0.5f, 0.1f, 0.1f, 1.0f); // Different random numbers. If these show up, theres a problem lol
		glClear(GL_COLOR_BUFFER_BIT);
	}
	
	void PostProcessor::draw()
	{
		//shader.use();
		effect->shader.use();
		glBindVertexArray(VAO);
		glBindTexture(GL_TEXTURE_2D, colourAttachment);	// Colour attachment -> screen quad
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}
	
	void PostProcessor::createColourAttachment(unsigned int width, unsigned int height)
	{
		glBindTexture(GL_TEXTURE_2D, colourAttachment);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colourAttachment, 0);


		// Bind it
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	
}

