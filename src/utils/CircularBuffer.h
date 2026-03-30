#pragma once

#include <atomic>
#include <array>
#include <cstring>

namespace EdgeGlow
{

    /**
     * @class CircularBuffer
     * @brief Lock-free single-producer single-consumer ring buffer for audio samples.
     *
     * This is a wait-free circular buffer designed for real-time audio processing.
     * It allows one thread (audio capture) to write samples while another thread
     * (render/analysis) reads them without locks.
     *
     * Key Features:
     * - Lock-free (uses atomic operations)
     * - Single producer, single consumer (SPSC)
     * - Fixed size (power of 2 for efficient modulo)
     * - No dynamic allocation in hot path
     *
     * @tparam T Sample type (typically float for audio)
     * @tparam Size Buffer capacity (must be power of 2)
     */
    template <typename T, size_t Size>
    class CircularBuffer
    {
        static_assert((Size & (Size - 1)) == 0, "Size must be a power of 2");

    public:
        CircularBuffer()
            : m_writePos(0), m_readPos(0)
        {
        }

        /**
         * @brief Write samples to the buffer (producer).
         * @param data Pointer to samples to write
         * @param count Number of samples to write
         * @return Number of samples actually written
         *
         * @note This method is lock-free and wait-free.
         * @note Only call from the producer thread (audio capture).
         */
        size_t Write(const T *data, size_t count)
        {
            const size_t writePos = m_writePos.load(std::memory_order_relaxed);
            const size_t readPos = m_readPos.load(std::memory_order_acquire);

            // Calculate available space
            const size_t available = Size - (writePos - readPos);
            const size_t toWrite = (count < available) ? count : available;

            // Write in two chunks if wrapping around
            const size_t writeIndex = writePos & (Size - 1);
            const size_t firstChunk = std::min(toWrite, Size - writeIndex);
            const size_t secondChunk = toWrite - firstChunk;

            // Copy first chunk
            std::memcpy(&m_buffer[writeIndex], data, firstChunk * sizeof(T));

            // Copy second chunk if needed (wrap around)
            if (secondChunk > 0)
            {
                std::memcpy(&m_buffer[0], data + firstChunk, secondChunk * sizeof(T));
            }

            // Update write position (release semantics for consumer)
            m_writePos.store(writePos + toWrite, std::memory_order_release);

            return toWrite;
        }

        /**
         * @brief Read samples from the buffer (consumer).
         * @param data Pointer to buffer where samples will be copied
         * @param count Number of samples to read
         * @return Number of samples actually read
         *
         * @note This method is lock-free and wait-free.
         * @note Only call from the consumer thread (render/analysis).
         */
        size_t Read(T *data, size_t count)
        {
            const size_t readPos = m_readPos.load(std::memory_order_relaxed);
            const size_t writePos = m_writePos.load(std::memory_order_acquire);

            // Calculate available data
            const size_t available = writePos - readPos;
            const size_t toRead = (count < available) ? count : available;

            // Read in two chunks if wrapping around
            const size_t readIndex = readPos & (Size - 1);
            const size_t firstChunk = std::min(toRead, Size - readIndex);
            const size_t secondChunk = toRead - firstChunk;

            // Copy first chunk
            std::memcpy(data, &m_buffer[readIndex], firstChunk * sizeof(T));

            // Copy second chunk if needed (wrap around)
            if (secondChunk > 0)
            {
                std::memcpy(data + firstChunk, &m_buffer[0], secondChunk * sizeof(T));
            }

            // Update read position (release semantics for producer)
            m_readPos.store(readPos + toRead, std::memory_order_release);

            return toRead;
        }

        /**
         * @brief Get number of samples available for reading.
         * @return Number of samples that can be read
         */
        size_t Available() const
        {
            const size_t writePos = m_writePos.load(std::memory_order_acquire);
            const size_t readPos = m_readPos.load(std::memory_order_acquire);
            return writePos - readPos;
        }

        /**
         * @brief Get buffer capacity.
         * @return Maximum number of samples the buffer can hold
         */
        constexpr size_t Capacity() const
        {
            return Size;
        }

        /**
         * @brief Clear the buffer.
         * @warning Not thread-safe! Only call when no other threads are accessing.
         */
        void Clear()
        {
            m_writePos.store(0, std::memory_order_relaxed);
            m_readPos.store(0, std::memory_order_relaxed);
        }

    private:
        std::array<T, Size> m_buffer;                     // Sample storage
        std::atomic<size_t> m_writePos;                   // Write position (producer)
        std::atomic<size_t> m_readPos;                    // Read position (consumer)
        char m_padding[64 - sizeof(std::atomic<size_t>)]; // Cache line padding
    };

} // namespace EdgeGlow