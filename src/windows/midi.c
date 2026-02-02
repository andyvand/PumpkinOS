#include <windows.h>

#ifdef _WIN32_WCE
#include <mmsystem.h>
#include <timidity.h>
#endif

#include "sys.h"
#include "midi.h"
#include "debug.h"

DWORD playMIDIFile(HWND hWndNotify, LPSTR lpszMIDIFileName) {
#ifdef _WIN32_WCE
  MidSong *song = NULL;
  MidIStream *stream = NULL;
  MidSongOptions options;
  // Create a buffer for 16-bit stereo PCM data
  HWAVEOUT hWaveOut;
  WAVEHDR header[2]; // Two headers for double-buffering
  char buffer[2][8192];
  const int BUF_SIZE = 8192;
  WAVEFORMATEX wfx = { 0 };
  int current = 0;
  size_t bytesRead = 0;

  // Set output options (e.g., 44100Hz, 16-bit stereo)
  options.rate = 44100;
  options.format = MID_AUDIO_S16LSB;
  options.channels = 2;
  options.buffer_size = 4096;

  // Set WAV options
  wfx.wFormatTag = WAVE_FORMAT_PCM;
  wfx.nChannels = 2;              // Stereo
  wfx.nSamplesPerSec = 44100;     // 44.1kHz
  wfx.wBitsPerSample = 16;        // 16-bit
  wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
  wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

  // Open the device
  waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);

  if (hWaveOut == NULL){
	  debug(DEBUG_ERROR, "MIDI", "Could not open wave audio stream");
	  return 1;
  }
 
  // Open the stream
  stream = mid_istream_open_file(lpszMIDIFileName);
  if (stream == NULL) {
	  debug(DEBUG_ERROR, "MIDI", "Could not create stream for %s", lpszMIDIFileName);
	  return 1;
  }

  // Load the MIDI file
  song = mid_song_load(stream, &options);

  // Close the stream handle after loading (data is now in the song object)
  mid_istream_close(stream);

  if (song == NULL) {
    debug(DEBUG_ERROR, "MIDI", "Failed to load MIDI with TiMidity");

	return 1;
  }

  // Start playback (requires an audio device to feed the buffer to)
  mid_song_start(song);

  while ((bytesRead = mid_song_read_wave(song, buffer[current], BUF_SIZE)) > 0) {
	  // 1. Prepare the header
	  header[current].lpData = buffer[current];
	  header[current].dwBufferLength = bytesRead;
	  waveOutPrepareHeader(hWaveOut, &header[current], sizeof(WAVEHDR));

	  // 2. Write to the device
	  waveOutWrite(hWaveOut, &header[current], sizeof(WAVEHDR));

	  // 3. Wait for this buffer to finish before reusing it
	  // In a real app, use CALLBACK_EVENT for better performance
	  while (!(header[current].dwFlags & WHDR_DONE)) {
		  Sleep(1);
	  }

	  waveOutUnprepareHeader(hWaveOut, &header[current], sizeof(WAVEHDR));
	  current = 1 - current; // Switch buffers
  }

  waveOutClose(hWaveOut);
  mid_song_free(song);
#else
  UINT wDeviceID;
  DWORD dwReturn;
#ifdef _UNICODE
  WCHAR wMIDIFileName[MAX_PATH];
#if __STDC_WANT_SECURE_LIB__
  size_t filelen = MAX_PATH;
#endif
#endif
  MCI_OPEN_PARMS mciOpenParms;
  MCI_PLAY_PARMS mciPlayParms;
  MCI_STATUS_PARMS mciStatusParms;
  //MCI_SEQ_SET_PARMS mciSeqSetParms;

  // Open the device by specifying the device and filename.
  // MCI will attempt to choose the MIDI mapper as the output port.
#ifdef _UNICODE
  mciOpenParms.lpstrDeviceType = L"sequencer";

#if __STDC_WANT_SECURE_LIB__
  mbstowcs_s(&filelen, wMIDIFileName, MAX_PATH, lpszMIDIFileName, MAX_PATH);
#else
  mbstowcs(wMIDIFileName, lpszMIDIFileName, MAX_PATH);
#endif

  mciOpenParms.lpstrElementName = wMIDIFileName;
#else
  mciOpenParms.lpstrDeviceType = "sequencer";
  mciOpenParms.lpstrElementName = lpszMIDIFileName;
#endif

  if ((dwReturn = mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD_PTR)(LPVOID)&mciOpenParms)) != 0) {
    // Failed to open device. Don't close it; just return error.
    debug(DEBUG_ERROR, "MIDI", "failed to open device (%d)", (uint32_t)dwReturn);
    return dwReturn;
  }

  // The device opened successfully; get the device ID.
  wDeviceID = mciOpenParms.wDeviceID;

  // Check if the output port is the MIDI mapper.
  mciStatusParms.dwItem = MCI_SEQ_STATUS_PORT;
  if ((dwReturn = mciSendCommand(wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)(LPVOID)&mciStatusParms)) != 0) {
    debug(DEBUG_ERROR, "MIDI", "failed to get device status (%d)", (uint32_t)dwReturn);
    mciSendCommand(wDeviceID, MCI_CLOSE, 0, 0);
    return dwReturn;
  }

  // The output port is not the MIDI mapper. 
  if (LOWORD(mciStatusParms.dwReturn) != MIDI_MAPPER) {
    debug(DEBUG_ERROR, "MIDI", "output is not MIDI mapper (%u)", (uint32_t)mciStatusParms.dwReturn);
    mciSendCommand(wDeviceID, MCI_CLOSE, 0, 0);
    return (DWORD)mciStatusParms.dwReturn;
  }

  // Begin playback. The window procedure function for the parent 
  // window will be notified with an MM_MCINOTIFY message when 
  // playback is complete. At this time, the window procedure closes the device.
  mciPlayParms.dwCallback = (DWORD_PTR)hWndNotify;
  if ((dwReturn = mciSendCommand(wDeviceID, MCI_PLAY, MCI_NOTIFY, (DWORD_PTR)(LPVOID)&mciPlayParms)) != 0) {
    debug(DEBUG_ERROR, "MIDI", "failed to start MIDI play (%d)", (uint32_t)dwReturn);
    mciSendCommand(wDeviceID, MCI_CLOSE, 0, 0);
    return dwReturn;
  }
#endif

  return 0;
}
