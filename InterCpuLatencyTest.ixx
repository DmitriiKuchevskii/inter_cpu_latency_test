//
// Created by dkuch on 6/29/25.
//
module;
#include <vector>
#include <atomic>

export module InterCpuLatencyTest;
import SPSCRingBuffer;
import CpuBondedThread;

export template <typename TransmissionType>
class CpuTransmissionLatencyTest {
    static constexpr size_t kWaitFreeQueueMemorySize = 1024 * 1024;
    std::vector<TransmissionType> waitFreeQueueMemory_;
    SPMCRingBuffer<TransmissionType> waitFreeQueue_;
    std::atomic<bool> producerRunning_{true};

public:
    explicit CpuTransmissionLatencyTest() :
        waitFreeQueueMemory_(kWaitFreeQueueMemorySize),
        waitFreeQueue_(waitFreeQueueMemory_.data(), waitFreeQueueMemory_.size())
    {}

    std::vector<size_t> run(const size_t measuresNumber, const int fromCpuId, const int toCpuId) {
        CpuBondedThread producerThread{fromCpuId, [this]() {
            runProducer();
        }};

        std::vector<size_t> latencies(measuresNumber);
        CpuBondedThread consumerThread{toCpuId, [this, &latencies]() {
            runConsumer(latencies);
        }};

        return latencies;
    }

private:
    void runConsumer(std::vector<size_t>& latencies) {
        size_t messagesHandled = 0;
        int64_t lastSeq = -1;

        while (messagesHandled != latencies.size()) {
            const int64_t curSeq = waitFreeQueue_.seq();

            if (lastSeq >= curSeq) {
                continue;
            }

            timespec t{};
            clock_gettime(CLOCK_MONOTONIC_RAW, &t);

            const auto& msg = waitFreeQueue_.get(curSeq);
            latencies[messagesHandled++] =
                (t.tv_sec * 1'000'000'000 + t.tv_nsec) - (msg.sec * 1'000'000'000 + msg.nsec);

            lastSeq = curSeq;
        }

        producerRunning_.store(false, std::memory_order_relaxed);
    }

    void runProducer() {
        while (producerRunning_.load(std::memory_order_relaxed)) {
            TransmissionType data;
            waitFreeQueue_.put(data);
        }
    }
};
