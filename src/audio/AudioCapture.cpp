#include "AudioCapture.h"
#include <cmath>
#include <iostream>
#include <mmreg.h> // For WAVEFORMATEXTENSIBLE
#include <ks.h>    // For KSDATAFORMAT_SUBTYPE_IEEE_FLOAT

// Link WASAPI libraries
#pragma comment(lib, "ole32.lib")

// WASAPI constants
const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

namespace EdgeGlow
{

    AudioCapture::AudioCapture()
        : m_deviceEnumerator(nullptr), m_device(nullptr), m_audioClient(nullptr), m_captureClient(nullptr), m_waveFormat(nullptr), m_sampleRate(0), m_channelCount(0), m_isCapturing(false), m_shouldStop(false), m_rmsEnergy(0.0f)
    {
    }

    AudioCapture::~AudioCapture()
    {
        Stop();
        Cleanup();
    }

    bool AudioCapture::Initialize()
    {
        HRESULT hr;

        // Create device enumerator
        hr = CoCreateInstance(
            CLSID_MMDeviceEnumerator,
            nullptr,
            CLSCTX_ALL,
            IID_IMMDeviceEnumerator,
            (void **)&m_deviceEnumerator);

        if (FAILED(hr))
        {
            std::cerr << "Failed to create device enumerator" << std::endl;
            return false;
        }

        // Get default audio output device (for loopback capture)
        hr = m_deviceEnumerator->GetDefaultAudioEndpoint(
            eRender,  // Render endpoint (output device)
            eConsole, // Console role
            &m_device);

        if (FAILED(hr))
        {
            std::cerr << "Failed to get default audio device" << std::endl;
            return false;
        }

        // Activate audio client
        hr = m_device->Activate(
            IID_IAudioClient,
            CLSCTX_ALL,
            nullptr,
            (void **)&m_audioClient);

        if (FAILED(hr))
        {
            std::cerr << "Failed to activate audio client" << std::endl;
            return false;
        }

        // Get device format
        hr = m_audioClient->GetMixFormat(&m_waveFormat);
        if (FAILED(hr))
        {
            std::cerr << "Failed to get audio format" << std::endl;
            return false;
        }

        // Store format info
        m_sampleRate = m_waveFormat->nSamplesPerSec;
        m_channelCount = m_waveFormat->nChannels;

        std::cout << "Audio Format:" << std::endl;
        std::cout << "  Sample Rate: " << m_sampleRate << " Hz" << std::endl;
        std::cout << "  Channels: " << m_channelCount << std::endl;
        std::cout << "  Bits Per Sample: " << m_waveFormat->wBitsPerSample << std::endl;
        std::cout << "  Format Tag: " << m_waveFormat->wFormatTag << std::endl;

        // Verify we got a format we can handle (32-bit float)
        bool isFloatFormat = false;
        if (m_waveFormat->wFormatTag == WAVE_FORMAT_IEEE_FLOAT && m_waveFormat->wBitsPerSample == 32)
        {
            isFloatFormat = true;
            std::cout << "  Format: Standard IEEE Float" << std::endl;
        }
        else if (m_waveFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE && m_waveFormat->wBitsPerSample == 32)
        {
            // Check the extensible format sub-type
            WAVEFORMATEXTENSIBLE *extFormat = reinterpret_cast<WAVEFORMATEXTENSIBLE *>(m_waveFormat);
            if (IsEqualGUID(extFormat->SubFormat, KSDATAFORMAT_SUBTYPE_IEEE_FLOAT))
            {
                isFloatFormat = true;
                std::cout << "  Format: Extensible IEEE Float" << std::endl;
            }
            else
            {
                std::cout << "  Format: Extensible (not IEEE float)" << std::endl;
            }
        }
        else
        {
            std::cout << "  Format: Unsupported" << std::endl;
        }

        if (!isFloatFormat)
        {
            std::cerr << "ERROR: Expected 32-bit IEEE float format, got format tag "
                      << m_waveFormat->wFormatTag << " with " << m_waveFormat->wBitsPerSample << " bits" << std::endl;
            return false;
        }

        // Initialize audio client in loopback mode
        // AUDCLNT_STREAMFLAGS_LOOPBACK = capture output audio
        hr = m_audioClient->Initialize(
            AUDCLNT_SHAREMODE_SHARED,     // Shared mode
            AUDCLNT_STREAMFLAGS_LOOPBACK, // Loopback capture
            10000000,                     // Buffer duration (1 second in 100ns units)
            0,                            // Periodicity (0 for shared mode)
            m_waveFormat,                 // Format
            nullptr);                     // Audio session GUID

        if (FAILED(hr))
        {
            std::cerr << "Failed to initialize audio client. HRESULT: 0x" << std::hex << hr << std::endl;
            return false;
        }

        // Get capture client interface
        hr = m_audioClient->GetService(
            IID_IAudioCaptureClient,
            (void **)&m_captureClient);

        if (FAILED(hr))
        {
            std::cerr << "Failed to get capture client" << std::endl;
            return false;
        }

        std::cout << "Audio capture initialized successfully!" << std::endl;
        return true;
    }

