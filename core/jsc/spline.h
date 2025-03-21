#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>
#include <imgui.h>

#include "../ew/model.h"
#include "ew/transform.h"
#include "ew/shader.h"
#include "ew/camera.h"

#include "../jsc/animation.h"


namespace jsc {
	typedef std::vector<ew::Transform> Points;

	struct Spline
	{
		Points points;
		Points controlPoints;

		glm::vec3 clr = glm::vec3(.7, .2, .2);
		float width = 5;
		int subdivs = 10;

		ew::Shader lineShader, pointShader;
		ew::Mesh pointMesh, ctrlPointMesh;

		// The points positions stored in 1 list. Easier to use when drawing
		// will need to update the SetVertices and this to draw curved lines
		std::vector<float> verts;

		GLuint VAO, VBO;

		Spline(ew::Shader lineShader, ew::Shader pointShader) 
			: lineShader(lineShader), pointShader(pointShader)
		{
			pointMesh = ew::Mesh(ew::createSphere(.1, 8));
			ctrlPointMesh = ew::Mesh(ew::createSphere(.05, 8));
		}

		ew::Transform getValue(float t) {
			ew::Transform value;
			int index = 0;

			while (t > 1) {
				t--;
				index++;
			}

			/*
			int index = 0;
			float tempT = t;
			float incrementSize = (1 / (float)points.size());
			while (tempT >= incrementSize) {
				index++;
				tempT -= incrementSize;
			}
			*/

			ew::Transform first = points[index], second = points[index + 1],
				ctrl1 = controlPoints[index], ctrl2 = controlPoints[index + 1];

			value.position = cubicBezierLerp(first.position, ctrl1.position, ctrl2.position, second.position, t);
			return value;
		}

		void addPoint(ew::Transform newPoint) {
			points.push_back(newPoint);

			// TODO Add new control points in a smarter way
			ew::Transform newCtrl = newPoint;
			newCtrl.position.y += 2;
			controlPoints.push_back(newCtrl);
		}

		void removeLastPoint() {
			points.pop_back();
			controlPoints.pop_back();
		}

		void refresh() {
			setVertices();
			refreshControls();

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
			// Draw lines
			lineShader.use();
			lineShader.setMat4("_ViewProjection", cam.projectionMatrix() * cam.viewMatrix());
			lineShader.setVec3("_Color", clr);

			glBindVertexArray(VAO);
			glLineWidth(width);
			glDrawArrays(GL_LINE_STRIP, 0, verts.size() / 3);

			// Draw points
			pointShader.use();
			pointShader.setMat4("_ViewProjection", cam.projectionMatrix() * cam.viewMatrix());
			pointShader.setVec3("_Color", clr);

			// Control Points
			for each(ew::Transform transform in controlPoints) {
				pointShader.setMat4("_Model", transform.modelMatrix());
				ctrlPointMesh.draw();
			}
			
			// Points
			for each(ew::Transform transform in points) {
				pointShader.setMat4("_Model", transform.modelMatrix());
				pointMesh.draw();
			}
		}

		// Will need to update this to work with multiple splines
		void showUI() {
			if (ImGui::CollapsingHeader("A Spline")) {
				ImGui::Text("Spline Info Here");
				
				ImGui::ColorEdit3("Spline CLr", &clr.x);
				ImGui::DragFloat("Width", &width, .2, 0, 10);
				if (ImGui::SliderInt("Subdivs", &subdivs, 1, 50)) {
					refresh();
				}

				// Temp
				int ID = 0;
				for (ID = 0; ID < points.size(); ID++)
				{
					ImGui::PushID(ID);
					ImGui::Indent();

					drawSplineTransformUI(points[ID]);

					ImGui::Dummy(ImVec2(0, 5));
					ImGui::Unindent();
					ImGui::PopID();
				}

				// Temp
				ImGui::Text("Control Points");
				for (int i = 0; i < controlPoints.size(); i++)
				{
					ImGui::PushID(i + ID);
					ImGui::Indent();

					drawSplineControlTransformUI(controlPoints[i]);

					ImGui::Dummy(ImVec2(0, 5));
					ImGui::Unindent();
					ImGui::PopID();
				}

				if (ImGui::Button("Add Point")) {
					addPoint(ew::Transform());
					refresh();
				}

				if (ImGui::Button("Remove Point")) {
					removeLastPoint();
					refresh();
				}

			}
		}

	private:
		// Forward will be Z pos
		void refreshControls() {
			//for (int i = 0; i < controlPoints.size(); i++)
			//{
			//	ew::Transform ctrl = controlPoints[i];
			//	ew::Transform first = points[i];

			//	glm::mat4 rotMat = glm::toMat4(first.rotation);

			//	ctrl.position = first.position + glm::vec3(rotMat * glm::vec4(first.scale, 0.0));
			//}
		}


		/// <summary>
		/// Helper function. Called when updating points
		/// </summary>
		void setVertices() {
			verts.clear();

			// Set Verts for spline segments
			for (int i = 0; i < points.size() - 1; i++) 
			{
				ew::Transform first = points[i], second = points[i + 1],
					ctrl1 = controlPoints[i], ctrl2 = controlPoints[i + 1];

				// First Vert
				verts.push_back(first.position.x);
				verts.push_back(first.position.y);
				verts.push_back(first.position.z);

				// Subdivisions
				float lerpIncrement = 1 / (float)subdivs;
				for (float lerpK = lerpIncrement; lerpK < 1; lerpK += lerpIncrement)
				{
					verts.push_back(cubicBezierLerp(first.position.x, ctrl1.position.x, ctrl2.position.x, second.position.x, lerpK));
					verts.push_back(cubicBezierLerp(first.position.y, ctrl1.position.y, ctrl2.position.y, second.position.y, lerpK));
					verts.push_back(cubicBezierLerp(first.position.z, ctrl1.position.z, ctrl2.position.z, second.position.z, lerpK));
				}

				// Last Vert
				verts.push_back(second.position.x);
				verts.push_back(second.position.y);
				verts.push_back(second.position.z);
			}
		}

		// shhh Ill make ui functions more consitent later
		void drawSplineTransformUI(ew::Transform& transform) {
			if (ImGui::DragFloat3("Position", &transform.position.x, .05f, -10.0f, 10.0f) ||
				ImGui::DragFloat4("Rotation", &transform.rotation.x, .05f, -10.0f, 10.0f) ||
				ImGui::DragFloat3("Scale", &transform.scale.x, .05f, -10.0f, 10.0f)) 
			{
				refresh();
			}

		}

		void drawSplineControlTransformUI(ew::Transform& transform) {
			if (ImGui::DragFloat3("Position", &transform.position.x, .05f, -10.0f, 10.0f))
			{
				refresh();
			}
		}
	};
}