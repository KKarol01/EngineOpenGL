#include "gpu_res_mgr.hpp"

eng::GpuResMgr::~GpuResMgr() {
    for (auto &[_, c] : _containers) {
        for (auto &e : c) { delete e; }
    }
}
