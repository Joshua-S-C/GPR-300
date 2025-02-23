#pragma once

#include "../ew/external/glad.h"
#include "../ew/transform.h"
#include "../ew/mesh.h"
#include "../ew/texture.h"
#include "../ew/shader.h"
#include "../ew/procGen.h"

#include <imgui.h>

namespace jsc {
	struct Light {
		ew::Transform transform; // World Space
		glm::vec3 clr; // RBG

		Light() {}

		Light(glm::vec3 clr) {
			this->clr = clr;
		}

		virtual void drawUI() {
			if (ImGui::CollapsingHeader("Light")) {
				ImGui::DragFloat3("Position", &transform.position.x, .05f, -10.0f, 10.0f);
				ImGui::ColorEdit3("Colour", &clr.x);
			}
		}
	};

	struct DirectionalLight : Light{
		glm::vec3 dir;

		DirectionalLight(glm::vec3 dir, glm::vec3 clr) {
			this->dir = dir;
			this->clr = clr;
		}

		void drawUI() {
			if (ImGui::CollapsingHeader("Directional Light")) {
				ImGui::SliderFloat3("Direction", &dir.x, -1.0f, 1.0f);
				ImGui::DragFloat3("Position", &transform.position.x, .05f, -10.0f, 10.0f);
				ImGui::ColorEdit3("Colour", &clr.x);
			}
		}
	};

	struct Material {
		// Coefficients 0-1
		float ambientK = .05;
		float diffuseK = .2;
		float specularK = .7;

		float shininess = 32;

		Material(float ambient, float diffuse, float specular, float _shininess) {
			ambientK = ambient;
			diffuseK = diffuse;
			specularK = specular;
			shininess = _shininess;
		};

		void drawUI() {
			if (ImGui::CollapsingHeader("Material")) {
				ImGui::SliderFloat("AmbientK", &ambientK, 0.0f, 1.0f);
				ImGui::SliderFloat("DiffuseK", &diffuseK, 0.0f, 1.0f);
				ImGui::SliderFloat("SpecularK", &specularK, 0.0f, 1.0f);
				ImGui::SliderFloat("Shininess", &shininess, 2.0f, 32.0f);
			}
		}
	};
}