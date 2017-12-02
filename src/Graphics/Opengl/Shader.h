/*
 * mpUtils
 * Shader.h
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

#pragma once

#include <algorithm>
#include <map>
#include <iterator>
#include <experimental/filesystem>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Handle.h"
#include "Graphics/Opengl/glsl/Preprocessor.h"

namespace mpu {
namespace gph {


    extern std::vector<std::experimental::filesystem::path> shader_include_paths;
    void addShaderIncludePath(std::experimental::filesystem::path include_path);

	/**
	 * enum of all shader stages for type-safety
	 */
	enum class ShaderStage
	{
		eVertex = GL_VERTEX_SHADER,
		eTessControl = GL_TESS_CONTROL_SHADER,
		eTessEvaluation = GL_TESS_EVALUATION_SHADER,
		eGeometry = GL_GEOMETRY_SHADER,
		eFragment = GL_FRAGMENT_SHADER,
		eCompute = GL_COMPUTE_SHADER,
	};

	/**
	 * struct to combine a shader stage and a source file
	 */
	struct ShaderModule
	{
		ShaderModule() = default;
		ShaderModule( ShaderStage stage, std::experimental::filesystem::path path_to_file);

		// For a quicker usage, determine shader stage with just the file extension.
		ShaderModule(std::experimental::filesystem::path path_to_file);

		ShaderStage stage;
        std::experimental::filesystem::path path_to_file;
	};

    /**
     * class ShaderProgram
     * A class to manage a shader program
     *
     * usage:
     * Use one of the constructors and pass a shaderModule struct for each shader stage you want to use. You can also provide definitions that are passed
     * to the preprocessor. You can also use rebuild() to rebuild the program from a new set of shader modules.
     * After that you set your uniforms by using the functions below. For arrays consider a uniform buffer object or
     * call the glProgramUniform**v() function on your own. (same with type double uniforms)
     * Now you can bind your shader using the use() function and start to render something.
     *
     * Compute Shader:
     * To compile a compute shader just use une of the constructors or the rebuild function as described above and then call one of the dispatch() functions.
     *
     * Preprocessor:
     * When compiling the custom c/c++ style preprocssor written by Johannes Braun is used on the shader and provides the ability to use
     * constructs like "#include", "#define", "#ifdef" and other preprocessor macros.
     *
     */
	class ShaderProgram : public Handle<uint32_t, decltype(&glCreateProgram), &glCreateProgram, decltype(&glDeleteProgram), &glDeleteProgram>
	{
	public:
		ShaderProgram();
		explicit ShaderProgram(nullptr_t);

		explicit ShaderProgram(const ShaderModule& shader, const std::vector<glsl::Definition>& definitions = {}); //!< construct from shader module
		ShaderProgram(std::initializer_list<const ShaderModule> shaders, const std::vector<glsl::Definition>& definitions = {}); //!< construct from a shader module initializer list

		template<typename TIterator>
		ShaderProgram(TIterator begin, TIterator end, const std::vector<glsl::Definition>& definitions = {}); //!< construct from a bigger container with multiple shaderModules

		void rebuild(const ShaderModule& shader, const std::vector<glsl::Definition>& definitions = {}); //!< rebuild the shader program from a shader module
		void rebuild(std::initializer_list<const ShaderModule> shaders, const std::vector<glsl::Definition>& definitions = {}); //!< rebuild the shader program from a shader module initalizer list

		// You can use this constructor if you want to use a subset of a bigger container of ShaderModules.
		template<typename TIterator>
		void rebuild(TIterator begin, TIterator end, const std::vector<glsl::Definition>& definitions = {}); //!< rebuild the shader program from bigger container with multiple shaderModules

		int attributeLocation(std::string_view attribute) const; //!< querry a given attributes location
		int uniformLocation(std::string_view uniform) const; //!< querry a given uniforms location
		int uniformBlock(std::string_view uniform) const; //!< querry a given uniform blocks location

		void use() const; //!< use the compute shader for the next rendering call

		void dispatch(uint32_t invocations, uint32_t group_size) const; //!< start a 1D compute shader run
		void dispatch(glm::u32vec2 invocations, glm::u32vec2 group_size) const; //!< start a 2D ompute shader run
		void dispatch(glm::uvec3 invocations, glm::uvec3 group_size) const; //!< start a 3D compute shader run

        void dispatch(uint32_t groups) const; //!< start a 1D compute shader run using a fixed group size
        void dispatch(glm::u32vec2 groups) const; //!< start a 2D compute shader run using a fixed group size
        void dispatch(glm::uvec3 groups) const; //!< start a 3D compute shader run using a fixed group size

        // uniform upload functions --------------------------------------------

        // upload vector of ints
		void uniform1i(std::string_view uniform, int32_t value) const; //!< upload an integer to a uniform
		void uniform2i(std::string_view uniform, const glm::ivec2& value) const; //!< upload an integer vec2 to a uniform
		void uniform3i(std::string_view uniform, const glm::ivec3& value) const; //!< upload an integer vec3 to a uniform
		void uniform4i(std::string_view uniform, const glm::ivec4& value) const; //!< upload an integer vec4 to a uniform

        // upload vectors of unsigned ints
        void uniform1ui(std::string_view uniform, uint32_t value) const; //!< upload an unsigned integer to a uniform
        void uniform2ui(std::string_view uniform, const glm::uvec2& value) const; //!< upload an unsigned vec2 integer to a uniform
        void uniform3ui(std::string_view uniform, const glm::uvec3& value) const; //!< upload an unsigned vec3 integer to a uniform
        void uniform4ui(std::string_view uniform, const glm::uvec4& value) const; //!< upload an unsigned vec4 integer to a uniform

        // upload vector of floats
        void uniform1f(std::string_view uniform, float value) const; //!< upload a float to a uniform
        void uniform2f(std::string_view uniform, const glm::vec2& vec) const; //!< upload a float vec2 to a uniform
        void uniform3f(std::string_view uniform, const glm::vec3& vec) const; //!< upload a float vec2 to a uniform
        void uniform4f(std::string_view uniform, const glm::vec4& vec) const; //!< upload a float vec2 to a uniform

        // upload matrices
		void uniformMat2(std::string_view uniform, const glm::mat2& mat, bool transpose=false) const; //!< upload a mat2 to a uniform
		void uniformMat3(std::string_view uniform, const glm::mat3& mat, bool transpose=false) const; //!< upload a mat3 to a uniform
		void uniformMat4(std::string_view uniform, const glm::mat4& mat, bool transpose=false) const; //!< upload a mat4 to a uniform
		void uniformMat2x3(std::string_view uniform, const glm::mat2x3& mat, bool transpose=false) const; //!< upload a mat2x3 to a uniform
		void uniformMat3x2(std::string_view uniform, const glm::mat3x2& mat, bool transpose=false) const; //!< upload a mat3x2 to a uniform
		void uniformMat2x4(std::string_view uniform, const glm::mat2x4& mat, bool transpose=false) const; //!< upload a mat2x4 to a uniform
		void uniformMat4x2(std::string_view uniform, const glm::mat4x2& mat, bool transpose=false) const; //!< upload a mat4x2 to a uniform
		void uniformMat4x3(std::string_view uniform, const glm::mat4x3& mat, bool transpose=false) const; //!< upload a mat4x2 to a uniform
		void uniformMat3x4(std::string_view uniform, const glm::mat3x4& mat, bool transpose=false) const; //!< upload a mat4x2 to a uniform

        // special
        void uniform1b(std::string_view uniform, bool value) const; //!< upload a boolean to a uniform
        void uniform1ui64(std::string_view uniform, uint64_t value) const; //!< upload a 64bit unsigned int to a uniform

	private:
		// Only used temporarily when constructing the ShaderProgram.
		using ShaderHandle = Handle<uint32_t, decltype(&glCreateShader), &glCreateShader, decltype(&glDeleteShader), &glDeleteShader, GLenum>;
	};

	template <typename TIterator>
	ShaderProgram::ShaderProgram(TIterator begin, TIterator end, const std::vector<glsl::Definition>& definitions)
	{
		rebuild(begin, end, definitions);
	}

	template <typename TIterator>
	void ShaderProgram::rebuild(TIterator begin, TIterator end, const std::vector<glsl::Definition>& definitions)
	{
		static_assert(std::is_same_v<std::decay_t<typename std::iterator_traits<TIterator>::value_type>, ShaderModule>,
			"This constructor only accepts iterators of containers containing ShaderModule objects.");

		recreate();

        using shp=std::pair<ShaderModule, ShaderHandle>;
		std::map<ShaderStage, shp> shader_modules;

        auto size = end - begin;
		if ( size == 1)
		{
			assert_true((*begin).stage == ShaderStage::eCompute, "Shader", "It's not allowed to have only one Shader stage it it's not a compute shader.");
            shader_modules[(*begin).stage] = std::make_pair((*begin), ShaderHandle(nullptr));
		}
		else
		{
			std::for_each(begin, end, [&shader_modules](const ShaderModule& shader)
			{
				assert_true(shader_modules.count(shader.stage) == 0, "Shader", "You cannot have multiple Shader modules with the same stage in the same program.");
				shader_modules[shader.stage] = std::make_pair(shader, ShaderHandle(nullptr));
			});
			assert_true(shader_modules.count(ShaderStage::eVertex) != 0, "Shader", "Missing a Vertex Shader.");
			assert_true(shader_modules.count(ShaderStage::eFragment) != 0, "Shader", "Missing a Fragment Shader.");
			assert_true(((shader_modules.count(ShaderStage::eTessControl) + shader_modules.count(ShaderStage::eTessEvaluation)) & 0x1) == 0,
                        "Shader", "When using Tesselation Shaders, you have to provide a Tesselation Control Shader as well as a Tesselation Evaluation Shader.");
		}

		for(auto&& mapped_shader : shader_modules)
		{
			assert_critical(std::experimental::filesystem::exists(mapped_shader.second.first.path_to_file), "Shader",  "Shader file not found: \"" + mapped_shader.second.first.path_to_file.string() + "\"");
			mapped_shader.second.second = ShaderHandle(static_cast<GLenum>(mapped_shader.first));
			const uint32_t shader_handle = mapped_shader.second.second;

            auto processed = glsl::process(mapped_shader.second.first.path_to_file, shader_include_paths, definitions);

			//auto source = mcpp::preprocess(mapped_shader.second.first.path_to_file, shader_include_paths, definitions);

			const auto sources = processed.contents.data();

			glShaderSource(shader_handle, 1, &sources, nullptr);
            glCompileShader(shader_handle);

			{
				int success;
				glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &success);
				if (!success)
				{
					int log_length;
					glGetShaderiv(shader_handle, GL_INFO_LOG_LENGTH, &log_length);
					std::string log(log_length, ' ');
					glGetShaderInfoLog(shader_handle, log_length, &log_length, log.data());
					glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, shader_handle, GL_DEBUG_SEVERITY_HIGH, -1, log.c_str());
					glFinish();
					assert_critical(false, "Shader", "Shader compilation failed.");
				}
			}

			glAttachShader(*this, shader_handle);
		}

		glLinkProgram(*this);

		{
			int success;
			glGetProgramiv(*this, GL_LINK_STATUS, &success);
			if(!success)
			{
				int log_length;
				glGetProgramiv(*this, GL_INFO_LOG_LENGTH, &log_length);
				std::string log(log_length, ' ');
				glGetProgramInfoLog(*this, log_length, &log_length, log.data());
				glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, *this, GL_DEBUG_SEVERITY_HIGH, -1, log.c_str());
				assert_critical(false,"Shader", "Program linking failed.");
			}
		}

		for (const auto& mapped_shader : shader_modules)
			glDetachShader(*this, mapped_shader.second.second);
	}

}}