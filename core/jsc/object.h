#ifndef OBJECT_H 
#define OBJECT_H

#include <string>
#include <imgui.h>
#include "../ew/transform.h"

namespace jsc {
	struct Object
	{
		std::string name = "[Blank Name]";
		ew::Transform transform;

		bool clicked = false;

		Object() = default;

		Object(std::string name) {
			this->name = name;
		};

		/// <summary>
		/// Sets click to false
		/// </summary>
		/// <returns>True if Object was clicked before</returns>
		virtual bool unClick() {
			if (!clicked)
				return false;

			clicked = false;
			return true;
		};

		virtual void drawSceneUI() {
			ImGui::Text(name.c_str());
		};

		virtual void drawInspectorUI() = 0;

	};

}

#endif