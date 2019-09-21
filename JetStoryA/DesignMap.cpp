#include "JetStory.h"

struct MapDesigner : TopWindow {
	enum { STANDARD, RECT, FILL };
	int blocksz = 32;
	const int MAXBLOCK = 29;
	
	int current = 0;
	int mode = STANDARD;
	
	String base;
	Point  basepoint;
	
	Vector<String> undo;
	
	Image cursor_image;

	virtual void Layout();
	virtual void Paint(Draw& w);
	virtual bool Key(dword key, int count);
	virtual void RightDown(Point p, dword flags);
	virtual void MouseWheel(Point p, int zdelta, dword keyflags);
	virtual void LeftDown(Point p, dword flags);
	virtual void LeftUp(Point p, dword flags);
	virtual void MouseMove(Point p, dword flags);
	virtual Image CursorImage(Point p, dword);
	
	Image GetBlockImage(int i);
	Point GetPos(Point p);
	int&  At(Point p);
	void  SyncCursorImage();
	void  Undo();
	String ToString();
	void   FromString(const String& data);
	
	ScrollBars sb;
	
	MapDesigner();
};

MapDesigner::MapDesigner()
{
	AddFrame(sb);
	FullScreen();
	sb.WhenScroll = [=] { Refresh(); };
	SyncCursorImage();
}

String MapDesigner::ToString()
{
	String data;
	for(int y = 0; y < MAPY; y++)
		for(int x = 0; x < MAPX; x++)
			data.Cat(jetstory[y][x]);
	return data;
}

void MapDesigner::FromString(const String& data)
{
	byte *s = (byte *)~data;
	for(int y = 0; y < MAPY; y++)
		for(int x = 0; x < MAPX; x++)
			jetstory[y][x] = *s++;
	Refresh();
}

void MapDesigner::Undo()
{
	if(undo.GetCount() == 0)
		return;
	FromString(ZDecompress(undo.Pop()));
}

void MapDesigner::RightDown(Point p, dword flags)
{
	current = At(p);
	SyncCursorImage();
}

void MapDesigner::MouseMove(Point p, dword flags)
{
	if(flags & K_MOUSELEFT) {
		if(mode == STANDARD) {
			At(p) = current;
			Refresh();
			return;
		}
		FromString(base);
		p = GetPos(p);
		Rect r;
		r.left = min(p.x, basepoint.x);
		r.right = max(p.x, basepoint.x);
		r.top = min(p.y, basepoint.y);
		r.bottom = max(p.y, basepoint.y);
		if(mode == FILL) {
			for(int y = r.top; y <= r.bottom; y++)
				for(int x = r.left; x <= r.right; x++)
					jetstory[y][x] = current;
		}
		else {
			for(int y = r.top; y <= r.bottom; y++)
				jetstory[y][r.left] = jetstory[y][r.right] = current;
			for(int x = r.left; x <= r.right; x++)
				jetstory[r.top][x] = jetstory[r.bottom][x] = current;
		}
		Refresh();
	}
}

void MapDesigner::MouseWheel(Point p, int zdelta, dword keyflags)
{
	current = (current + sgn(zdelta)) % (MAXBLOCK + 1);
	SyncCursorImage();
}

void MapDesigner::LeftDown(Point p, dword flags)
{
	mode = STANDARD;
	base = ToString();
	String data = ZCompress(base);
	if(!(undo.GetCount() && undo.Top() == data))
		undo.Add(data);
	basepoint = GetPos(p);
	if(flags & K_ALT) {
		p = GetPos(p);
		p.x = clamp(p.x + (int)Random(32) - 16, 0, (int)MAPX - 1);
		p.y = clamp(p.y + (int)Random(32) - 16, 0, (int)MAPY - 1);
		for(Point dir : { Point(-1, 0), Point(1, 0), Point(0, 1), Point(0, -1) }) {
			int l = Random(5);
			for(int i = 0; i < l; i++) {
				Point a = p + dir * i;
				if(a.x >= 0 && a.x < MAPX && a.y >= 0 && a.y < MAPY)
					jetstory[a.y][a.x] = current;
			}
		}
		return;
	}
		
	At(p) = current;
	mode = flags & K_SHIFT ? RECT : flags & K_CTRL ? FILL : STANDARD;
	Refresh();
}

void MapDesigner::LeftUp(Point p, dword flags)
{
}

Image MapDesigner::CursorImage(Point p, dword)
{
	return cursor_image;
}

Point MapDesigner::GetPos(Point p)
{
	return p / blocksz + sb.Get();
}

int& MapDesigner::At(Point p)
{
	p = GetPos(p);
	return jetstory[clamp(p.y, 0, (int)MAPY)][clamp(p.x, 0, (int)MAPX)];
}

Image MapDesigner::GetBlockImage(int i)
{
	return CachedRescale(BlocksImg::Get(clamp(i, 0, MAXBLOCK)), Size(blocksz, blocksz));
}

void MapDesigner::SyncCursorImage()
{
	cursor_image = GetBlockImage(current);
	ImageBuffer ib(cursor_image);
	ib.SetHotSpot(Point(blocksz, blocksz) / 2);
	Unmultiply(ib);
	for(RGBA *t = ~ib; t < ib.End(); t++)
		t->a = 120;
	Premultiply(ib);
	int h = blocksz / 2;
	for(int i = 0; i < 5; i++) {
		ib[h - i][h] = White();
		ib[h + i][h] = White();
		ib[h][h - i] = White();
		ib[h][h + i] = White();
	}
	cursor_image = ib;
}

void MapDesigner::Layout()
{
	sb.SetTotal(MAPX, MAPY);
	sb.SetPage(GetSize() / blocksz);
}

bool MapDesigner::Key(dword key, int count)
{
	switch(key) {
	case K_ESCAPE:
		Break();
		break;
	case K_LEFT:
		sb.LineLeft();
		break;
	case K_RIGHT:
		sb.LineRight();
		break;
	case K_UP:
		sb.LineUp();
		break;
	case K_DOWN:
		sb.LineDown();
		break;
	case K_CTRL_Z:
		Undo();
		break;
	default:
		return false;
	}
	return true;
}

void MapDesigner::Paint(Draw& w)
{
	w.DrawRect(GetSize(), Black());
	Size sz = GetSize();
	for(int x = 0; x < sz.cx; x += blocksz)
		for(int y = 0; y < sz.cy; y += blocksz) {
			w.DrawImage(x, y, GetBlockImage(At(Point(x, y))));
			w.DrawRect(x, y, blocksz, 1, White());
			w.DrawRect(x, y, 1, blocksz, White());
		}
}

void SaveMap()
{
	String h;
	for(int y = 0; y < MAPY; y++) {
		h << "{ ";
		for(int x = 0; x < MAPX; x++) {
			h << jetstory[y][x] << ", ";
		}
		h << "},\r\n";
	}
	SaveFile(GetDataFile("map.i"), h);
}

void DesignMap()
{
	SDL2GUI gui;
	gui.Attach(sdl_window, glcontext);
	RunVirtualGui(gui, [&] {
		MapDesigner().Run();
	});
	gui.Detach();
	SaveMap();
}
