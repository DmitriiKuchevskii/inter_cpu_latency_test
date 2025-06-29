module;

#include <algorithm>
#include <cstdint>
#include <vector>
#include <fstream>

export module LatenciesWriter;

export void write_latencies(
    const uint32_t fromCpuId,
    const uint32_t toCpuId,
    std::vector<uint64_t>&& latencies,
    const std::string& outFileName = "latencies.data"
) {
    std::ranges::sort(latencies);

    std::ofstream out(outFileName, std::ios::binary | std::ios::app);
    out.write(reinterpret_cast<const char*>(&fromCpuId), sizeof(uint32_t)); // Min
    out.write(reinterpret_cast<const char*>(&toCpuId), sizeof(uint32_t)); // Min

    out.write(reinterpret_cast<const char*>(&latencies.front()), sizeof(uint64_t)); // Min
    for (size_t i = 1; i < 10; ++i) {
        out.write(reinterpret_cast<const char*>(&latencies[latencies.size() * (i / 10.0)]), sizeof(uint64_t)); // 10%% 20%% ... 90%%
    }

    out.write(reinterpret_cast<const char*>(&latencies[latencies.size() * 0.99]), sizeof(uint64_t)); // 99%%
    out.write(reinterpret_cast<const char*>(&latencies[latencies.size() * 0.999]), sizeof(uint64_t)); // 99.9%%
    out.write(reinterpret_cast<const char*>(&latencies[latencies.size() * 0.9999]), sizeof(uint64_t)); // 99.99%%

    out.write(reinterpret_cast<const char*>(&latencies.back()), sizeof(uint64_t)); // Max
}