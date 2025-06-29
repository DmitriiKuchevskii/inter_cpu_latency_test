#include <cstdint>
#include <csignal>
#include <filesystem>
#include <format>
#include <iterator>
#include <iostream>
#include <fstream>

import SPSCRingBuffer;
import LatenciesWriter;
import HwTopology;
import CpuBondedThread;
import InterCpuLatencyTest;

struct __attribute__((packed)) CacheLine  {
    CacheLine() {
        for (size_t i = 0; i < std::size(data_); i++) {
            if (i % 5 == 0) {
                data_[i] = static_cast<char>(rand());
            } else {
                data_[i] = static_cast<char>(i);
            }
        }
    }
    char data_[48]{};
    uint64_t sec{};
    uint64_t nsec{};
};

struct __attribute__((packed)) DoubleCacheLine  {
    DoubleCacheLine() {
        for (size_t i = 0; i < std::size(data_); i++) {
            if (i % 5 == 0) {
                data_[i] = static_cast<char>(rand());
            } else {
                data_[i] = static_cast<char>(i);
            }
        }
    }

    char data_[112]{};
    uint64_t sec{};
    uint64_t nsec{};
};

static void disable_signals() {
    sigset_t mask;
    if (sigaddset(&mask, SIGINT)) {
        throw std::runtime_error("Not valid signal");
    }
    if (sigprocmask(SIG_UNBLOCK, &mask, nullptr)) {
        throw std::runtime_error("Cant disable signals");
    }
}

template<typename TransmissionType>
void run_test(const uint32_t physicalCpusNumber) {
    static_assert(sizeof(TransmissionType) <= 128,
        "This test has been designed to measure transmission for up to 2 cache lines. "
        "If you know what you're doing you can get rid of this static_assert");

    const std::string kTypeName = typeid(TransmissionType).name();

    std::cout << std::format("Running test for transmission type '{}'.... it can take couple of minutes. "
                             "PLease wait...\n", kTypeName);

    const std::string outFileName = std::format("latencies_{}.data", kTypeName);
    std::filesystem::remove(outFileName);

    static constexpr size_t kHandledTestMessages = 1024 * 1024;

    for (uint32_t fromCpuId = 0; fromCpuId < physicalCpusNumber; ++fromCpuId) {
        for (uint32_t toCpuId = 0; toCpuId < physicalCpusNumber; ++toCpuId) {
            if (fromCpuId != toCpuId) {
                auto latencies = CpuTransmissionLatencyTest<TransmissionType>{}.run(
                    kHandledTestMessages,
                    fromCpuId,
                    toCpuId
                );

                write_latencies(
                    fromCpuId,
                    toCpuId,
                    std::move(latencies),
                    outFileName
                );
            }
        }
    }

    std::cout << std::format("The result for transmission type '{}' has been written into '{}'\n", kTypeName, outFileName);
}

int main() {
    disable_signals();

    const uint32_t physicalCpusNumber = HwTopology{}.getPhysicalCpusNumber();

    run_test<CacheLine>(physicalCpusNumber);
    run_test<DoubleCacheLine>(physicalCpusNumber);
}