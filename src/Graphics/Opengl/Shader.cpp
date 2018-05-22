/*
 * mpUtils
 * Shader.cpp
 *
 * Contains the Shader class which is used to manage and compile openGL shader.
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Copyright (c) 2017 Hendrik Schwanekamp
 *
 * This file was originally written and generously provided for this framework from Johannes Braun.
 *
 */

#include "Shader.h"
#include "glm/ext.hpp"

namespace mpu {
namespace gph {

    namespace fs = std::experimental::filesystem;
    std::vector<fs::path> shader_include_paths;
    void addShaderIncludePath(std::experimental::filesystem::path include_path)
    {
        shader_include_paths.emplace_back(std::forward<std::experimental::filesystem::path>(include_path));
    }

	ShaderModule::ShaderModule(const ShaderStage stage, const fs::path path_to_file)
		: stage(stage), path_to_file(path_to_file)
	{
	}

	ShaderModule::ShaderModule(fs::path path_to_file)
		: ShaderModule([ext = path_to_file.extension()]()
	{
		if (ext == ".vert") return ShaderStage::eVertex;
		if (ext == ".frag") return ShaderStage::eFragment;
		if (ext == ".geom") return ShaderStage::eGeometry;
		if (ext == ".comp") return ShaderStage::eCompute;
		if (ext == ".tesc") return ShaderStage::eTessControl;
		if (ext == ".tese") return ShaderStage::eTessEvaluation;

        logERROR("ShaderModule") << "Wrong File extension. Should be one of .vert, .frag, .geom, .comp, .tesc or .tese.";
        throw std::runtime_error("File extension should be one of .vert, .frag, .geom, .comp, .tesc or .tese.");
	}(), path_to_file)
	{
	}

	ShaderProgram::ShaderProgram()
		: Handle(nullptr)
	{
	}

	ShaderProgram::ShaderProgram(nullptr_t)
		: Handle(nullptr)
	{

	}

	ShaderProgram::ShaderProgram(const ShaderModule& shader, const std::vector<glsl::Definition>& definitions)
		: Handle(nullptr)
	{
		rebuild(&shader, &shader + 1, definitions);
	}

	ShaderProgram::ShaderProgram(const std::initializer_list<const ShaderModule> shaders, const std::vector<glsl::Definition>& definitions)
		: Handle(nullptr)
	{
		rebuild(std::begin(shaders), std::end(shaders), definitions);
	}

	int ShaderProgram::attributeLocation(std::string_view attribute) const
	{
		return glGetProgramResourceLocation(*this, GL_PROGRAM_INPUT, attribute.data());
	}

	int ShaderProgram::uniformLocation(std::string_view uniform) const
	{
		return glGetProgramResourceLocation(*this, GL_UNIFORM, uniform.data());
	}

	int ShaderProgram::uniformBlock(std::string_view uniform) const
	{
		return glGetUniformBlockIndex(*this, uniform.data());
	}

	void ShaderProgram::rebuild(const ShaderModule& shader, const std::vector<glsl::Definition>& definitions)
	{
		rebuild(&shader, &shader + 1, definitions);
	}

	void ShaderProgram::rebuild(const std::initializer_list<const ShaderModule> shaders, const std::vector<glsl::Definition>& definitions)
	{
		rebuild(std::begin(shaders), std::end(shaders), definitions);
	}

	void ShaderProgram::use() const
	{
		glUseProgram(*this);
	}

	void ShaderProgram::dispatch(const uint32_t invocations, const uint32_t group_size) const
	{
		dispatch(glm::uvec3(invocations, 1, 1), glm::uvec3(group_size, 1, 1));
	}

	void ShaderProgram::dispatch(const glm::uvec2 invocations, const glm::uvec2 group_size) const
	{
		dispatch(glm::uvec3(invocations.x, invocations.y, 1), glm::uvec3(group_size.x, group_size.y, 1));
	}

	void ShaderProgram::dispatch(const glm::uvec3 invocations, const glm::uvec3 group_size) const
	{
		const static auto invocation_count = [](uint32_t global, uint32_t local)
		{
			return (global % local == 0) ? global / local : global / local + 1;
		};

		use();
		glDispatchComputeGroupSizeARB(
			invocation_count(invocations.x, group_size.x),
			invocation_count(invocations.y, group_size.y),
			invocation_count(invocations.z, group_size.z),
			group_size.x,
			group_size.y,
			group_size.z
		);
	}

	void ShaderProgram::dispatch(uint32_t groups) const
	{
		use();
		glDispatchCompute(groups,1,1);
	}

	void ShaderProgram::dispatch(glm::u32vec2 groups) const
	{
		use();
		glDispatchCompute(groups.x,groups.y,1);
	}

	void ShaderProgram::dispatch(glm::uvec3 groups) const
	{
		use();
		glDispatchCompute(groups.x,groups.y,groups.z);
	}

	void ShaderProgram::uniform1i(const std::string_view uniform, const int32_t value) const
	{
		glProgramUniform1i(*this, uniformLocation(uniform), value);
	}

