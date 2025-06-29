module;
#include <vector>
#include <atomic>

export module InterCpuLatencyTest;
import SPSCRingBuffer;
import CpuBondedThread;

export template <typename TransmissionType>
class CpuTransmissionLatencyTest {
    alignas(64) TransmissionType data_[1024];
    SPMCRingBuffer<TransmissionType> waitFreeQueue_{data_, std::size(data_)};
    std::atomic<bool> producerRunning_{true};

public:
    std::vector<uint64_t> run(const uint64_t measuresNumber, const uint32_t fromCpuId, const uint32_t toCpuId) {
        CpuBondedThread producerThread{fromCpuId, [this]() {
            runProducer();
        }};

        std::vector<uint64_t> latencies(measuresNumber);
        CpuBondedThread consumerThread{toCpuId, [this, &latencies]() {
            runConsumer(latencies);
        }};

        return latencies;
    }

private:
    void runConsumer(std::vector<uint64_t>& latencies) {
        uint64_t messagesHandled = 0;
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
