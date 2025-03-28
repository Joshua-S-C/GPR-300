#include <jsc/object.h>
#include <vector>

namespace jsc {

	/// <summary>
	/// Joint and Skeleton represent the hierarchy structure, but do not hold poses directly. 
	/// </summary>
	struct Joint : public Object {
		Joint* parent = nullptr;
		std::vector<Joint*> children;

		glm::mat4 localTransform, globalTransform;

		Joint() = default;
		Joint(std::string name) {
			this->name = name;
		};

		bool setParent(Joint* newParent) {
			if (newParent == this)
				return false;

			if (parent != nullptr)
				parent->removeChild(this);

			parent = newParent;
			newParent->children.push_back(this);
		}

		bool removeChild(Joint* newParent) {
			if (std::find(children.begin(), children.end(), newParent) != children.end())
				return false;
			
			children.erase(std::remove(children.begin(), children.end(), newParent), children.end());

			return true;

		}

		void solveFK() {
			localTransform = transform.modelMatrix();

			if (this->parent == nullptr)
				globalTransform = localTransform;
			else
				this->globalTransform = parent->globalTransform * localTransform;
				for each(Joint* child in children)
					child->solveFK();

		}

		/// <summary>
		/// Used to quickly add children to an Object vector
		/// </summary>
		/// <returns></returns>
		std::vector<Object*> getAllChildren() {
			std::vector<Object*> allKids;

			for each(Joint * obj in children)
			{
				allKids.push_back(obj);
				std::vector<Object*> kids = (obj->getAllChildren());
				allKids.insert(allKids.end(), kids.begin(), kids.end());
			}

			return allKids;
		}

		void drawSceneUI() {
			ImGuiTreeNodeFlags flag = ImGuiTreeNodeFlags_DefaultOpen;
			if (children.empty())
				flag |= ImGuiTreeNodeFlags_Leaf;

			if (ImGui::TreeNodeEx(name.c_str(), flag))
			{
				if (ImGui::IsItemClicked())
					clicked = true;

				for each(Joint* j in children)
					j->drawSceneUI();

				ImGui::TreePop();
			}
		};

		void drawInspectorUI() {
			ImGui::Text(("Joint : " + name).c_str());
			transform.drawInspectorUI();
		};
	};

	struct Skeleton : public Object {
		std::vector<Joint*> joints;

		void solveFK() {
			for each(Joint* joint in joints)
			{
				if (joint->children.empty())
					joint->globalTransform = joint->localTransform;
				else
					joint->solveFK();
			}
		}

		/// <summary>
		/// Used to quickly add children to an Object vector
		/// </summary>
		/// <returns></returns>
		std::vector<Object*> getAllChildren() {
			std::vector<Object*> allKids;

			for each(Joint* obj in joints)
			{
				allKids.push_back(obj);
				std::vector<Object*> kids = (obj->getAllChildren());
				allKids.insert(allKids.end(), kids.begin(), kids.end());
			}

			return allKids;
		}

		void drawSceneUI() {
			ImGuiTreeNodeFlags flag = ImGuiTreeNodeFlags_DefaultOpen;
			if (ImGui::TreeNodeEx(name.c_str(), flag))
			{
				if (ImGui::IsItemClicked())
					clicked = true;

				for each(Joint* j in joints)
					j->drawSceneUI();

				ImGui::TreePop(); 
			}
		};

		void drawInspectorUI() {
			ImGui::Text(("Skeleton : " + name).c_str());
		};
	};

	/// <summary>
	/// Represents the local transformation of a single joint
	/// Contains a transform
	/// </summary>
	struct JointPose : public Object {
		void drawInspectorUI() {

		};
	};

	/// <summary>
	///  Represents a pose for an entire skeleton
	/// </summary>
	struct SkeletonPose : public Object {
		Skeleton* skeleton;
		std::vector<JointPose*> localPoses;
		glm::mat4x4* globalPose; //Global joint poses. This is what is calculated using FK!

		void drawInspectorUI() {
			
		};

	};

}