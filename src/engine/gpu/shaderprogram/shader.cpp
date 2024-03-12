#include "shader_dec.hpp"
#include "shader_template.cpp"

#include <filesystem>
#include <ranges>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include <glad/glad.h>
#include <vector>

static unsigned compile_shader(const std::string &path, unsigned type);

namespace eng {
    ShaderProgram::ShaderProgram(const std::string &file_name) : file_name{file_name} {
        // auto files = std::filesystem::directory_iterator{SHADERS_DIR} |
        // std::views::filter([&file_name](const auto &entry) {
        //                  auto fname  = entry.path().filename().string();
        //                  auto substr = fname.substr(0, fname.rfind('.'));
        //                  return substr == file_name;
        //              })
        //              | std::views::transform([](const auto &entry) { return
        //              entry.path().string(); });

        std::vector<std::string> files;

        for (auto i : std::filesystem::directory_iterator{SHADERS_DIR}) {
            auto file     = i.path().string();
            auto filename = i.path().filename().string();
            auto substr   = filename.substr(0, filename.rfind('.'));

            if (substr != file_name) { continue; }

            files.push_back(file);
        }

        unsigned present_shaders = 0;
        using enum SHADER_TYPE;
        for (const auto &file : files) {
            const auto ext = file.substr(file.rfind('.') + 1);
            if (ext == "vert") {
                present_shaders |= (unsigned)VERTEX;
            } else if (ext == "frag") {
                present_shaders |= (unsigned)FRAGMENT;
            } else if (ext == "comp") {
                present_shaders |= (unsigned)COMPUTE;
            } else if (ext == "tesc") {
                present_shaders |= (unsigned)TESS_C;
            } else if (ext == "tese") {
                present_shaders |= (unsigned)TESS_E;
            }
        }

        program_id = glCreateProgram();

        const auto link_program = [this](const std::vector<unsigned> &ids) -> void {
            for (const auto &h : ids) { glAttachShader(program_id, h); }
            glLinkProgram(program_id);
            for (const auto &h : ids) { glDeleteShader(h); }
        };

        if (present_shaders & ((unsigned)VERTEX | (unsigned)FRAGMENT)) {
            std::vector<uint32_t> shader_handles{
                compile_shader(std::string{SHADERS_DIR}.append(file_name).append(".vert"),
                               GL_VERTEX_SHADER),
                compile_shader(std::string{SHADERS_DIR}.append(file_name).append(".frag"),
                               GL_FRAGMENT_SHADER)};

            if (present_shaders & ((unsigned)TESS_C | (unsigned)TESS_E)) {
                shader_handles.push_back(
                    compile_shader(std::string{SHADERS_DIR}.append(file_name).append(".tesc"),
                                   GL_TESS_CONTROL_SHADER));
                shader_handles.push_back(
                    compile_shader(std::string{SHADERS_DIR}.append(file_name).append(".tese"),
                                   GL_TESS_EVALUATION_SHADER));
            }

            link_program(shader_handles);
        } else if (present_shaders & (uint32_t)(COMPUTE)) {
            link_program({compile_shader(std::string{SHADERS_DIR}.append(file_name).append(".comp"),
                                         GL_COMPUTE_SHADER)});
        }
    }

    ShaderProgram::ShaderProgram(const ShaderProgram &s) noexcept { *this = s; }

    ShaderProgram::ShaderProgram(ShaderProgram &&s) noexcept { *this = std::move(s); }

    ShaderProgram &ShaderProgram::operator=(const ShaderProgram &s) noexcept {
        id         = s.id;
        program_id = s.program_id;
        file_name  = s.file_name;
        return *this;
    }

    ShaderProgram &ShaderProgram::operator=(ShaderProgram &&s) noexcept {
        id           = s.id;
        program_id   = s.program_id;
        s.program_id = 0;
        file_name    = std::move(s.file_name);
        return *this;
    }

    ShaderProgram::~ShaderProgram() {
        glDeleteProgram(program_id); 
    }

    void ShaderProgram::use() { glUseProgram(program_id); }

    void ShaderProgram::recompile() {

        glFinish();
        glUseProgram(0);
        try {
            auto prog = ShaderProgram{file_name};
            *this     = std::move(prog);
        } catch (std::runtime_error &error) { std::cout << error.what(); }
    }
} // namespace eng

static unsigned compile_shader(const std::string &path, unsigned type) {
    uint32_t shader_id;

    std::ifstream file{path};
    if (!file.is_open()) {
        throw std::runtime_error{
            std::string{"Could not open shader file at path: \""}.append(path).append("\"")};
    }

    std::stringstream ss;
    ss << file.rdbuf();

    const auto shader_source_str   = ss.str();
    const char *shader_source_cstr = shader_source_str.c_str();

    shader_id = glCreateShader(type);
    glShaderSource(shader_id, 1, &shader_source_cstr, 0);
    glCompileShader(shader_id);

    int shader_info;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &shader_info);

    if (shader_info) return shader_id;

    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &shader_info);
    char *log = new char[shader_info];
    glGetShaderInfoLog(shader_id, shader_info, &shader_info, log);

    throw std::runtime_error{std::string{"SHADER COMPILATION ERROR: \""}.append(log).append("\"")};
}
