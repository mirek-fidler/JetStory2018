#include "SoundMaker.h"

void ScopeCtrl::SetSb()
{
	Size sz = GetSize();
	sb.SetPage(sz.cx);
	sb.SetTotal(data.GetCount());
}

void ScopeCtrl::Paint(Draw& w)
{
	Size sz = GetSize();
	w.DrawRect(sz, White());
	int y0 = sz.cy / 2;
	w.DrawRect(0, y0, sz.cx, 1, LtGray());
	int x0 = sb.Get();
	for(int i = 0; i < sz.cx; i++) {
		int x = x0 + i;
		if(x >= 0 && x < data.GetCount()) {
			w.DrawRect(i, int((y0 - 1) * clamp(-(double)data[x], -1.0, 1.0) + y0), 1, 1, Black());
		}
	}
}

void ScopeCtrl::Pick(Vector<float>&& data_)
{
	data = pick(data_);
	SetSb();
	Refresh();
}

void ScopeCtrl::Layout()
{
	SetSb();
}

ScopeCtrl::ScopeCtrl()
{
	sb << [=] { Refresh(); };
	SetFrame(ViewFrame());
	AddFrame(sb.Horz());
}
