#pragma once

#include "../utils/CircularBuffer.h"
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <thread>
#include <atomic>

namespace EdgeGlow
{

    /**
     * @class AudioCapture
     * @brief Captures system audio using WASAPI loopback mode.
     *
     * This class interfaces with the Windows Audio Session API (WASAPI) to capture
     * whatever audio is currently playing on the system (loopback mode).
     *
     * Key Features:
     * - Loopback capture (captures "what you hear")
     * - Shared mode (non-exclusive, works with other audio apps)
     * - Real-time audio thread (high priority)
     * - Lock-free communication via circular buffer
     *
     * Thread Safety:
     * - Initialize() and Start() are NOT thread-safe (call from main thread)
     * - GetRMSEnergy() IS thread-safe (can call from render thread)
     * - Audio capture runs on dedicated thread
     */
    class AudioCapture
    {
    public:
        AudioCapture();
        ~AudioCapture();

        // Prevent copying
        AudioCapture(const AudioCapture &) = delete;
        AudioCapture &operator=(const AudioCapture &) = delete;

        /**
         * @brief Initialize WASAPI and get default audio device.
         * @return true if successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Start audio capture thread.
         * @return true if successful, false otherwise
         */
        bool Start();

        /**
         * @brief Stop audio capture thread.
         */
        void Stop();

        /**
         * @brief Check if audio capture is running.
         * @return true if capture thread is active
         */
        bool IsCapturing() const { return m_isCapturing.load(); }

        /**
         * @brief Get current RMS energy (volume) level.
         * @return RMS energy normalized to 0.0 - 1.0 range
         *
         * @note Thread-safe, can be called from render thread
         */
        float GetRMSEnergy() const { return m_rmsEnergy.load(); }

        /**
         * @brief Get sample rate of audio stream.
         * @return Sample rate in Hz (typically 48000)
         */
        uint32_t GetSampleRate() const { return m_sampleRate; }

        /**
         * @brief Get number of audio channels.
         * @return Number of channels (1 = mono, 2 = stereo)
         */
        uint32_t GetChannelCount() const { return m_channelCount; }

    private:
        /**
         * @brief Audio capture thread function.
         * Runs in a loop capturing audio samples from WASAPI.
         */
        void CaptureThreadFunc();

        /**
         * @brief Calculate RMS energy from audio samples.
         * @param samples Pointer to sample data
         * @param count Number of samples
         * @return RMS energy value (0.0 - 1.0)
         */
        float CalculateRMS(const float *samples, size_t count);

        /**
         * @brief Release COM interfaces.
         */
        void Cleanup();

    private:
        // WASAPI interfaces
        IMMDeviceEnumerator *m_deviceEnumerator; // Device enumeration
        IMMDevice *m_device;                     // Audio device
        IAudioClient *m_audioClient;             // Audio client
        IAudioCaptureClient *m_captureClient;    // Capture interface

        // Audio format
        WAVEFORMATEX *m_waveFormat; // Audio format description
        uint32_t m_sampleRate;      // Sample rate (Hz)
        uint32_t m_channelCount;    // Number of channels

        // Circular buffer for samples (4096 samples = ~85ms at 48kHz)
        CircularBuffer<float, 4096> m_buffer;

        // Threading
        std::thread m_captureThread;     // Audio capture thread
        std::atomic<bool> m_isCapturing; // Capture state flag
        std::atomic<bool> m_shouldStop;  // Stop signal

        // RMS energy (updated by capture thread, read by render thread)
        std::atomic<float> m_rmsEnergy;

        // Constants
        static constexpr size_t BUFFER_FRAMES = 4096; // Circular buffer size
    };

} // namespace EdgeGlow