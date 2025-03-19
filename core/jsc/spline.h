#include <glm/glm.hpp>
#include <vector>
#include <imgui.h>

#include "ew/transform.h"
#include "ew/shader.h"
#include "ew/camera.h"

namespace jsc {
	typedef std::vector<ew::Transform> Points;

	struct Spline
	{
		Points points;
		glm::vec3 clr = glm::vec3(.5, .5, .5);
		float width = 5;

		ew::Shader shader;
		GLuint VAO, VBO;

		// The points positions stored in 1 list. Easier to use when drawing
		// will need to update the SetVertices and this to draw curved lines
		std::vector<float> verts;

		Spline(ew::Shader shader) 
			: shader(shader)
		{}

		void addPoint(ew::Transform newPoint) {
			points.push_back(newPoint);
		}

		void refresh() {
			setVertices();

			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);
			glBindVertexArray(VAO);

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(verts) * verts.size(), verts.data(), GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}

		void draw(ew::Camera cam) {
			shader.use();
			shader.setMat4("_ViewProjection", cam.projectionMatrix() * cam.viewMatrix());
			shader.setVec3("_Color", clr);

			glBindVertexArray(VAO);
			glLineWidth(width);
			glDrawArrays(GL_LINE_STRIP, 0, verts.size() / 3);
		}

		// Will need to update this to work with multiple splines
		void showUI() {
			if (ImGui::CollapsingHeader("A Spline")) {
				ImGui::Text("Spline Info Here");
				
				ImGui::ColorEdit3("Spline CLr", &clr.x);
				ImGui::DragFloat("Width", &width, .2, 0, 10);

				// Temp
				for (int i = 0; i < points.size(); i++)
				{
					ImGui::PushID(i);
					ImGui::Indent();

					drawSplineTransformUI(points[i]);

					ImGui::Dummy(ImVec2(0, 2));
					ImGui::Unindent();
					ImGui::PopID();
				}

				if (ImGui::Button("Add Point")) {
					addPoint(ew::Transform());
					refresh();
				}

			}
		}

	private:
		/// <summary>
		/// Helper function. Called when updating points
		/// </summary>
		void setVertices() {
			verts.clear();

			for each(ew::Transform transform in points)
			{
				verts.push_back(transform.position.x);
				verts.push_back(transform.position.y);
				verts.push_back(transform.position.z);
			}
		}

		// shhh Ill make ui functions more consitent later
		void drawSplineTransformUI(ew::Transform& transform) {
			if (ImGui::DragFloat3("Position", &transform.position.x, .05f, -10.0f, 10.0f)) {
				refresh();
			}
			ImGui::DragFloat4("Rotation", &transform.rotation.x, .05f, -10.0f, 10.0f);
			ImGui::DragFloat3("Scale", &transform.scale.x, .05f, -10.0f, 10.0f);
		}
	};
}