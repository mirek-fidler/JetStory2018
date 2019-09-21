#include "JetStory.h"

const Zoom credits_zoom(100, 625);

void CreditsCtrl::Layout()
{
	limity = text.GetHeight(credits_zoom, GetSize().cx);
//	Rect r = GetScreenRect(); window_size
	
	Rect r = GetScreenRect();
	
	Image m = TitleScreen();
	
	Point topleft = (m.GetSize() - window_size) / 2;

	Image h = Copy(m, RectC(r.left + topleft.x, r.top, r.GetWidth(), 60));
	Image h2 = Copy(m, RectC(r.left + topleft.x, r.bottom - 60, r.GetWidth(), 60));
	Size sz = h.GetSize();
	ImageBuffer ib(h), ib2(h2);
	for(int y = 0; y < sz.cy; y++) {
		RGBA *t = ib[sz.cy - y - 1];
		RGBA *t2 = ib2[sz.cy - y - 1];
		for(int x = 0; x < sz.cx; x++) {
			t->a = y * 255 / sz.cy;
			t++;
			t2->a = (sz.cy - y - 1) * 255 / sz.cy;
			t2++;
		}
	}
	Premultiply(~ib, ~ib, ib.GetLength());
	Premultiply(~ib2, ~ib2, ib2.GetLength());
	top = ib;
	bottom = ib2;
}

void CreditsCtrl::Paint(Draw& w)
{
	w.Offset(0, -y);
	PaintInfo pi;
	pi.zoom = credits_zoom;
	text.Paint(w, 0, 0, GetSize().cx, pi);
	w.End();
	w.DrawImage(0, 0, top);
	w.DrawImage(0, GetSize().cy - bottom.GetHeight(), bottom);
}

CreditsCtrl::CreditsCtrl()
{
	text = ParseQTF("[@W " + GetTopic("topic://JetStory/app/credits_en-us").text);
	tm.Set(-30, [=] { y++; if(y > limity) y = -GetSize().cy; Refresh(); });
}
