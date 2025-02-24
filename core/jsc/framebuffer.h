#include <glm/glm.hpp>


namespace jsc {
	struct Framebuffer {
		GLuint fbo;
		GLuint width, height;

		// TODO : Add Colour and use to replace in Post Processor

		GLuint depthBuffer;

		static Framebuffer createFramebufferDepth(GLuint width, GLuint height) {
			Framebuffer fb;
		
			// Depth buffer creation
			glGenFramebuffers(1, &fb.fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo);

			// Depth texture creation
			glGenTextures(1, &fb.depthBuffer);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, fb.depthBuffer);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
				width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

			// Attach them
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fb.depthBuffer, 0);
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				printf("[Error] Framebuffer is not complete\n");

			return fb;
		}
	};

}