/*
 * mpUtils
 * Preprocessor.cpp
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

#include "Preprocessor.h"

namespace mpu {
namespace gph {
namespace glsl {

    ProcessedFile process(const fs::path& file_path, const std::vector<fs::path>& include_directories,
        const std::vector<Definition>& definitions)
    {
        [[maybe_unused]] const static auto ext = [] {
            int n;
            glGetIntegerv(GL_NUM_EXTENSIONS, &n);
            for (GLuint i = 0; i < n; ++i)
                extensions.emplace(reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i)));
            return 0;
        }();

        ProcessedFile processed;
        processed.version = -1;
        processed.file_path = file_path;

        for (auto&& definition : definitions)
            processed.definitions[definition.name] = definition.info;

        std::stringstream result;
        std::set<fs::path> unique_includes;
        unique_includes.emplace(file_path);
        processImpl(file_path, include_directories, processed, unique_includes, result);

        processed.contents = result.str();
        return processed;
    }
}}}
