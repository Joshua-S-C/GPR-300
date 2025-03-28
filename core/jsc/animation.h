#include <glm/glm.hpp>
#include <imgui.h>
#include <vector>
#include "../jsc/easing.h"

namespace jsc {
	// TODO : Update UI based on type
	template <typename T> 
	struct KeyFrame {
		float time;
		T value;
		int easeType = 0;

		KeyFrame() {
			time = -1;
		}

		KeyFrame(float time, T value) {
			this->time = time;
			this->value = value;
		}

		bool operator==(const KeyFrame& other) {
			return (this->time == other.time) && (this->value == other.value);
		}
	};

	typedef std::vector<jsc::KeyFrame<glm::vec3>> KeysVec3;

	struct AnimationClip {
		float duration;

		KeysVec3 posKeys, rotKeys, scaleKeys;

		//bool compareKeyTimes(KeyFrame<glm::vec3> frame1, KeyFrame<glm::vec3> frame2) {
		//	return frame1.time > frame2.time;
		//}

		void showUI() {
			if (ImGui::CollapsingHeader("Position Keys")) {
				for (int i = 0; i < posKeys.size(); i++)
				{
						ImGui::PushID(i);
						ImGui::Indent();

						ImGui::DragFloat("Time Pos", &posKeys[i].time, 0.1f);
						ImGui::DragFloat3("Value Pos", &posKeys[i].value.x, 0.1f);
						ImGui::Combo("Ease Function Pos", &posKeys[i].easeType, EasingNames, AllEasingFuncs.size());

						//if (ImGui::Button("Remove")) {
						//	posKeys.erase(posKeys.begin() + i);
						//	//std::sort(posKeys.begin(), posKeys.end(), compareKeyTimes);
						//}
						ImGui::Dummy(ImVec2(0, 2));
					
						ImGui::Unindent();
						ImGui::PopID();
				}

				if (ImGui::Button("Remove Pos")) {
					if (!posKeys.empty())
						posKeys.erase(std::find(posKeys.begin(), posKeys.end(), posKeys.back()));
				}

				if (ImGui::Button("Add Pos")) {
					posKeys.push_back(KeyFrame<glm::vec3>(-1, glm::vec3(0,0,0)));
				}
			}

			if (ImGui::CollapsingHeader("Scale Keys")) {
				for (int i = 0; i < scaleKeys.size(); i++)
				{
					ImGui::PushID(i);
					ImGui::Indent();

					ImGui::DragFloat("Time Scl", &scaleKeys[i].time, 0.1f);
					ImGui::DragFloat3("Value Scl", &scaleKeys[i].value.x, 0.1f);
					ImGui::Combo("Ease Function Pos Scl", &scaleKeys[i].easeType, EasingNames, AllEasingFuncs.size());


					//if (ImGui::Button("Remove")) {
					//	scaleKeys.erase(scaleKeys.begin() + i);
					//}

					ImGui::Dummy(ImVec2(0, 2));
					ImGui::Unindent();
					ImGui::PopID();
				}

				if (ImGui::Button("Remove Scale")) {
					if (!scaleKeys.empty())
						scaleKeys.erase(std::find(scaleKeys.begin(), scaleKeys.end(), scaleKeys.back()));
				}

				if (ImGui::Button("Add Scale")) {
					scaleKeys.push_back(KeyFrame<glm::vec3>(-1, glm::vec3(0, 0, 0)));
				}
			}

			if (ImGui::CollapsingHeader("Rotation Keys")) {
				for (int i = 0; i < rotKeys.size(); i++)
				{
					ImGui::PushID(i);
					ImGui::Indent();

					ImGui::DragFloat("Time", &rotKeys[i].time, 0.1f);
					ImGui::DragFloat3("Value", &rotKeys[i].value.x, 0.1f);
					ImGui::Combo("Ease Function Pos Rot", &rotKeys[i].easeType, EasingNames, AllEasingFuncs.size());

					//if (ImGui::Button("Remove")) {
					//	rotKeys.erase(rotKeys.begin() + i);
					//}

					ImGui::Dummy(ImVec2(0, 2));
					ImGui::Unindent();
					ImGui::PopID();
				}

				if (ImGui::Button("Remove Rot")) {
					if (!rotKeys.empty())
						rotKeys.erase(std::find(rotKeys.begin(), rotKeys.end(), rotKeys.back()));
				}

				if (ImGui::Button("Add Rot")) {
					rotKeys.push_back(KeyFrame<glm::vec3>(-1, glm::vec3(0, 0, 0)));
				}
			}
		}

	};

