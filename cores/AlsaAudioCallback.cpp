#include "AlsaAudioCallback.h"

#include <alsa/asoundlib.h>
#include <stdio.h>

AlsaAudioCallback::AlsaAudioCallback()
 : pcm_handle_(NULL),
   pcm_params_(NULL) {
}

AlsaAudioCallback::~AlsaAudioCallback() {
  if (NULL != pcm_handle_) {
    //snd_pcm_drain(pcm_handle_);
    snd_pcm_close(pcm_handle_);
    pcm_handle_ = NULL;
  }
}

void AlsaAudioCallback::OnInitialize(int iChannels,
                                     int iSamplesPerSec,
                                     int iBitsPerSample) {
  printf("OnInitialize: %d channels, %d samples/sec, %d bits/sample\n",
         iChannels,
         iSamplesPerSec,
         iBitsPerSample);

  if (0 == iSamplesPerSec) {
    printf("iSamplesPerSec %d -> 44100\n", iSamplesPerSec);
    iSamplesPerSec = 44100;
  }

  int rc = snd_pcm_open(&pcm_handle_, "default", SND_PCM_STREAM_PLAYBACK, 0);
  if (rc < 0) {
    fprintf(stderr, "Unable to open pcm device: %s\n", snd_strerror(rc));
    exit(1);
  }

  snd_pcm_hw_params_alloca(&pcm_params_);
  snd_pcm_hw_params_any(pcm_handle_, pcm_params_);
  snd_pcm_hw_params_set_access(pcm_handle_,
                               pcm_params_,
                               SND_PCM_ACCESS_RW_INTERLEAVED);
  snd_pcm_hw_params_set_format(pcm_handle_,
                               pcm_params_,
                               SND_PCM_FORMAT_S16_LE);
  snd_pcm_hw_params_set_channels(pcm_handle_,
                                 pcm_params_,
                                 static_cast<unsigned>(iChannels));

  snd_pcm_hw_params_set_rate(pcm_handle_, pcm_params_, iSamplesPerSec, 0);

  // TODO(jmeady): Check if OMXPlayer can adjust this.

  snd_pcm_uframes_t frames = 4608 / 4;
  int frames_dir = 0;
  snd_pcm_hw_params_set_period_size_near(pcm_handle_,
                                         pcm_params_,
                                         &frames,
                                         &frames_dir);
  printf("Requested period 1152, got period %u with dir %i\n",
         frames,
         frames_dir);

  printf("Can pause: %d\n", snd_pcm_hw_params_can_pause(pcm_params_));

  /*unsigned long buffer_size = 0;
  snd_pcm_hw_params_get_buffer_size(pcm_params_, &buffer_size);
  printf("Buffer time defaults to %u\n", buffer_size);
  buffer_size = frames * 128 * 8;
  printf("Asking for %u\n", buffer_size);
  rc = snd_pcm_hw_params_set_buffer_size_near(pcm_handle_, pcm_params_, &buffer_size);
  printf("Buffer size returned %u\n", buffer_size);
  snd_pcm_hw_params_get_buffer_size(pcm_params_, &buffer_size);
  printf("Result of setting buffer: %d -> %u\n", rc, buffer_size);
  if (rc < 0) {
    fprintf(stderr, "Unable to set buffer size: %s\n", snd_strerror(rc));
    exit(1);
  }*/

  rc = snd_pcm_hw_params(pcm_handle_, pcm_params_);
  if (rc < 0) {
    fprintf(stderr, "Unable to set hw params: %s\n", snd_strerror(rc));
    exit(1);
  }

  snd_pcm_nonblock(pcm_handle_, 1);
  snd_pcm_pause(pcm_handle_, 1);
}

void AlsaAudioCallback::OnAudioData(const unsigned char* pAudioData,
                                    int iAudioDataLength) {
  static int total = 0;
  total += iAudioDataLength;
  printf("Total: %d\n", total);
  //printf("OnAudioData: %d\n", iAudioDataLength);

  int rc = snd_pcm_writei(pcm_handle_, pAudioData, iAudioDataLength / 4);
  if (rc == -EPIPE) {
    fprintf(stderr, "ALSA underrun\n");
    snd_pcm_prepare(pcm_handle_);
  } else if (rc < 0) {
    fprintf(stderr, "ALSA writei error: %s\n", snd_strerror(rc));
  } else if (rc != iAudioDataLength / 4) {
    fprintf(stderr,
        "ALSA write short, only wrote %d frames\n", rc);
  }
}

void AlsaAudioCallback::Pause() {
  printf("Pausing ALSA\n");
  snd_pcm_pause(pcm_handle_, 1);
}

void AlsaAudioCallback::Resume() {
  printf("Resuming ALSA\n");
  snd_pcm_pause(pcm_handle_, 0);
}
