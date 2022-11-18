#pragma once

#include <vector>
#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>

typedef std::uint32_t StageID;
typedef std::uint32_t BufferID;
typedef std::uint32_t VaoID;
typedef std::uint32_t PipelineID;
typedef std::uint32_t ProgramID;
typedef uint32_t RESHANDLE;

enum class TEX { DIFFUSE, NORMAL, METALLIC, ROUGHNESS, AO };
enum class DRAW_CMD { MULTI_DRAW_ELEMENTS_INDIRECT, TEST };


struct ModelBufferRecord {

};