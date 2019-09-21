#include "SoundMaker.h"

OperatorCtrl::OperatorCtrl()
{
	CtrlLayout(*this);
	waveform.Add(WAVEFORM_SIN, "Sine");
	waveform.Add(WAVEFORM_SQUARE, "Square");
	waveform.Add(WAVEFORM_TRIANGLE, "Triangle");
	waveform.Add(WAVEFORM_SAWTOOTH, "Sawtooth");

	waveform.Add(WAVEFORM_SAXOPHONE, "Sax");
	waveform.Add(WAVEFORM_VIOLIN, "Violin");
	waveform.Add(WAVEFORM_DOUBLEBASS, "DoubleBass");
	waveform.Add(WAVEFORM_BANJO, "Banjo");
	waveform.Add(WAVEFORM_TRUMPET, "Trumpet");

	waveform.Add(WAVEFORM_BROWN, "Brown Noise");
	waveform.Add(WAVEFORM_WHITE, "White Noise");
}

void OperatorCtrl::Set(FMOP op)
{
	duration <<= op.duration;
	volume <<= op.volume;
	waveform <<= op.waveform;
	f <<= op.f;
	fdrift <<= op.fdrift;
	attack <<= op.attack;
	decay <<= op.decay;
	sustain <<= op.sustain;
	release <<= op.release;
}

FMOP OperatorCtrl::Get() const
{
	FMOP op;
	op.duration = ~duration;
	op.volume = ~volume;
	op.waveform = ~waveform;
	op.f = ~f;
	op.fdrift = ~fdrift;
	op.attack = ~attack;
	op.decay = ~decay;
	op.sustain = ~sustain;
	op.release = ~release;
	return op;
}
