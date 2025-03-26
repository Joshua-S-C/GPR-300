#include <string>
#include <imgui.h>
#include "../ew/transform.h"

namespace jsc {
	struct Object
	{
		std::string name;
		ew::Transform transform;

		virtual void drawSceneUI() = 0;
		virtual void drawInspectorUI() = 0;
	};

}