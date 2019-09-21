#ifndef _SoundMaker2_SoundMaker_h_
#define _SoundMaker2_SoundMaker_h_

#include <CtrlLib/CtrlLib.h>
#include <Synth/Synth.h>

#define LAYOUTFILE <SoundMaker/SoundMaker.lay>
#include <CtrlCore/lay.h>

using namespace Upp;

struct OperatorCtrl : WithOperatorLayout<ParentCtrl> {
	void Set(FMOP op);
	FMOP Get() const;
	
	OperatorCtrl();
};

struct ScopeCtrl : Ctrl {
	Vector<float> data;
	ScrollBar     sb;

	virtual void Paint(Draw& w);
	virtual void Layout();

	void SetSb();
	void Pick(Vector<float>&& data);
	
	ScopeCtrl();
};

struct SoundMakerDlg : WithSoundMakerLayout<TopWindow> {
	virtual bool Key(dword key, int count);

	typedef SoundMakerDlg CLASSNAME;
	SoundMakerDlg();

	StaticRect timber;
	
	Vector<Tuple<EditDouble *, SliderCtrl *, double>> param;
	
	Button tone[72];
	
	TimeCallback       tm;
//	Vector<SoundPoint> sp;
	int                tm0 = 0;
	double             f = 440;

	WithOperatorNameLayout<ParentCtrl> opname1, opname2;
	
	OperatorCtrl       op[OPCOUNT];

	ScopeCtrl          scope;

	void Sync();
	void AddParam(const char *id, EditDouble& p, double max);
	void Param();
	
	void Set(const Sound& s);
};

#endif
