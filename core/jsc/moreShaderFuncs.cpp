#include "../ew/shader.h"
#include "../jsc/light.h"

namespace ew {
	// Added
	void Shader::setBool(const std::string& name, bool v) const
	{
		glUniform1i(glGetUniformLocation(m_id, name.c_str()), v);
	}

	// Added
	void Shader::setMaterial(const std::string& name, jsc::Material v) const
	{
		glUniform1f(glGetUniformLocation(m_id, (name + ".aK").c_str()), v.ambientK);
		glUniform1f(glGetUniformLocation(m_id, (name + ".dK").c_str()), v.diffuseK);
		glUniform1f(glGetUniformLocation(m_id, (name + ".sK").c_str()), v.specularK);
		glUniform1f(glGetUniformLocation(m_id, (name + ".shininess").c_str()), v.shininess);
	}

	// Added
	void Shader::setLight(const std::string& name, jsc::Light light) const
	{
		setVec3(name + ".pos", light.transform.position);
		setVec3(name + ".clr", light.clr);
	}
}