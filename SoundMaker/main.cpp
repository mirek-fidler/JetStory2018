#include "SoundMaker.h"

// 277.180972598656:L1V100f1r0A100D100S100R100:L99999V0f1r0A100D100S100R100:L99999V0f1r0A100D100S100R100

bool SoundMakerDlg::Key(dword key, int count)
{
	switch(key) {
	case K_SHIFT_F5:
		stop.PseudoPush();
		return true;
	case K_F5:
		play.PseudoPush();
		return true;
	case K_CTRL_F5:
		playfrom.PseudoPush();
		return true;
	case K_CTRL_V:
		Sound s;
		s.Load(ReadClipboardText());
		Set(s);
		return true;
	}
	return false;
}

void SoundMakerDlg::Set(const Sound& s)
{
	for(int i = 0; i < OPCOUNT; i++) {
		this->op[i].Set(s.op[i]);
//		reverb <<= s.reverb;
//		delay <<= s.delay;
	}
}

SoundMakerDlg::SoundMakerDlg()
{
	CtrlLayout(opname1);
	CtrlLayout(opname2);

	CtrlLayout(*this, "");
	
	Sound s;
	Set(s);

	for(int i = 0; i < 72; i++)
		tone[i] << [=] {
			Sound s;
			for(int i = 0; i < OPCOUNT; i++)
				s.op[i] = op[i].Get();
//			s.reverb = ~reverb;
//			s.delay = ~delay;
			s.f = 65.406 * exp2(i / 12.0) / s.op[0].f;
			SetChannel(0, s);
		
			WriteClipboardText(s.Save());

			SoundGen sg;
			sg.Start(s);

			Vector<float> data;
			for(int t = 0; t < 5 * 44100 && (t < 10000 || sg.current_volume > 0.001); t++)
				data.Add(sg.Get());
			
			scope.Pick(pick(data));
		};

	play.Ok();
	play << [=] {
		SoundSequence h = ParseQSF(~track);
		PlayTempSequence(pick(h));
	};

	playfrom << [=] {
		String s = ~track;
		s.Replace("\r", "");
		int c = track.GetCursor();
		s.Insert(c, " !cursor ");
		SoundSequence h = ParseQSF(s);
		PlayTempSequence(pick(h));
	};
	
	stop << [=] { StopSequencer(); };

	paste << [=] {
		Sound s;
		s.Load(ReadClipboardText());
		Set(s);
	};
}

GUI_APP_MAIN
{
	InitSoundSynth();
/*
	static TuneMaker ss;
//	ss.Put(0, "130.81:L250V100R30::::", 100, 440, 1);
//	ss.Put(2, "130.81:L250V100R30::::", 100, 410, 1);
//	ss.Put(4, "130.81:L0.25V90A90D35S30R35:V160D40S20R20:V100f2A90D43S0R40", 100, 350, 1);

	ss.Sound("130.81:L250V100R30::::");
	ss.Tempo(180);
	ss.Add("/4 0245 /2 79 !130.81:L0.25V90A90D35S30R35:V160D40S20R20:V100f2A90D43S0R40 "
	       "/4 0245 /2 79 ____");
	
	PlaySequence(ss);
*/
	SoundMakerDlg dlg;

	String filename = GetHomeDirFile("track.qsf");
	dlg.track <<= LoadFile(filename);
	dlg.Run();
	SaveFile(filename, ~dlg.track);
	
	CloseSoundSynth();
}

// 130:L0.25V100f1r0A70D50S90R30:L99V90f1r0A70D50S0R100:L99V92f1r0A100D100S100R100
// 391.99:L0.25V100f1r0A55D50S100R30:L99V70f7r0A100D50S100R100:L99V110f2r0A100D100S100R100
// 277.18:L0.25V100A55D50R30:V70f7D50:V110f2

// explosion 3951.04:L0.25V100r-20A90D35S30R35:V120D20S20B:V2f2

// explosion 146.83:L0.25V100r-20A90D35S30R35:V120D20S20B:V100f2A90D43S0N

// gun fire 146.83:L0.25V100r-14A90D35S30R35:V150D30S20B:V100f2A90D43S0N

// bass 130.81:L0.25V90A90D35S30R35:V160D40S20R20:V100f2A90D43S0R40

#if 0

!261.62:L250V100A90R40:V91.60f2:V60f3W:: 123

!87.31:L50V100A98R50:V120D60S0B::: ****

JetStory intro
$180 !261.62:L750V100A70D40R40w0:V90f2A40D35R50w-4:V80f0.50D30S0w-4:V70f4w-4:w0 w25m9.q7.5.4w25m9:w0'qe.7weq9w25m9w29q7.5.4w25m0'.q2'.0'.e @7 w25m9.q7.5.4w25m9:w0'qe.7weq9w25m9w29q7.5.4w25m0'.q2'.0'.e"

JetStory quiet
/8 !261.62:L2000V100A80D40R40:V100f0.50D30S0:V100A35D45S0:V80f5r-2: 2 4 0 -9+ 2 4 0 -9+  24 /4 5424 /8 0 -9+  2 4 0 -9+  24 /4 5424 /8 0 -9+

Good drum
/4 !174.61:L200V100r-50A90D40R40:V130f7D45S0B:V90D55S0:f5r-2: -5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5

Melodic drums
/16 !174.61:L2000V100r-8D40S0R35:V90f3r-2D50S0B:V80r2D55S0N:f5r-2: - 0_5_0_5 05050505 _ 0_5_0_5 05050505 _ 0_5_0_5 05050505_ 0_5_0_5 05050505 _ 0_5_0_5 05050505 _ 0_5_0_5 05050505_ 0_5_0_5 05050505 _ 0_5_0_5 05050505 _ 0_5_0_5 05050505_

Good trumpet
261.62:L500V100A60D50S95R40w4:V100f0.50A98D50S0N:V90f0.50A30D40S0w-4:V60f5r-2w-4:V60f8w-4

//q $4(0&5&7 0&4&7 0&3&7 0&4&7);

#BIGDRUM !174.61:L200V100r-50A90D40R40:V130f7D45S0B:V90D55S0:f5r-2:
#SAX !261.62:L750V100A70D40R40w0:V90f2A40D35R50w-4:V80f0.50D30S0w-4:V70f4w-4:w0
#TRUMPET !261.62:L500V100A60D50S95R40w4:V100f0.50A98D50S0N:V90f0.50A30D40S0w-4:V60f5r-2w-4:V60f8w-4
#TOMTOM !174.61:L2000V100r-8D40S0R35:V90f3r-2D50S0B:V80r2D55S0N:f5r-2:
#BASS !130.81:L0.25V90A90D35S30R35:V160D40S20R20:V100f2A90D43S0R40
#SNARE !493.88:L250V100D45S0N:D45S0N:w-4:w-4:w-4

#TUNE w0247 5747 0247 574=
$SAX w $2TUNE [!shift:7 $2TUNE]  $SAX $2TUNE;
$BIGDRUM q $64(0);
$TOMTOM w - $16(_0_7 _0_7) +;
$SNARE w $16(_0_0 _00x00w);
#BASS_TUNE (w000x74 w000x47)
$BASS -- $4BASS_TUNE @7 $4BASS_TUNE @-7 $4BASS_TUNE $4BASS_TUNE;

#VIOLIN !698.45:L2500V100A60R40w1:L500A98D50S0R40N:V20f0.25A98D30S0w0:V70f5A90D40:f5r-2w0
#JBASS !110.00:L250V100A80D40S50R40Q:L500A98D50S0R40N:f0.25A98D30S0w0:f6A90D40:f5r-2w0

#endif
