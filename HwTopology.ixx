//
// Created by dkuch on 6/29/25.
//
module;

#include <hwloc.h>
#include <thread>

export module HwTopology;

export class HwTopology {
    hwloc_topology_t topology_{};

public:
    HwTopology() {
        // if fails get will use std::thread::hardware_concurrency();
        if (!hwloc_topology_init(&topology_)) {
            hwloc_topology_load(topology_);
        }
    }

    ~HwTopology() {
        hwloc_topology_destroy(topology_);
    }

    [[nodiscard]] auto getPhysicalCpusNumber() const {
        if(const auto depth = hwloc_get_type_depth(topology_, HWLOC_OBJ_CORE); depth == HWLOC_TYPE_DEPTH_UNKNOWN) {
            return std::thread::hardware_concurrency();
        } else {
            return hwloc_get_nbobjs_by_depth(topology_, depth);
        }
    }
};