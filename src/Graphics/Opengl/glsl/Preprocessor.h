/*
 * mpUtils
 * Preprocessor.h
 *
 * Contains the glsl preprocessor which allows defines, macros and includes for glsl just like c/c++ preprocessor.
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

#include <fstream>
#include <experimental/filesystem>
#include <iostream>
#include <iterator>
#include <sstream>
#include <map>
#include <set>
#include <stack>
#include <cstring>

#include <GL/glew.h>
#include <Log/Log.h>

namespace mpu {
namespace gph {
namespace glsl {

    namespace fs = std::experimental::filesystem;

    enum class ShaderProfile
    {
        eCore = 0,
        eCompatibility,
        eDefault = eCore
    };

    struct DefinitionInfo
    {
        DefinitionInfo() = default;

        DefinitionInfo(const std::string value)
            : replacement(std::move(value)){}

        DefinitionInfo(const std::vector<std::string> parameters, const std::string replacement)
            : replacement(std::move(replacement)), parameters(std::move(parameters)){}

        std::string replacement;
        std::vector<std::string> parameters;
    };

    struct Definition
    {
        std::string name;
        DefinitionInfo info;
    };

    struct ProcessedFile
    {
        int32_t version;
        ShaderProfile profile;
        fs::path file_path;
        std::set<fs::path> dependencies;
        std::map<std::string, bool> extensions;
        std::map<std::string, DefinitionInfo> definitions; // name, <comm-sep-params, value>
        std::string contents;
    };

    ProcessedFile process(const fs::path& file_path, const std::vector<fs::path>& include_directories, const std::vector<Definition>& definitions);


    //////////////////////////////////////////////////////////////////////////////////////////
    //////
    //////  Everything that follows after here is not relevant for external access.
    //////
    //////////////////////////////////////////////////////////////////////////////////////////
    namespace
    {
        void syntaxError(const fs::path& file, const int line, const std::string& reason)
        {
            logERROR("Shader Preprocessor") << "Error in " << file.string() << ":" << std::to_string(line) << ": " << reason;
            throw std::runtime_error("Syntax error in shader preprocessor"+file.string()+":"+ std::to_string(line) + ": "+ reason);
        }

        bool isNewLine(const char* in)
        {
            return *in == '\n' || *in == '\r';
        }

        bool isSpace(const char* in)
        {
            return *in == ' ' || *in == '\t';
        }

        const char* skipSpace(const char* in)
        {
            while(isSpace(in))
                ++in;
            return in;
        }

		const char* skipSpaceRev(const char* in)
		{
			while (isSpace(in))
				--in;
			return in;
		}

        const char* skipToNextSpace(const char* in)
        {
            while(!isSpace(in) && !isNewLine(in) && *in != '\0')
                ++in;
            return in;
        }

		const char* skipToLineEnd(const char* in)
		{
			while (!isNewLine(in) && *in != '\0')
				++in;
			return in;
		}

        bool isTokenSame(const char* text_ptr, const char * token, const unsigned size, const bool test_before = true, const bool test_after = true)
        {
            // N is the size of the token including the null-termination char.
            return (!test_before || (!isalpha(*(text_ptr-1)))) &&
                (memcmp(text_ptr, token, size-1) == 0) &&
                (!test_after || (!isalnum(*(text_ptr + size-1)) && *(text_ptr + size-1) != '_'));
        }

        template<unsigned N>
        bool isTokenSame(const char* text_ptr, const char (&token)[N], const bool test_before = true, const bool test_after = true)
        {
            // N is the size of the token including the null-termination char.
            return isTokenSame(text_ptr, token, N, test_before, test_after);
        }

        bool isDirectiveBegin(const char* text_ptr, const bool test_before = true)
        {
            // detects a directive. It's when a '#' is the first non-space symbol of a line.
            return *text_ptr == '#' && (!test_before || *skipSpaceRev(text_ptr-1) == '\n');
        }

        bool tokenEq(const char* one, const char* other, const size_t len)
        {
            return (memcmp(one, other, len) == 0) && (isNewLine(one + len) || isSpace(one + len) || one[len] == '\0');
        }

        const char* skipToNextToken(const char* in)
        {
            return skipSpace(skipToNextSpace(in));
        }

        std::string lineCommand(const fs::path& file, const int line)
        {
            return "\n#line " + std::to_string(line) + " \"" + file.filename().string() + "\"\n";
        }

        std::set<std::string> extensions;

		bool isDefined(const std::string& val, const ProcessedFile& processed)
		{
			return processed.definitions.count(val) != 0 || (memcmp(val.data(), "GL_", 3) == 0 && extensions.count(val) != 0);
		}

		void incrementLine(int& current_line, ProcessedFile& processed)
		{
            processed.definitions["__LINE__"] = { {}, std::to_string(++current_line) };
		}

		const char* ignoreComments(const char* text_ptr, int& current_line, ProcessedFile& processed, const fs::path& current_file, std::stringstream& result)
		{
			if (memcmp(text_ptr, "//", 2) == 0)
			{
				while (!isNewLine(text_ptr))
					++text_ptr;
			}
			else if (memcmp(text_ptr, "/*", 2) == 0)
			{
				while (memcmp(text_ptr, "*/", 2) != 0)
				{
					if (isNewLine(text_ptr))
						incrementLine(current_line, processed);

					++text_ptr;
				}

				text_ptr += 2;
				if (processed.version != -1)
				{
					result << lineCommand(current_file, current_line);
				}
			}
			return text_ptr;
		}

        bool isMacro(const char* text_ptr, ProcessedFile& processed)
		{
            const auto begin = text_ptr;
            while(isalnum(*text_ptr) || *text_ptr == '_')
                ++text_ptr;

            const std::string str(begin, text_ptr);
            return isDefined(str, processed);
		}

        std::string expandMacro(const std::string& name, const char* param_start, const int param_length,
            const fs::path& current_file, const int current_line, ProcessedFile& processed)
		{
		    std::stringstream stream;
			if (processed.definitions.count(name) == 0)
				return name;

            auto info = processed.definitions.at(name);

            if(info.parameters.empty())
                return info.replacement;

            if(memcmp(name.data(), "GL_", 3) == 0 && extensions.count(name) != 0)
                return "1";

            std::vector<std::string> inputs;

            if(param_start != nullptr)
            {
                std::stringstream param_stream({param_start, param_start+param_length});
                std::string in;

                while(std::getline(param_stream, in, ','))
                {
                    const auto bg = skipSpace(in.data());
                    inputs.push_back({bg, skipToNextSpace(bg)});
                }
            }

            if(inputs.size() != info.parameters.size() || (info.parameters.size() >= inputs.size()-1 && inputs.back() == "..."))
                syntaxError(current_file, current_line, "Macro " + name + ": non-matching argument count.");


            bool skip_stream = false;
            for(int replacement_offset=0; replacement_offset<info.replacement.length(); ++replacement_offset)
            {
                for(int parameter = 0; parameter < info.parameters.size(); ++parameter)
                {
                    if(isTokenSame(&info.replacement[replacement_offset], info.parameters[parameter].data(),
                                   static_cast<unsigned>(info.parameters[parameter].length() + 1), replacement_offset!=0))
                    {
                        skip_stream = true;
                        stream << inputs[parameter];
                        replacement_offset += static_cast<int>(info.parameters[parameter].length()-1);
                        break;
                    }
                    if(isTokenSame(&info.replacement[replacement_offset], "__VA_ARGS__", replacement_offset != 0) && info.parameters[parameter] == "...")
                    {
                        skip_stream = true;
                        for(auto input_parameter = parameter; input_parameter != inputs.size(); ++input_parameter)
                            stream << inputs[input_parameter];
                        break;
                    }
                }
                if(skip_stream)
                    skip_stream = false;
                else
                   stream << info.replacement[replacement_offset];
            }
            return stream.str();
		}

		namespace {
			int neg(const int in) { return -in; }
			int pos(const int in) { return in; }
			int inv(const int in) { return ~in; }
			int no(const int in) { return !in; }

			int add(const int in, const int ot) { return in + ot; }
			int sub(const int in, const int ot) { return in - ot; }
			int mul(const int in, const int ot) { return in*ot; }
			int dv(const int in, const int ot) { return in / ot; }
			int mod(const int in, const int ot) { return in%ot; }
		}

		std::map<char, int(*)(int)> prefixes =
		{
			{ '-', &neg },
			{ '+', &pos },
			{ '~', &inv },
			{ '!', &no },
		};

		std::map<char, int(*)(int, int)> operators_arithmetic =
		{
			{ '-', &sub },
			{ '+', &add },
			{ '*', &mul },
			{ '/', &dv },
			{ '%', &mod },
		};

		bool isImportantArithmeticOp(const char* c)
		{
			return *c == '*' || *c == '/' || *c == '%';
		}

		const char* skipPrefixes(const char* c)
		{
			while (prefixes.count(*c) != 0)
				++c;
			return c;
		}

		const char* skipEvaluationToken(const char* current_character, const int max_length, const fs::path& current_file, const int current_line)
		{
			const auto begin = current_character = skipPrefixes(skipSpace(current_character));
			if (*begin == '(')
			{
				int bracket_stack = 0;
				while (true)
				{
					if (*current_character == ')')
						--bracket_stack;
					else if (*current_character == '(')
						++bracket_stack;
					++current_character;

					if (!bracket_stack)
						break;

					if (*current_character == '\0')
						syntaxError(current_file, current_line, "Unexpected end of brackets. " + std::to_string(bracket_stack));
				}
			}
			else
			{
				while ((isalnum(*current_character) || *current_character == '_') && current_character - begin < max_length)
					++current_character;
			}

			return current_character;
		}

		int evaluate(const char* ptr, int length, const fs::path& current_file, int current_line);

		int evaluateToken(const char* ptr, int length, const fs::path& current_file, const int current_line)
		{
			const auto space_skipped = skipSpace(ptr);
			length = static_cast<int>(1 + length - (space_skipped - ptr));
			ptr = space_skipped;

			if (*ptr == '(')
			{
				return evaluate(ptr + 1, length - 2, current_file, current_line);
			}

			if (prefixes.count(*ptr))
			{
				const auto prefix_skipped = skipSpace(ptr + 1);
				return prefixes[*ptr](evaluateToken(prefix_skipped,
                    static_cast<int>(skipEvaluationToken(prefix_skipped, static_cast<int>(length - (prefix_skipped - ptr)), current_file, current_line) - prefix_skipped - 1),
                    current_file, current_line));
			}

			const auto token_skipped = skipEvaluationToken(ptr, length, current_file, current_line);

            // range-based atoi
			int val = 0;
			for (auto p = ptr; p != token_skipped; ++p)
			{
				val = (10 * val) + (*p - '0');
			}
			return val;
		}

		const char* skipImportantOperator(const char* ptr, int length, const fs::path& current_file, const int current_line)
		{
			while (isImportantArithmeticOp(ptr) || isalnum(*ptr) || *ptr == '_' || isSpace(ptr) || *ptr == '(')
			{
				const auto last = ptr;
				ptr = skipEvaluationToken(ptr, length, current_file, current_line);
				auto a = ptr - last;
				if (a == 0) {
                    // Safety check to prohibit infinite loops
					a = 1;
					++ptr;
				}
				length -= static_cast<int>(a);
			}
			return ptr;
		}

		int evaluate(const char* ptr, const int length, const fs::path& current_file, const int current_line)
		{
			const auto start = ptr;
			const auto token_skipped = skipEvaluationToken(prefixes.count(*ptr) ? ptr + 1 : ptr, length, current_file, current_line);
			int current_value = evaluateToken(ptr, static_cast<int>(token_skipped - ptr - 1), current_file, current_line);
			ptr = token_skipped;

			while (*ptr != '\0' && ptr - start < length)
			{
				if (operators_arithmetic.count(*ptr) != 0)
				{
					if (isImportantArithmeticOp(ptr))
					{
						const auto next_token_skipped = skipEvaluationToken(ptr + 1, static_cast<int>(length - (ptr + 1 - start)), current_file, current_line);
						const auto next_evaluated = evaluateToken(ptr + 1, static_cast<int>(next_token_skipped - (ptr + 1)), current_file, current_line);
						current_value = operators_arithmetic[*ptr](current_value, next_evaluated);
						ptr = next_token_skipped;
					}
					else
					{
						const auto next_token_skipped = skipImportantOperator(ptr + 1, static_cast<int>(length - (ptr + 1 - start)), current_file, current_line);
						const auto next_evaluated = evaluate(ptr + 1, static_cast<int>(next_token_skipped - (ptr + 1)), current_file, current_line);
						current_value = operators_arithmetic[*ptr](current_value, next_evaluated);
						ptr = next_token_skipped;
					}
				}
				else
				{
					++ptr;
				}
				ptr = skipSpace(ptr);
			}

			return current_value;
		}

		std::string expandMacrosInLine(const char* text_ptr, const char* &text_ptr_after, const fs::path& current_file, const int current_line, ProcessedFile& processed)
		{
			std::string line(text_ptr, skipToLineEnd(text_ptr));
			bool first_replacement = true;
			while (true) {
				const auto begin = text_ptr;
				while (isalnum(*text_ptr) || *text_ptr == '_')
					++text_ptr;

				const auto begin_params = skipSpace(text_ptr);
				auto end_params = begin_params - 1;
				if (*begin_params == '(')
				{
					while (*end_params != ')')
						++end_params;
				}

				if (!isMacro(begin, processed))
				{
					++text_ptr;
					if (*text_ptr == '\0' || *text_ptr == '\n' || line.empty() || text_ptr == &line[line.size() - 1])
						break;
					continue;
				}

				const auto params_start = *begin_params == '(' ? begin_params + 1 : nullptr;
				const auto params_length = *begin_params == '(' ? end_params - params_start : 0;

				std::string expanded_macro = expandMacro({ begin, text_ptr }, params_start, static_cast<int>(params_length), current_file, current_line, processed);

				if (first_replacement)
				{
					first_replacement = false;
					text_ptr_after = end_params;
					line = expanded_macro;
				}
				else
				{
					line.replace(line.begin() + static_cast<size_t>(begin - line.data()), line.begin() + static_cast<size_t>(text_ptr - line.data()), expanded_macro.begin(), expanded_macro.end());
				}

				text_ptr = line.data();
				bool enable_test_macro = true;
				while (!line.empty() && ((!enable_test_macro || !isMacro(text_ptr, processed)) && text_ptr != &line[line.size() - 1]))
				{
					if (!(isalnum(*text_ptr) || *text_ptr == '_'))
						enable_test_macro = true;
					else
						enable_test_macro = false;
					++text_ptr;
				}

				if (line.empty() || text_ptr == &line[line.size() - 1])
					break;
			}
			return line;
		}

        void processImpl(const fs::path& file_path,
            const std::vector<fs::path>& include_directories,
            ProcessedFile& processed,
			std::set<fs::path>& unique_includes,
			std::stringstream& result)
        {
			int defines_nesting = 0;
            std::stack<bool> accept_else_directive;

            std::ifstream root_file(file_path, std::ios::in);
            std::string contents(std::istreambuf_iterator<char>{root_file}, std::istreambuf_iterator<char>{});

            const char* text_ptr = contents.data();

            fs::path current_file = file_path;
            processed.definitions["__FILE__"] = { {}, current_file.string() };
            int current_line = 1;

            // There is no way you could put a macro starting from the first character of the shader.
            // Set to true if the current text_ptr may point to the start of a macro name.
            bool enable_macro = false;

            while(*text_ptr != '\0')
            {
				text_ptr = ignoreComments(text_ptr, current_line, processed, current_file, result);

                if(isNewLine(text_ptr))
                {
					incrementLine(current_line, processed);
                    result << '\n';
                    ++text_ptr;
                }
                else if(enable_macro && isMacro(text_ptr, processed))
                {
                    result << expandMacrosInLine(text_ptr, text_ptr, current_file, current_line, processed);
					++text_ptr;
                }
                else if(isDirectiveBegin(text_ptr, current_line!=1))
                {
                    const auto directive_name = skipSpace(text_ptr + 1);
                    if(isTokenSame(directive_name, "version"))
                    {
                        text_ptr = skipToNextToken(directive_name);
                        processed.version = (*text_ptr - '0') * 100 +
                                            (*(text_ptr+1) - '0') * 10 +
                                            (*(text_ptr+2) - '0');

						processed.definitions["__VERSION__"] = { {}, {text_ptr, text_ptr + 3} };

                        result << "#version " << *text_ptr << *(text_ptr+1) << *(text_ptr + 2) << " ";
                        text_ptr = skipToNextToken(text_ptr);

						if (isNewLine(text_ptr))
						{
							processed.definitions["GL_core_profile"] = { {}, "1" };
							processed.profile = ShaderProfile::eDefault;
						}
						else if (isTokenSame(text_ptr, "core"))
						{
							processed.definitions["GL_core_profile"] = { {}, "1" };
							processed.profile = ShaderProfile::eCore;
						}
						else if (isTokenSame(text_ptr, "compatibility"))
						{
							processed.definitions["GL_compatibility_profile"] = { {}, "1" };
							processed.profile = ShaderProfile::eCompatibility;
						}
                        else
                        {
                            syntaxError(current_file, current_line, "Unrecognized profile: " + std::string(text_ptr, skipToLineEnd(text_ptr)));
                        }

                        while(!isNewLine(text_ptr))
                            result << *text_ptr++;

                        result << lineCommand(current_file, current_line);
                    }
                    else if (isTokenSame(directive_name, "extension"))
                    {
                        text_ptr = skipToNextToken(directive_name);
                        result << "#extension ";

                        const auto name_end = skipToNextSpace(text_ptr);
                        const std::string extension(text_ptr, name_end);

                        result << extension << " : ";

                        text_ptr = skipSpace(skipSpace(name_end)+1);

						if (isTokenSame(text_ptr, "require"))
							processed.extensions[extension] = true;
						else if (isTokenSame(text_ptr, "enable"))
							processed.extensions[extension] = false;
						else
						{
							syntaxError(current_file, current_line, "Unrecognized extension requirement: " + std::string(text_ptr, skipToLineEnd(text_ptr)));
						}

						while (!isNewLine(text_ptr))
						    result << *text_ptr++;
					}
					else if (isTokenSame(directive_name, "pragma"))
					{
						text_ptr = skipToNextToken(directive_name);

						if (isTokenSame(text_ptr, "once"))
						{
							unique_includes.emplace(current_file);
							text_ptr = skipToLineEnd(text_ptr);
						}
						else
						{
							result << "#pragma ";
						}
                        // TODO: It is possible to add custom pragmas
					}
					else if (isTokenSame(directive_name, "define"))
					{
						text_ptr = skipToNextToken(directive_name);
						const auto name_begin = text_ptr;
						while (!isSpace(text_ptr) && !isNewLine(text_ptr) && *text_ptr != '(')
							++text_ptr;

						if (const auto space_skipped = skipSpace(text_ptr); isSpace(text_ptr) && (*space_skipped != '(') && !isNewLine(space_skipped))
						{
							// macro without params
							auto value_end = space_skipped;
							std::stringstream val;
							while (!(isNewLine(value_end) && *(value_end - 1) != '\\'))
							{
								if (*value_end != '\\')
									val << *value_end;
								if (isNewLine(value_end))
									incrementLine(current_line, processed);
								++value_end;
							}

                            processed.definitions[{name_begin, text_ptr}] = DefinitionInfo(val.str());

							text_ptr = value_end;
						}
						else if (isNewLine(text_ptr) || isNewLine(space_skipped))
						{
							// define without value
							processed.definitions[{name_begin, text_ptr}];
						}
						else if (*space_skipped == '(')
						{
							// macro with params
							const auto name_end = text_ptr;
							const auto params_begin = space_skipped + 1;
							while (*text_ptr != ')')
								++text_ptr;
							const auto params_end = text_ptr;

							// macro without params
							auto value_end = skipSpace(params_end + 1);
							std::stringstream definition_stream;
							while (!(isNewLine(value_end) && *(value_end - 1) != '\\'))
							{
								if (*value_end != '\\')
									definition_stream << *value_end;
								if (isNewLine(value_end))
									incrementLine(current_line, processed);
								++value_end;
							}

                            std::string parameter;
                            std::vector<std::string> parameters;
                            std::stringstream param_stream({ skipSpace(params_begin), params_end });

                            while(std::getline(param_stream, parameter, ','))
                            {
                                const auto param_begin = skipSpace(parameter.data());
                                parameters.push_back({param_begin, skipToNextSpace(param_begin)});
                            }

                            while(param_stream >> parameter)
                            {
                                parameters.push_back(parameter);

                                if(param_stream.peek() == ',' || param_stream.peek() == ' ')
                                    param_stream.ignore();
                            }

                            processed.definitions[{name_begin, name_end}] = {std::move(parameters), definition_stream.str()};

							text_ptr = value_end;
						}

						result << lineCommand(current_file, current_line);
					}
					else if (isTokenSame(directive_name, "undef"))
					{
						text_ptr = skipToNextToken(directive_name);
						const auto begin = text_ptr;
						while (!isSpace(text_ptr) && !isNewLine(text_ptr))
							++text_ptr;

						processed.definitions.erase({ begin, text_ptr });
					}
					else if (const auto elif = isTokenSame(directive_name, "elif");  isTokenSame(directive_name, "if", true, false) || (elif))
					{
						++defines_nesting;
						text_ptr = skipToNextToken(directive_name);
						const auto value_begin = text_ptr;
						while (!isNewLine(text_ptr) && !isSpace(text_ptr))
							++text_ptr;

						bool evaluated;
						if (isTokenSame(directive_name, "ifdef"))
							evaluated = isDefined({ value_begin, text_ptr }, processed);
						else if (isTokenSame(directive_name, "ifndef"))
							evaluated = !isDefined({ value_begin, text_ptr }, processed);
						else if (elif && !accept_else_directive.top())
							evaluated = false;
						else
						{
							// Simple IF
							std::stringstream line;
							for (auto i = value_begin; i != text_ptr; ++i)
							{
								if (memcmp(i, "//", 2) == 0)
								{
									break;
								}
								if (memcmp(i, "/*", 2) == 0)
								{
									while (memcmp(i, "*/", 2) != 0)
										++i;
									++i;
								}
                                else if(isTokenSame(i, "defined"))
                                {
                                    while(*i != '(')
                                        ++i;
                                    const auto defined_macro_begin = i;
                                    while(*i != ')')
                                        ++i;

                                    line << (isDefined(std::string(defined_macro_begin+1, i), processed) ? '1' : '0');
                                }
								else
									line << *i;
							}

							auto line_str = line.str();
							auto str = expandMacrosInLine(line_str.c_str(), text_ptr, current_file, current_line, processed);

							evaluated = evaluate(str.data(), static_cast<int>(str.length()), current_file, current_line);
						}

						if (evaluated)
						{
							text_ptr = skipToLineEnd(text_ptr);
							if (elif)
								accept_else_directive.top() = false;
							else
								accept_else_directive.push(false);
						}
						else
						{
							if (!elif)
								accept_else_directive.push(true);
							for(;;++text_ptr)
							{
								text_ptr = ignoreComments(text_ptr, current_line, processed, current_file, result);
								if (isNewLine(text_ptr))
								{
									incrementLine(current_line, processed);
								}
								else if (const auto space_skipped = skipSpace(text_ptr);  (*(text_ptr - 1) == '\n') && *space_skipped == '#')
								{
									text_ptr = space_skipped;
									if (memcmp(skipSpace(text_ptr + 1), "if", 2) == 0)
									{
										auto deeper_skipped = skipSpace(text_ptr);
										while (!(isDirectiveBegin(deeper_skipped) && (isTokenSame(skipSpace(deeper_skipped+1), "endif") || isTokenSame(deeper_skipped+1, "elif"))))
										{
											text_ptr = ignoreComments(text_ptr, current_line, processed, current_file, result);
											if (isNewLine(text_ptr))
												incrementLine(current_line, processed);
											++text_ptr;
											deeper_skipped = skipSpace(text_ptr);
										}
										text_ptr = skipToLineEnd(text_ptr);
									}
									else if (isTokenSame(skipSpace(text_ptr + 1), "elif") ||
                                        isTokenSame(skipSpace(text_ptr + 1), "endif") ||
                                        isTokenSame(skipSpace(text_ptr + 1), "else"))
									{
										break;
									}
								}
								else if (*text_ptr == '\0')
								{
									syntaxError(current_file, current_line, "no closing endif or else found.");
									break;
								}
							}
						}
					}
					else if (isTokenSame(directive_name, "else"))
					{
						if (accept_else_directive.top())
						{
							text_ptr = skipToLineEnd(text_ptr);
						}
						else
						{
							while (true)
							{
								if (isNewLine(text_ptr))
								{
									incrementLine(current_line, processed);
								}
								else if (isDirectiveBegin(text_ptr) && isTokenSame(skipSpace(text_ptr + 1), "endif"))
								{
									break;
								}
								else if (*text_ptr == '\0')
								{
									syntaxError(current_file, current_line, "no closing endif found.");
									break;
								}

							    ++text_ptr;
							}
						}
					}
					else if (isTokenSame(directive_name, "endif"))
					{
						accept_else_directive.pop();
						text_ptr = skipToLineEnd(directive_name);
						--defines_nesting;
						result << lineCommand(current_file, current_line);
					}
					else if (isTokenSame(directive_name, "line"))
					{
						text_ptr = skipToNextToken(directive_name);
						const auto line_nr_end = skipToNextSpace(text_ptr);

						result << "#line ";

						int new_line_number = 0;
						for (auto i = text_ptr; (i != line_nr_end) && (new_line_number *= 10) != -1; ++i)
							new_line_number += *i - '0';

						result << new_line_number << " ";

						text_ptr = skipSpace(line_nr_end);
						if (*text_ptr == '\"')
						{
							if (const auto file_name_end = skipToNextSpace(text_ptr)-1;*file_name_end == '\"')
							{
                                current_file = fs::path(std::string(text_ptr + 1, file_name_end));
                                processed.definitions["__FILE__"] = { {}, current_file.string() };
								result << "\"" << current_file << "\"";
							}
							else
							{
								syntaxError(current_file, current_line, "Invalid line directive, did not find closing \".");
							}
						}
						text_ptr = skipToLineEnd(text_ptr);

						incrementLine(current_line = new_line_number - 1, processed);
					}
					else if (isTokenSame(directive_name, "error"))
					{
						const auto begin = skipToNextToken(directive_name);
					    logERROR("Shader Preprocessor") << current_file.string() << ":" << std::to_string(current_line) << " " << std::string(begin, skipToLineEnd(begin));
                        throw std::runtime_error("");
                    }
					else if (isTokenSame(directive_name, "include"))
					{
						auto include_begin = skipToNextToken(text_ptr);
						auto include_filename = expandMacrosInLine(include_begin, include_begin, current_file, current_line, processed);

						if ((include_filename.front() != '\"' && include_filename.back() != '\"') && (include_filename.front() != '<' && include_filename.back() != '>'))
						{
							syntaxError(current_file, current_line, "Include must be in \"...\" or <...>");
						}
						fs::path file = { std::string(include_filename.begin()+1, include_filename.end()-1) };

						bool found_file = false;
						for (auto&& directory : include_directories)
						{
							if (exists(directory / file))
							{
								found_file = true;
								file = directory / file;
							}
						}

						if (!found_file)
						{
							file = file_path.parent_path() / file;
						}

                        if(!fs::exists(file))
                            syntaxError(current_file, current_line, "File not found: " + file.string());

						if (unique_includes.count(file) == 0)
						{
							result << lineCommand(file, 1);
							processImpl(file, include_directories, processed, unique_includes, result);
                            processed.dependencies.emplace(file);
						}
						text_ptr = skipToLineEnd(include_begin);

						result << lineCommand(current_file, current_line);
					}
					else
                    {
                        if(!(isalpha(*text_ptr) || *text_ptr == '_'))
                            enable_macro = true;
                        else
                            enable_macro = false;
                        result << *text_ptr;
                        ++text_ptr;
                    }
                }
                else
                {
                    if(!(isalpha(*text_ptr) || *text_ptr == '_'))
                        enable_macro = true;
                    else
                        enable_macro = false;

                    result << *text_ptr;
                    ++text_ptr;
                }
            }
        }
    }
}}}
