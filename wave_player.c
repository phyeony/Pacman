/*
 *  Small program to read a 16-bit, signed, 44.1kHz wave file and play it.
 *  Written by Brian Fraser, heavily based on code found at:
 *  http://www.alsa-project.org/alsa-doc/alsa-lib/_2test_2pcm_min_8c-example.html
 */

#include <alsa/asoundlib.h>
#include <stdbool.h>
#include <alloca.h>
#include "wave_player.h"

// File used for play-back:
// If cross-compiling, must have this file available, via this relative path,
// on the target when the application is run. This example's Makefile copies the wave-files/
// folder along with the executable to ensure both are present.
#define INTERMISSION_FILE "audio_files/pacman_intermission_fixed.wav"
#define CHOMP_FILE "audio_files/pacman_chomp_fixed.wav"
#define DEATH_FILE "audio_files/pacman_death_fixed.wav"
#define EAT_GHOST_FILE "audio_files/pacman_eatghost_fixed.wav"
#define EAT_FRUIT_FILE "audio_files/pacman_eatfruit_fixed.wav"
#define EXTRAPAC_FILE "audio_files/pacman_extrapac_fixed.wav"

#define SAMPLE_RATE   44100
#define NUM_CHANNELS  1
#define SAMPLE_SIZE   (sizeof(short)) 	// bytes per sample
#define DEFAULT_VOLUME 80
#define MAX_VOLUME 100

// Store data of a single wave file read into memory.
// Space is dynamically allocated; must be freed correctly!
typedef struct {
	int numSamples;
	short *pData;
} wavedata_t;

static snd_pcm_t *handle;
static int volume = DEFAULT_VOLUME;

// Prototypes:
snd_pcm_t *Audio_openDevice();
void Audio_readWaveFileIntoMemory(char *fileName, wavedata_t *pWaveStruct);
void Audio_playFile(wavedata_t *pWaveData);

wavedata_t intermissionFile;
wavedata_t chompFile;
wavedata_t deathFile;
wavedata_t eatGhostFile;
wavedata_t eatFruitFile;
wavedata_t extraPacFile;

void WavePlayer_init(void)
{
	printf("Init Wave Player...\n");
	setVolume(DEFAULT_VOLUME);

	// Configure Output Device
	handle = Audio_openDevice();

	// Load wave file we want to play:
	Audio_readWaveFileIntoMemory(INTERMISSION_FILE, &intermissionFile);
	Audio_readWaveFileIntoMemory(CHOMP_FILE, &chompFile);
	Audio_readWaveFileIntoMemory(DEATH_FILE, &deathFile);
	Audio_readWaveFileIntoMemory(EAT_GHOST_FILE, &eatGhostFile);
	Audio_readWaveFileIntoMemory(EAT_FRUIT_FILE, &eatFruitFile);
	Audio_readWaveFileIntoMemory(EXTRAPAC_FILE, &extraPacFile);
}

void WavePlayer_playIntermission(void)
{
	Audio_playFile(&intermissionFile);
}

void WavePlayer_playChomp(void)
{
	Audio_playFile(&chompFile);
}

void WavePlayer_playDeath(void)
{
	Audio_playFile(&deathFile);
}

void WavePlayer_playEatGhost(void)
{
	Audio_playFile(&eatGhostFile);
}

void WavePlayer_playEatFruit(void)
{
	Audio_playFile(&eatFruitFile);
}

void WavePlayer_playExtraPac(void)
{
	Audio_playFile(&extraPacFile);
}

void WavePlayer_cleanup(void)
{
	printf("Cleanup Wave Player...\n");

	// Cleanup, letting the music in buffer play out (drain), then close and free.
	snd_pcm_drain(handle);
	snd_pcm_hw_free(handle);
	snd_pcm_close(handle);

	free(intermissionFile.pData);
	free(chompFile.pData);
	free(deathFile.pData);
	free(eatGhostFile.pData);
	free(eatFruitFile.pData);
	free(extraPacFile.pData);

	printf("Done stopping audio...\n");
}

