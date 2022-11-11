#pragma once

#include <memory>

#include "./typedefs.hpp"
#include "./draw_commands.hpp"

class Shader;

struct Stage {
    Shader* pid{nullptr};
    VaoID vid;
    std::shared_ptr<DrawCommand> cmd;
};

using ScheduleStages = std::vector<Stage>;

class Schedule {
  public:
    std::vector<ScheduleStages> phases;
};