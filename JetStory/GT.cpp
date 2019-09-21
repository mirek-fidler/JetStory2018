#include "JetStory.h"

#define IMAGECLASS GtImg
#define IMAGEFILE <JetStory/Gt.iml>
#include <Draw/iml_source.h>

void DrawImageC(Pointf p, const Image& m)
{
	Size sz = m.GetSize();
	DrawImage(int(p.x - sz.cx / 2), int(p.y - sz.cy / 2), m);
}

struct GoldenTriangle : Enemy {
	virtual void SerializeKind(Stream& s) {}
	virtual Size GetSize() const { return Size(550, 550); }
	virtual bool IsActive() const { return false; }
	virtual void Paint(Point sp) {
		Pointf org = sp + GetSize() / 2;
		double angle = 0.002 * GameTick();
		Pointf p1 = Polar(angle);
		Pointf p2 = Polar(angle + M_2PI / 3);
		Pointf p3 = Polar(angle + 2 * M_2PI / 3);
		
		DrawTriangle(p1 * 75 + org, p2 * 75 + org, p3 * 75 + org, Yellow());
		
		DrawImageC(p1 * 90 + org, GtImg::CYBEXLAB());
		DrawImageC(p2 * 90 + org, GtImg::FUXOFT());
		DrawImageC(p3 * 90 + org, GtImg::TRC());
	}

	virtual void Do() {}
	GoldenTriangle() { image = GtImg::icon(); shield = explosion_size = 70; z = 1; Missile(); }
};

REGISTER(GoldenTriangle);
