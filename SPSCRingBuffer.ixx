module;

#include <exception>
#include <chrono>
#include <atomic>

export module SPSCRingBuffer;

export template <typename T>
class SPMCRingBuffer {
public:
    SPMCRingBuffer(T *msgsMem, const uint64_t msgsSize)
            : m_msgs(msgsMem), m_maxMsgs(msgsSize) {
        // We want it to be a power of two for fast '%' operation
        if (!(m_maxMsgs > 0 && ((m_maxMsgs & (m_maxMsgs - 1)) == 0))) {
            throw std::runtime_error(
                    "Invalid segment size: "
                    "(size / sizeof(MessageType)) must be a power of 2");
        }
    }

    void put(const T& data) {
        if (m_msgsPos == m_maxMsgs) [[unlikely]] {
            m_msgsPos = 0;
        }

        m_msgs[m_msgsPos++] = data;
        auto& curData = m_msgs[m_msgsPos - 1];

        timespec t{};
        clock_gettime(CLOCK_MONOTONIC_RAW, &t);
        curData.sec = t.tv_sec;
        curData.nsec = t.tv_nsec;

        m_curSeq.store(m_curSeq.load(std::memory_order_relaxed) + 1, std::memory_order_release);
    }

    [[nodiscard]] int64_t seq() const { return m_curSeq.load(std::memory_order_acquire); }

    [[nodiscard]] const T &get(const size_t seqNum) const {
        return m_msgs[seqNum & (m_maxMsgs - 1)];
    }

private:
    T *m_msgs{nullptr};
    uint64_t m_msgsPos{0};
    const uint64_t m_maxMsgs;
    std::atomic<int64_t> m_curSeq{-1};
};
