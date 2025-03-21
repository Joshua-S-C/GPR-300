#include <math.h>

/// Easing functions from https://easings.net/

namespace jsc {
	
	template<class T>
	T lerp(T a, T b, float t) {
		return a + (b - a) * t;
	}
	
	template<class T>
	T inverseLerp(T a, T b, float v) {
		return (v - a) / (b - a);
	}

	template<class T>
	T cubicBezierLerp(T a, T b, T c, T d, float v) {

		// Same thing
		T l1 = lerp(a, b, v);
		T l2 = lerp(b, c, v);
		T l3 = lerp(c, d, v);

		T l5 = lerp(l1, l2, v);
		T l6 = lerp(l2, l3, v);

		return lerp(l5, l6, v);

		/* I had this before, but it doesn't work with vec3s
		return
			pow((1 - v), 3) * a +
			3 * v * pow((1 - v), 2) * b +
			3 * (v * v) * (1 - v) * c
			+ pow(v, 3) * d;
		*/
	}

	const float PI = 3.141592;

	float easeNone(float t) { return t; }

	float easeInSine(float t) { return 1 - cos((t * PI) / 2); }

	float easeOutBounce(float t) {
		const float n1 = 7.5625;
		const float d1 = 2.75;

		if (t < 1 / d1) {
			return n1 * t * t;
		}
		else if (t < 2 / d1) {
			return n1 * (t -= 1.5 / d1) * t + 0.75;
		}
		else if (t < 2.5 / d1) {
			return n1 * (t -= 2.25 / d1) * t + 0.9375;
		}
		else {
			return n1 * (t -= 2.625 / d1) * t + 0.984375;
		}
	}

	float easeInOutElastic(float t) {
		const float c5 = (2 * PI) / 4.5;

		return 
		(t == 0) ? 
			0 : 
			(t == 1) ? 
				1 : 
				(t < 0.5) ? 
				-(pow(2, 20 * t - 10) * sin((20 * t - 11.125) * c5)) / 2 : 
				(pow(2, -20 * t + 10) * sin((20 * t - 11.125) * c5)) / 2 + 1;
	}


	typedef float EasingFunction(float);

	/// <summary>
	/// Call associated easing function
	/// </summary>
	float ease(float t, float (*f)(float)) {
		return f(t);
	}

	const enum EasingType {
		None = 0,
		InSine,
		OutBounce,
		InOutElastic,
	};

	const char* EasingNames[4] = {
		"None",
		"Ease In Sine",
		"Ease Out Bounce"
		"Ease In Out Elastic"
	};

	/// <summary>
	/// Easing Type is index
	/// </summary>
	const std::vector<EasingFunction*> AllEasingFuncs = {
		easeNone, easeInSine, easeOutBounce, easeInOutElastic
	};

}