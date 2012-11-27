#ifndef ALSA_AUDIO_CALLBACK
#define ALSA_AUDIO_CALLBACK

#include "IAudioCallback.h"

extern "C" {
struct _snd_pcm;
typedef struct _snd_pcm snd_pcm_t;

struct _snd_pcm_hw_params;
typedef struct _snd_pcm_hw_params snd_pcm_hw_params_t;
}

class OMXClock;

class AlsaAudioCallback : public IAudioCallback {
public:
  AlsaAudioCallback();
  virtual ~AlsaAudioCallback();
  virtual void OnInitialize(int iChannels, int iSamplesPerSec, int iBitsPerSample);
  virtual void OnAudioData(const unsigned char* pAudioData, int iAudioDataLength);

  void Pause();
  void Resume();

private:
  snd_pcm_t* pcm_handle_;
  snd_pcm_hw_params_t* pcm_params_;
};

#endif  // ALSA_AUDIO_CALLBACK