    bool AudioCapture::Start()
    {
        if (m_isCapturing.load())
        {
            return true; // Already capturing
        }

        if (!m_audioClient)
        {
            std::cerr << "Audio client not initialized" << std::endl;
            return false;
        }

        // Start audio client
        HRESULT hr = m_audioClient->Start();
        if (FAILED(hr))
        {
            std::cerr << "Failed to start audio client" << std::endl;
            return false;
        }

        // Start capture thread
        m_shouldStop.store(false);
        m_captureThread = std::thread(&AudioCapture::CaptureThreadFunc, this);

        // Set thread priority to high (important for audio)
        SetThreadPriority(m_captureThread.native_handle(), THREAD_PRIORITY_TIME_CRITICAL);

        m_isCapturing.store(true);
        std::cout << "Audio capture started!" << std::endl;

        return true;
    }

    void AudioCapture::Stop()
    {
        if (!m_isCapturing.load())
        {
            return; // Not capturing
        }

        // Signal thread to stop
        m_shouldStop.store(true);

        // Wait for thread to finish
        if (m_captureThread.joinable())
        {
            m_captureThread.join();
        }

        // Stop audio client
        if (m_audioClient)
        {
            m_audioClient->Stop();
        }

        m_isCapturing.store(false);
        std::cout << "Audio capture stopped" << std::endl;
    }

    void AudioCapture::CaptureThreadFunc()
    {
        std::cout << "Capture thread started" << std::endl;

        HRESULT hr;
        UINT32 packetLength = 0;
        UINT32 numFramesAvailable;
        BYTE *pData;
        DWORD flags;

        while (!m_shouldStop.load())
        {
            // Check if data is available
            hr = m_captureClient->GetNextPacketSize(&packetLength);
            if (FAILED(hr))
            {
                std::cerr << "GetNextPacketSize failed" << std::endl;
                break;
            }

            // Process all available packets
            while (packetLength != 0 && !m_shouldStop.load())
            {
                // Get the captured data
                hr = m_captureClient->GetBuffer(
                    &pData,
                    &numFramesAvailable,
                    &flags,
                    nullptr,
                    nullptr);

                if (FAILED(hr))
                {
                    std::cerr << "GetBuffer failed" << std::endl;
                    break;
                }

                // Convert to float samples and calculate RMS
                if (numFramesAvailable > 0 && !(flags & AUDCLNT_BUFFERFLAGS_SILENT))
                {
                    // Assuming 32-bit float format (WASAPI typically uses this)
                    const float *samples = reinterpret_cast<const float *>(pData);
                    size_t totalSamples = numFramesAvailable * m_channelCount;

                    // Calculate RMS energy
                    float rms = CalculateRMS(samples, totalSamples);
                    m_rmsEnergy.store(rms, std::memory_order_relaxed);

                    // Debug: print first few samples occasionally
                    static int debugCounter = 0;
                    if (debugCounter++ % 100 == 0) // Every 100th packet
                    {
                        std::cout << "Audio packet: " << numFramesAvailable << " frames, "
                                  << totalSamples << " samples, RMS: " << rms << std::endl;
                        if (totalSamples > 0)
                        {
                            std::cout << "  First 4 samples: "
                                      << samples[0] << ", " << samples[1] << ", "
                                      << samples[2] << ", " << samples[3] << std::endl;
                        }
                    }

                    // Write to circular buffer (mono conversion)
                    // For stereo, average the channels
                    if (m_channelCount == 2)
                    {
                        float monoBuffer[4096];
                        size_t monoCount = numFramesAvailable;
                        for (size_t i = 0; i < monoCount; i++)
                        {
                            monoBuffer[i] = (samples[i * 2] + samples[i * 2 + 1]) * 0.5f;
                        }
                        m_buffer.Write(monoBuffer, monoCount);
                    }
                    else
                    {
                        m_buffer.Write(samples, numFramesAvailable);
                    }
                }

                // Release the buffer
                hr = m_captureClient->ReleaseBuffer(numFramesAvailable);
                if (FAILED(hr))
                {
                    std::cerr << "ReleaseBuffer failed" << std::endl;
                    break;
                }

                // Check for more packets
                hr = m_captureClient->GetNextPacketSize(&packetLength);
                if (FAILED(hr))
                {
                    break;
                }
            }

            // Sleep briefly to avoid busy-wait (10ms)
            Sleep(10);
        }

        std::cout << "Capture thread stopped" << std::endl;
    }

    float AudioCapture::CalculateRMS(const float *samples, size_t count)
    {
        if (count == 0)
            return 0.0f;

        double sum = 0.0;
        for (size_t i = 0; i < count; i++)
        {
            double sample = static_cast<double>(samples[i]);
            sum += sample * sample;
        }

        double mean = sum / count;
        float rms = static_cast<float>(std::sqrt(mean));

        // Clamp to 0.0 - 1.0 range
        return (rms > 1.0f) ? 1.0f : rms;
    }

    void AudioCapture::Cleanup()
    {
        if (m_captureClient)
        {
            m_captureClient->Release();
            m_captureClient = nullptr;
        }

        if (m_audioClient)
        {
            m_audioClient->Release();
            m_audioClient = nullptr;
        }

        if (m_device)
        {
            m_device->Release();
            m_device = nullptr;
        }

        if (m_deviceEnumerator)
        {
            m_deviceEnumerator->Release();
            m_deviceEnumerator = nullptr;
        }

        if (m_waveFormat)
        {
            CoTaskMemFree(m_waveFormat);
            m_waveFormat = nullptr;
        }
    }

} // namespace EdgeGlow