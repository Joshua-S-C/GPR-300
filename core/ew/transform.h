/*
*	Author: Eric Winebrenner
*/

#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>

namespace ew {
	struct Transform {
		Transform() = default;

		Transform(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale)
			: position(position), rotation(rotation), scale(scale)
		{}
		
		glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f,0.0f);
		glm::vec3 eulerRot = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);

		glm::mat4 modelMatrix() const {
			glm::mat4 m = glm::mat4(1.0f);
			m = glm::translate(m, position);
			m *= glm::mat4_cast(rotation);
			m = glm::scale(m, scale);
			return m;
		}

		glm::mat4 modelMatrixEuler() const {
			glm::mat4 m = glm::mat4(1.0f);
			m = glm::translate(m, position);
			m *= glm::mat4_cast(glm::quat(eulerRot));
			m = glm::scale(m, scale);
			return m;
		}

		void drawInspectorUI() {
			ImGui::DragFloat3("Position", &position.x, .05f, -10.0f, 10.0f);
			//ImGui::DragFloat4("Rotation", &rotation.x, .05f, -10.0f, 10.0f);
			ImGui::DragFloat3("E Rotation", &eulerRot.x, .05f, -10.0f, 10.0f);
			ImGui::DragFloat3("Scale", &scale.x, .05f, -10.0f, 10.0f);
		}
	};
}
