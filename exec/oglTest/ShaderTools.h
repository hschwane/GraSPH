#include <GL/glew.h>
#include <GLFW/glfw3.h>

/**
 * A collection of useful global functions to reduce the code for shader complation
 */
namespace ShaderTools {
	GLuint createColorShaderProgram(float r, float g, float b);

	/**
	 * Checks a shader for compilation errors
	 * 
	 * @param shaderHandle 
	 * The id of the shader to check
	 */
	void checkShader(GLuint shaderHandle);

	/**
	 * Reads a file and returns the content as a pointer to chars
	 * 
	 * @param shaderHandle 
	 * The id of a created shader 
	 * 
	 * @param fileName 
	 * The path and filename to a shader file.
	 * \n Recommendation: Define compiler flag @c -D"SHADERS_PATH" and call 
	 * @a SHADERS_PATH"[filename]" 
	 */
	void loadShaderSource(GLint shaderHandle, const char* fileName);

	/**
	 * A combination of loadShaderSource(), checkShader() and shader compilation.
	 * 
	 * @param  vertexShaderName
	 * The path and filename to a vertex shader file.
	 * \n Recommendation: Define compiler flag @c -D"SHADERS_PATH" and call 
	 * @a SHADERS_PATH"[filename]"
	 * 
	 * @param  fragmentShaderName
	 * The path and filename to a fragment shader file. 
	 * 
	 * @return
	 * The id of a created the shader program
	 */
	GLuint makeShaderProgram(const char* vertexShaderName, const char* fragmentShaderName);
}