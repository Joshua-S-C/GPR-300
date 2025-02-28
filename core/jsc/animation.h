#include <glm/glm.hpp>
#include <imgui.h>

namespace jsc {
	// TODO : Template this
	//template <typename T> 
	struct KeyFrame {
		float time;
		//T value;
		glm::vec3 value;
	};

	typedef std::vector<jsc::KeyFrame> Keys;

	struct Animator {
		AnimationClip* clip;

		bool isPlaying;
		bool isLooping;

		float playbackSpd = 1;
		float playbackTime;

		bool update(float deltaTime) {
			if (!isPlaying)
				return false;

			playbackTime += deltaTime;


			if (playbackTime > clip->duration)
				if (isLooping)
					playbackTime = (playbackTime + deltaTime) - clip->duration;
				else
					playbackTime = clip->duration;

			return true;
		}

		void showUI() {
			if (ImGui::CollapsingHeader("Animator")) {
				ImGui::Indent();

				ImGui::Checkbox("Playing", &isPlaying);
				ImGui::Checkbox("Looping", &isLooping);

				ImGui::DragFloat("Playback Speed", &playbackSpd);
				ImGui::SliderFloat("Playback Time", &playbackTime, 0, clip->duration);
				ImGui::DragFloat("Duration", &clip->duration);
			}
		}
	};

	struct AnimationClip {
		float duration;
		
		Keys posKeys, rotKeys, scaleKeys;

	};





}