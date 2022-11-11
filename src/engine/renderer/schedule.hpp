#pragma once

#include <memory>

#include "./typedefs.hpp"
#include "./draw_commands.hpp"

struct Stage {
    Stage() = default;

    ProgramID pid;
    VaoID vid;
    std::shared_ptr<DrawCommand> cmd;
};

using ScheduleStages = std::vector<Stage>;

class Schedule {
  public:
    std::vector<ScheduleStages> phases;
};