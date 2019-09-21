#include "JetStory.h"

double PanPos(Pointf pos)
{
	double x = pos.x - (ship.pos.x - window_size.cx / 2);
	return 1 - clamp(x / window_size.cx, 0.0, 1.0);
}

void PlaySound(int priority, double pan, const char *s, double volume, double freq)
{
	if(design_mode)
		return;
	volume *= 0.9;
	Sound snd;
	snd.Load(s);
	if(!IsNull(volume))
		snd.op[0].volume = volume;
	if(!IsNull(freq))
		snd.f = freq;
	snd.pan = pan;
	int ii = FindChannel(priority, 1, snd.op[0].volume);
	if(ii < 0)
		return;
	SetChannel(ii, snd, priority);
}
