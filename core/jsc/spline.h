#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>
#include <imgui.h>

#include "../ew/model.h"
#include "../ew/transform.h"
#include "../ew/shader.h"
#include "../ew/camera.h"

#include "../jsc/animation.h"

#include "../jsc/object.h"


namespace jsc {
	typedef std::vector<ew::Transform> Points;

	/// <summary>
	/// Helper class for splines
	/// </summary>
	struct ControlPoint 
	{
		ControlPoint(ew::Transform transform, ew::Transform* parent, bool opposite = false)
			:transform(transform), parent(parent), opposite(opposite)
		{}

		ew::Transform transform;
		ew::Transform* parent;
		bool opposite = false;
	};
	typedef std::vector<ControlPoint> ControlPoints;

	struct Spline : public Object
	{
		Points points;
		ControlPoints controlPoints;

		glm::vec3 clr = glm::vec3(.7, .2, .2);
		float width = 5;
		int subdivs = 10;

		ew::Shader lineShader, pointShader;
		ew::Mesh pointMesh, ctrlPointMesh;

		// The points positions stored in 1 list. Easier to use when drawing
		// will need to update the SetVertices and this to draw curved lines
		std::vector<float> verts;

		GLuint VAO, VBO;

		GLuint velVAO, velVBO;


		Spline(ew::Shader lineShader, ew::Shader pointShader, std::string newName) 
			: lineShader(lineShader), pointShader(pointShader)
		{
			pointMesh = ew::Mesh(ew::createSphere(.1, 8));
			ctrlPointMesh = ew::Mesh(ew::createSphere(.05, 8));

			name = newName;
		}

		ew::Transform getValue(float t) {
			ew::Transform value;
			int index = 0;

			while (t > 1) {
				t--;
				index++;
			}

			// Out of range
			if (index > points.size() - 2) {
				value.position = points.back().position;
				value.rotation = points.back().rotation;
				
				return value;
			}

			ew::Transform first = points[index], second = points[index + 1],
				ctrl1 = controlPoints[index].transform, ctrl2 = controlPoints[index + 1].transform;

			value.position = cubicBezier(first.position, ctrl1.position, ctrl2.position, second.position, t);

			value.rotation = glm::quat(
				glm::normalize(cubicBezierDeriv(first.position, ctrl1.position, ctrl2.position, second.position, t))
			);

			return value;
		}

		void addPoint(ew::Transform newPoint) {
			points.push_back(newPoint);

			// TODO Add new control points in a smarter way
			ew::Transform newCtrl = newPoint;
			controlPoints.push_back(ControlPoint(newCtrl, &newPoint));

			newCtrl.position = newPoint.position + glm::eulerAngles(newPoint.rotation) + newPoint.scale;

			refresh();
		}

		void removeLastPoint() {
			if (points.size() <= 2)
				return;
			
			points.pop_back();
			controlPoints.pop_back();

			refresh();
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
			for (int i = 0; i < controlPoints.size(); i++) {
				ControlPoint ctrl = controlPoints[i];

				// Forward Control Point
				pointShader.setMat4("_Model", ctrl.transform.modelMatrix());
				ctrlPointMesh.draw();

				// Backwards Control Point
				if (i != 0 && i != controlPoints.size() - 1) {
					ew::Transform tempTransform = ctrl.transform;
					glm::vec3 c2p = (points[i].position - tempTransform.position);
					tempTransform.position += c2p * 2.0f;

					pointShader.setMat4("_Model", tempTransform.modelMatrix());
					ctrlPointMesh.draw();
				}
			}
			
			// Points
			for each(ew::Transform transform in points) {
				glm::mat4 posMatrix = glm::translate(glm::mat4(1.0f), transform.position);
				pointShader.setMat4("_Model", posMatrix);
				pointMesh.draw();
			}
		}

		void debugDrawVelocity(ew::Camera cam, float t) {
			pointShader.use();
			pointShader.setMat4("_ViewProjection", cam.projectionMatrix() * cam.viewMatrix());
			pointShader.setVec3("_Color", glm::vec3(.1, 1, .1));


			ew::Transform vel = getValue(t);
			vel.position += glm::eulerAngles(vel.rotation);
			pointShader.setMat4("_Model", vel.modelMatrix());

			ctrlPointMesh.draw();

			return;
		}

		void drawSceneUI() {
			ImGui::Text("This a splin");
			ImGui::Text(name.c_str());
		}

		// Will need to update this to work with multiple splines
		void drawInspectorUI() {
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

				if (ImGui::Button("Add Point"))
					addPoint(ew::Transform(
						points.back().position + glm::vec3(1, 0, 0),
						glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
						glm::vec3(1, 1, 1)
					));

				if (ImGui::Button("Remove Point"))
					removeLastPoint();

			}
		}

	private:
		void refreshControls() {
			for (int i = 0; i < controlPoints.size(); i++)
			{
				ew::Transform* ctrl = &controlPoints[i].transform;
				ew::Transform* point = &points[i];

				glm::mat4 rotMat = glm::toMat4(glm::quat(point->eulerRot));
				ctrl->position = point->position + glm::vec3(rotMat * glm::vec4(point->scale, 0.0));
			}
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
					ctrl1 = controlPoints[i].transform, ctrl2 = controlPoints[i + 1].transform;

				// First Vert
				verts.push_back(first.position.x);
				verts.push_back(first.position.y);
				verts.push_back(first.position.z);

				// Subdivisions
				float lerpIncrement = 1 / (float)subdivs;
				for (float lerpK = lerpIncrement; lerpK < 1; lerpK += lerpIncrement)
				{
					verts.push_back(cubicBezier(first.position.x, ctrl1.position.x, ctrl2.position.x, second.position.x, lerpK));
					verts.push_back(cubicBezier(first.position.y, ctrl1.position.y, ctrl2.position.y, second.position.y, lerpK));
					verts.push_back(cubicBezier(first.position.z, ctrl1.position.z, ctrl2.position.z, second.position.z, lerpK));
				}

				// Last Vert
				verts.push_back(second.position.x);
				verts.push_back(second.position.y);
				verts.push_back(second.position.z);
			}
		}

		void drawSplineTransformUI(ew::Transform& transform) {
			//ImGui::Text((
			//	"Quat: " +
			//	std::to_string(transform.rotation.x) + "," +
			//	std::to_string(transform.rotation.y) + "," +
			//	std::to_string(transform.rotation.z) + "," +
			//	std::to_string(transform.rotation.w) + ","
			//).c_str());
			//ImGui::Text((
			//	"Euler" + 
			//	std::to_string(transform.eulerRot.x) + "," +
			//	std::to_string(transform.eulerRot.y) + "," +
			//	std::to_string(transform.eulerRot.z) + ","
			//).c_str());

			if (ImGui::DragFloat3("Position", &transform.position.x, .05f, -10.0f, 10.0f) ||
				ImGui::DragFloat2("Rotation", &transform.eulerRot.x, .05f, 0.0f, 6.28f) ||
				ImGui::DragFloat("Scale", &transform.scale.x, .05f, -10.0f, 10.0f))
			{
				transform.rotation = glm::quat(transform.eulerRot);
				transform.scale.y = transform.scale.x;
				transform.scale.z = transform.scale.x;

				refresh();
			}
		}

};
}