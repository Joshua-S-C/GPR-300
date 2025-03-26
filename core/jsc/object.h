#include <string>
#include <imgui.h>

namespace jsc {
	struct Object
	{
		std::string name;

		virtual void drawSceneUI() = 0;
		virtual void drawInspectorUI() = 0;
	};

}