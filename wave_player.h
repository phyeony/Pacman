#ifndef WAVE_PLAYER_H
#define WAVE_PLAYER_H

#include <stdbool.h>

void WavePlayer_init(void);
void WavePlayer_cleanup(void);

void WavePlayer_playIntermission(void);
void WavePlayer_playChomp(void);
void WavePlayer_playDeath(void);
void WavePlayer_playEatGhost(void);
void WavePlayer_playEatFruit(void);
void WavePlayer_playExtraPac(void);

void setVolume(int volume);
int getVolume(void);
void controlVolume(bool increase);

#endif