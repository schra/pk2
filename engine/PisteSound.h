//#########################
//PisteEngine - PisteSound
//by Janne Kivilahti from Piste Gamez
//#########################

#ifndef P_SOUND
#define P_SOUND

int PisteSound_Start();
int PisteSound_End();

int PisteSound_LoadSFX(char* filename); //0 success, -1 fail
int PisteSound_FreeSFX(int index);
void PisteSound_ResetSFX();
void PisteSound_SetSFXVolume(int volume);
void PisteSound_PlaySFX(int index);
void PisteSound_PlaySFX(int index, int volume, int freq);

int PisteSound_StartMusic(char* filename);
void PisteSound_SetMusicVolume(int volume);
void PisteSound_StopMusic();


#endif

