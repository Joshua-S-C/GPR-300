#include <imgui.h>

#include "../ew/shader.h"
#include "../jsc/postProcessEffect.h"

typedef std::vector<jsc::PostProcessEffect*> EffectsList;

namespace jsc {
	class PostProcessor
	{
	public:
		PostProcessor(EffectsList effect, unsigned int width, unsigned int height);
		~PostProcessor();

		EffectsList effects;

		/// <summary>
		/// Called at the start of Render Loop
		/// </summary>
		void preRender();

		/// <summary>
		/// Call this after objects are drawn. Draws to the texture and prepares to draw
		/// </summary>
		void render(GLuint frameBuffer = 0);

		/// <summary>
		/// Renders the full screen tri
		/// </summary>
		void draw(GLuint frameBuffer = 0);

		void updateTextureIndex(int screenTexIndexStart = 1) {
			int i = 0;

			for each(PostProcessEffect * effect in effects)
			{
				effect->updateTextureIndex(screenTexIndexStart + i);
				i++;
				}
		}

		void drawUIWindow() {
			ImGui::SetNextWindowPos({ (float)width - 500,0 });
			ImGui::SetNextWindowSize({ 500, 300 });
			ImGui::Begin("Post Processing Editor", 0);

			ImGui::Columns(2, "locations");
			ImGui::SetColumnWidth(0, 250.0f);
			ImGui::SetColumnWidth(1, 250.0f);

			drawAllImages();

			ImGui::NextColumn();

			// For some reason, pressing this makes it so that more than 2 render effects work. Doesn't work when I put this in the constructor tho so I don't know what's happening
			// TODO Reording
			// Look at for Drag and Drop
			// https://github.com/ocornut/imgui/issues/1931
			if (ImGui::Button("Swap Effect Order")) {
				EffectsList tempList = effects;
				tempList[0] = effects[1];
				tempList[1] = effects[0];
				effects = tempList;
				updateTextureIndex();
			}

			for each (PostProcessEffect* effect in effects)
			{
				effect->drawUI();
			}

			ImGui::End();
		}


		int _texIndex = 0;
		void drawDebugUIWindow() {
			ImGui::SetNextWindowPos({ 300,0 });
			ImGui::SetNextWindowSize({ 200, 300 });
			ImGui::Begin("Post Processing Debug", 0, ImGuiWindowFlags_NoResize);

			ImGui::SliderInt("Tex Index", &_texIndex, -3, 5);

			if (ImGui::Button("Update Texture Index")) {
				updateTextureIndex(_texIndex);
			}

			ImGui::End();
		}

		GLuint* getColourTextures() {
			return texClr;
		}

		glm::vec2 getWidthHeight() {
			return glm::vec2(width, height);
		}

		// TODO Fix the aspect ratio on this, cuz for some reason calcing the aspect ratio (as done in main) doesn't work :3
		// For when there's more than 2 effects
		void drawAllImages() 
		{
			float ratio = 250.0f / width;
			ImVec2 imageDrawSize = ImVec2(width * ratio, height * ratio);

			int i = 0;

			for each (GLuint image in debug_texClr)
			{
				ImGui::Image((ImTextureID)image, imageDrawSize, ImVec2(0, 1), ImVec2(1, 0));
				
				i++;
			}
		}

	private:
		unsigned int width, height;

		int textureIndexOffset = 0;

		GLuint VAO;

		GLuint RBO[2];		// Render buffer object

		GLuint FBO[2];		// Framebuffers

		GLuint texClr[2];	// Colour attachments

		std::vector<GLuint> debug_texClr;	
		// Testing Colour attachments. For when theres more than 2 effects
	};

	PostProcessor::PostProcessor(EffectsList effects, unsigned int width, unsigned int height) :
		width(width), height(height)
	{
		this->effects = effects;

		textureIndexOffset = effects.size() - 1;

		// Ping Pong Setup
		for (int i = 0; i < 2; i++)
		{
			// FBO
			glGenFramebuffers(1, &FBO[i]);
			glBindFramebuffer(GL_FRAMEBUFFER, FBO[i]);

			// Colour Attachment
			glGenTextures(1, &texClr[i]);
			glActiveTexture(GL_TEXTURE0 + i + textureIndexOffset);
			glBindTexture(GL_TEXTURE_2D, texClr[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texClr[i], 0);

			// RBO : Used for depth and stencil buffers. tbh still not fully sure what thedeal is
			glGenRenderbuffers(1, &RBO[i]);
			glBindRenderbuffer(GL_RENDERBUFFER, RBO[i]);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO[i]);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glEnable(GL_DEPTH_TEST);

		// Framebuffer validation
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			printf("[Error] Framebuffer is incomplete");

		// Dummy VAO
		glCreateVertexArrays(1, &VAO);

		updateTextureIndex();
	}
	
	PostProcessor::~PostProcessor()
	{
		glDeleteVertexArrays(1, &VAO);

		glDeleteFramebuffers(1, &FBO[0]);
		glDeleteFramebuffers(1, &FBO[1]);

		glDeleteTextures(1, &texClr[0]);
		glDeleteTextures(1, &texClr[1]);

		glDeleteRenderbuffers(1, &RBO[0]);
		glDeleteRenderbuffers(1, &RBO[1]);

		for each (auto var in effects)
			delete(var);
	}

	void PostProcessor::preRender()
	{
		for (int i = 0; i < 2; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, FBO[i]);
			glClearColor(0.6f, 0.8f, 0.92f, 1.0f); // Literally random numbers
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		// Go back to first FB to draw the scene
		glBindFramebuffer(GL_FRAMEBUFFER, FBO[0]);
	}


	// Ping Pong approach heavily inspiried by:
	// https://research.ncl.ac.uk/game/mastersdegree/graphicsforgames/postprocessing/Tutorial%2010%20-%20Post%20Processing.pdf
	// https://github.com/thoxey/ezPP/tree/master
	// https://www.reddit.com/r/opengl/comments/patqpn/can_i_apply_the_same_post_processing_multiple/

	void PostProcessor::render(GLuint frameBuffer) 
	{
		debug_texClr.clear();

		// Pre Render Setup
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glBindVertexArray(VAO);

		//------------------------------------------------------*
		//		   _											|
		//	      (_)											|
		//	 _ __  _ _ __   __ _ _ __   ___  _ __   __ _		|
		//	| '_ \| | '_ \ / _` | '_ \ / _ \| '_ \ / _` |		|
		//	| |_) | | | | | (_| | |_) | (_) | | | | (_| |		|
		//	| .__/|_|_| |_|\__, | .__/ \___/|_| |_|\__, |		|
		//	| |             __/ | |                 __/ |		|
		//	|_|            |___/|_|                |___/		|
		//														|
		//------------------------------------------------------*
		// class starts in 7 hours 

		// Ping Ponging
		bool pingPong = false;
		for (PostProcessEffect* effect : effects) {
			glClearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red ig idk

			// Bind opposite FB and Texture to each other
			glBindFramebuffer(GL_FRAMEBUFFER, FBO[pingPong]);

			// Draw
			effect->renderToTexture(texClr[!pingPong]);

			// Debugging
			debug_texClr.push_back(texClr[!pingPong]);

			pingPong = !pingPong;
		}
	}
	
	void PostProcessor::draw(GLuint frameBuffer) {

		// To FB to display
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		glDrawArrays(GL_TRIANGLES, 0, 3);

		// Reenable tests
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
	}

}