    void ShaderProgram::uniform2i(const std::string_view uniform, const glm::ivec2& value) const
    {
        glProgramUniform2iv(*this, uniformLocation(uniform), 1, glm::value_ptr(value));
    }

    void ShaderProgram::uniform3i(const std::string_view uniform, const glm::ivec3& value) const
    {
        glProgramUniform3iv(*this, uniformLocation(uniform), 1, glm::value_ptr(value));
    }

    void ShaderProgram::uniform4i(const std::string_view uniform, const glm::ivec4& value) const
    {
        glProgramUniform4iv(*this, uniformLocation(uniform), 1, glm::value_ptr(value));
    }

    void ShaderProgram::uniform1ui(const std::string_view uniform, const uint32_t value) const
    {
        glProgramUniform1ui(*this, uniformLocation(uniform), value);
    }

    void ShaderProgram::uniform2ui(const std::string_view uniform, const glm::uvec2& value) const
    {
        glProgramUniform2uiv(*this, uniformLocation(uniform), 1, glm::value_ptr(value));
    }

    void ShaderProgram::uniform3ui(const std::string_view uniform, const glm::uvec3& value) const
    {
        glProgramUniform3uiv(*this, uniformLocation(uniform), 1, glm::value_ptr(value));
    }

    void ShaderProgram::uniform4ui(const std::string_view uniform, const glm::uvec4& value) const
    {
        glProgramUniform4uiv(*this, uniformLocation(uniform), 1, glm::value_ptr(value));
    }

    void ShaderProgram::uniform1f(const std::string_view uniform, const float value) const
	{
        glProgramUniform1f(*this, uniformLocation(uniform), value);
	}

    void ShaderProgram::uniform2f(const std::string_view uniform, const glm::vec2& vec) const
    {
        glProgramUniform2fv(*this, uniformLocation(uniform), 1, glm::value_ptr(vec));
    }

    void ShaderProgram::uniform3f(const std::string_view uniform, const glm::vec3& vec) const
    {
        glProgramUniform3fv(*this, uniformLocation(uniform), 1, glm::value_ptr(vec));
    }

    void ShaderProgram::uniform4f(const std::string_view uniform, const glm::vec4& vec) const
    {
        glProgramUniform4fv(*this, uniformLocation(uniform), 1, glm::value_ptr(vec));
    }

    void ShaderProgram::uniformMat2(const std::string_view uniform, const glm::mat2& mat, const bool transpose) const
    {
        glProgramUniformMatrix2fv(*this, uniformLocation(uniform), 1, transpose, glm::value_ptr(mat));
    }

    void ShaderProgram::uniformMat3(const std::string_view uniform, const glm::mat3& mat, const bool transpose) const
    {
        glProgramUniformMatrix3fv(*this, uniformLocation(uniform), 1, transpose, glm::value_ptr(mat));
    }

    void ShaderProgram::uniformMat4(const std::string_view uniform, const glm::mat4& mat, const bool transpose) const
    {
        glProgramUniformMatrix4fv(*this, uniformLocation(uniform), 1, transpose, glm::value_ptr(mat));
    }

    void ShaderProgram::uniformMat2x3(const std::string_view uniform, const glm::mat2x3& mat, const bool transpose) const
    {
        glProgramUniformMatrix2x3fv(*this, uniformLocation(uniform), 1, transpose, glm::value_ptr(mat));
    }

    void ShaderProgram::uniformMat3x2(const std::string_view uniform, const glm::mat3x2& mat, const bool transpose) const
    {
        glProgramUniformMatrix3x2fv(*this, uniformLocation(uniform), 1, transpose, glm::value_ptr(mat));
    }

    void ShaderProgram::uniformMat2x4(const std::string_view uniform, const glm::mat2x4& mat, const bool transpose) const
    {
        glProgramUniformMatrix2x4fv(*this, uniformLocation(uniform), 1, transpose, glm::value_ptr(mat));
    }

    void ShaderProgram::uniformMat4x2(const std::string_view uniform, const glm::mat4x2& mat, const bool transpose) const
    {
        glProgramUniformMatrix4x2fv(*this, uniformLocation(uniform), 1, transpose, glm::value_ptr(mat));
    }

    void ShaderProgram::uniformMat4x3(const std::string_view uniform, const glm::mat4x3& mat, const bool transpose) const
    {
        glProgramUniformMatrix4x3fv(*this, uniformLocation(uniform), 1, transpose, glm::value_ptr(mat));
    }

    void ShaderProgram::uniformMat3x4(const std::string_view uniform, const glm::mat3x4& mat, const bool transpose) const
    {
        glProgramUniformMatrix3x4fv(*this, uniformLocation(uniform), 1, transpose, glm::value_ptr(mat));
    }

    void ShaderProgram::uniform1b(const std::string_view uniform, const bool value) const
    {
        uniform1i(uniform, static_cast<int32_t>(value));
    }

    void ShaderProgram::uniform1ui64(const std::string_view uniform, const uint64_t value) const
    {
        glProgramUniform1ui64ARB(*this, uniformLocation(uniform), value);
    }

}}