	struct Animator {
		AnimationClip* clip;

		bool isPlaying = false;
		bool isLooping = true;
		bool infinity = true;

		float playbackSpd = 1;
		float playbackTime = 0;

		bool update(float deltaTime) {
			if (!isPlaying)
				return false;

			playbackTime += deltaTime * playbackSpd;

			// Looping Logic
			if (playbackTime > clip->duration)
				if (isLooping)
					playbackTime = (playbackTime + deltaTime) - clip->duration;
				else
					playbackTime = clip->duration;

			return true;
		}

		// TODO Template this
		/// <summary>
		/// 
		/// </summary>
		/// <param name="collection">To iterate thru. Will be the Keys</param>
		/// <param name="fallbackValue">Value to return if there are no keys</param>
		/// <returns>Lerped value</returns>
		glm::vec3 getValue(KeysVec3 collection, glm::vec3 fallbackValue) {
			// Debug
			//playbackTime = 10.0f;
			
			if (collection.empty())
				return fallbackValue;

			if (collection.size() == 1)
				return collection.front().value;

			// Get current keys
			KeyFrame<glm::vec3>* prevKey = nullptr;
			KeyFrame<glm::vec3>* nextKey = nullptr;
			int index = 0;

			// Upcoming Key
			for (index = 0; index < collection.size(); index++) {
				if (collection[index].time >= playbackTime) {
					nextKey = &collection[index];
					break;
				}
			}

			// Infinity: Lerp between last and first frame
			if (infinity && nextKey == nullptr) {
				nextKey = &collection.front();
				prevKey = &collection.back();

				// Value to lerp to
				float t = inverseLerp(prevKey->time, nextKey->time, (playbackTime - clip->duration));

				if (t > 1 && t < 2) {
					t--;

					printf(std::to_string(t).c_str());
					printf("\n");

					t = ease(t, AllEasingFuncs[prevKey->easeType]);
					return lerp(prevKey->value, nextKey->value, t);
				}

				// Remap into [0-1] range. 
				// Then count how many times its looped basically
				bool isEven = true;
				while (t > 1) {
					t -= 1;
					isEven = !isEven;
				}

				if (isEven) 
					t = 1 - t;

				t = ease(t, AllEasingFuncs[prevKey->easeType]);
				return lerp(prevKey->value, nextKey->value, t);
			}

			// Stay on last frame
			if (nextKey == nullptr) {
				return collection.back().value;
			}

			// Next Key exists
			prevKey = &collection[index - 1];
			
			// Value to lerp to
			float t = inverseLerp(prevKey->time, nextKey->time, playbackTime);
			t = ease(t, AllEasingFuncs[prevKey->easeType]);
			return lerp(prevKey->value, nextKey->value, t);
		}

		void showUI() {
			if (ImGui::CollapsingHeader("Animator")) {
				ImGui::Indent();

				ImGui::Checkbox("Playing", &isPlaying);
				ImGui::Checkbox("Looping", &isLooping);
				ImGui::Checkbox("Infinity", &infinity);

				ImGui::DragFloat("Playback Speed", &playbackSpd);
				ImGui::SliderFloat("Playback Time", &playbackTime, 0, clip->duration);
				ImGui::DragFloat("Duration", &clip->duration);

				ImGui::Unindent();
			}


			clip->showUI();

		}

	};







}