// Open the PCM audio output device and configure it.
// Returns a handle to the PCM device; needed for other actions.
snd_pcm_t *Audio_openDevice()
{
	// Open the PCM output
	int err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
	if (err < 0) {
		printf("Play-back open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	// Configure parameters of PCM output
	err = snd_pcm_set_params(handle,
			SND_PCM_FORMAT_S16_LE,
			SND_PCM_ACCESS_RW_INTERLEAVED,
			NUM_CHANNELS,
			SAMPLE_RATE,
			1,			// Allow software resampling
			50000);		// 0.05 seconds per buffer
	if (err < 0) {
		printf("Play-back configuration error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	return handle;
}

// Read in the file to dynamically allocated memory.
// !! Client code must free memory in wavedata_t !!
void Audio_readWaveFileIntoMemory(char *fileName, wavedata_t *pWaveStruct)
{
	assert(pWaveStruct);

	// Wave file has 44 bytes of header data. This code assumes file
	// is correct format.
	const int DATA_OFFSET_INTO_WAVE = 44;

	// Open file
	FILE *file = fopen(fileName, "r");
	if (file == NULL) {
		fprintf(stderr, "ERROR: Unable to open file %s.\n", fileName);
		exit(EXIT_FAILURE);
	}

	// Get file size
	fseek(file, 0, SEEK_END);
	int sizeInBytes = ftell(file) - DATA_OFFSET_INTO_WAVE;
	fseek(file, DATA_OFFSET_INTO_WAVE, SEEK_SET);
	pWaveStruct->numSamples = sizeInBytes / SAMPLE_SIZE;

	// Allocate Space
	pWaveStruct->pData = malloc(sizeInBytes);
	if (pWaveStruct->pData == NULL) {
		fprintf(stderr, "ERROR: Unable to allocate %d bytes for file %s.\n",
				sizeInBytes, fileName);
		exit(EXIT_FAILURE);
	}

	// Read data:
	int samplesRead = fread(pWaveStruct->pData, SAMPLE_SIZE, pWaveStruct->numSamples, file);
	if (samplesRead != pWaveStruct->numSamples) {
		fprintf(stderr, "ERROR: Unable to read %d samples from file %s (read %d).\n",
				pWaveStruct->numSamples, fileName, samplesRead);
		exit(EXIT_FAILURE);
	}

	fclose(file);
}

// Play the audio file (blocking)
void Audio_playFile(wavedata_t *pWaveData)
{
	// If anything is waiting to be written to screen, can be delayed unless flushed.
	fflush(stdout);

	// Write data and play sound (blocking)
	snd_pcm_sframes_t frames = snd_pcm_writei(handle, pWaveData->pData, pWaveData->numSamples);

	// Check for errors
	if (frames < 0)
		frames = snd_pcm_recover(handle, frames, 0);
	if (frames < 0) {
		fprintf(stderr, "ERROR: Failed writing audio with snd_pcm_writei(): %li\n", frames);
		exit(EXIT_FAILURE);
	}
	if (frames > 0 && frames < pWaveData->numSamples)
		printf("Short write (expected %d, wrote %li)\n", pWaveData->numSamples, frames);
}

// Function copied from:
// http://stackoverflow.com/questions/6787318/set-alsa-master-volume-from-c-code
// Written by user "trenki".
void setVolume(int newVolume)
{
	// Ensure volume is reasonable; If so, cache it for later getVolume() calls.
	if (newVolume < 0 || newVolume > MAX_VOLUME) {
		printf("ERROR: Volume must be between 0 and 100.\n");
		return;
	}
	volume = newVolume;

    long min, max;
    snd_mixer_t *volHandle;
    snd_mixer_selem_id_t *sid;
    const char *card = "default";
    const char *selem_name = "PCM";

    snd_mixer_open(&volHandle, 0);
    snd_mixer_attach(volHandle, card);
    snd_mixer_selem_register(volHandle, NULL, NULL);
    snd_mixer_load(volHandle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    snd_mixer_elem_t* elem = snd_mixer_find_selem(volHandle, sid);

    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    snd_mixer_selem_set_playback_volume_all(elem, volume * max / 100);

    snd_mixer_close(volHandle);
}

int getVolume()
{
	// Return the cached volume; good enough unless someone is changing
	// the volume through other means and the cached value is out of date.
	return volume;
}

// Control the output volume in range [0, 100] (inclusive), default 80
void controlVolume(bool increase)
{
	if (increase) {
		volume = volume + 5 >= 100 ? 100 : volume + 5;
	}
	else {
		volume = volume - 5 <= 0 ? 0 : volume - 5;
	}

	setVolume(volume);
}
