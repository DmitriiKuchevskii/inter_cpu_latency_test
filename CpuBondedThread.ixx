module;

#include <thread>
#include <format>
#include <cstring>

export module CpuBondedThread;

export class CpuBondedThread {
    std::jthread thread_;
public:
    explicit CpuBondedThread(const uint32_t cpuId, auto&& functor, auto&&... args) :
        thread_{
            [&, this, cpuId]() {
                cpu_set_t cpuSet;
                CPU_ZERO(&cpuSet);
                CPU_SET(cpuId, &cpuSet);
                if (const int rc = pthread_setaffinity_np(thread_.native_handle(), sizeof(cpu_set_t), &cpuSet); rc) {
                    throw std::runtime_error(
                        std::format("Can't set affinity for cpu={} rc={} message='{}'", cpuId, rc, strerror(rc))
                    );
                }
                functor(std::forward<decltype(args)>(args)...);
            }
        } {}
};