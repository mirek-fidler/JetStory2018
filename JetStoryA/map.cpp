#include "JetStory.h"
#define IMAGECLASS BlocksImg
#define IMAGEFILE <JetStoryA/Blocks.iml>
#include <Draw/iml_source.h>

int jetstory[MAPY][MAPX] = {
#include "map.i"
};

int blocks[MAPY][MAPX];

Vector<Image> block;

bool blockmap[64][4][4] = {};

Vector< Vector<int> > rightmap;
Vector<int> righttop;

Vector< Vector<int> > leftmap;
Vector<int> lefttop;

Vector< Vector<int> > rustmap;

Size training_size;
int  training_left;

void BlockMap()
{
	Image t = GtImg::training_map();
	
	for(int y = 100; y < 200; y++)
		for(int x = 0; x < MAPX; x++)
			jetstory[y][x] = 4;
	training_size = t.GetSize();
	training_left = -training_size.cx * BLOCKSIZE;
	for(int x = 0; x < training_size.cx; x++)
		for(int y = 0; y < training_size.cy; y++) {
			RGBA m = t[y][x];
			int b = 30;
			if(m.a == 0)
				b = 0;
			else {
				Color c = m;
				b = decode(c, LtGray(), 31, Yellow(), 6, 30);
			}
			jetstory[y + 128][x] = b;
		}
	
	RGBA black = Black();
	for(int i = 0; i <= 31; i++) {
		Image mm = BlocksImg::Get(i);
		for(int x = 0; x < 32; x++)
			for(int y = 0; y < 32; y++) {
				if(mm[y][x] != black && mm[y][x].a)
					blockmap[i][y / 8][x / 8] = true;
			}
	}
}

void PaintMap(Size sz, Point topleft, Event<int, int, const Image&> DrawImage)
{
	Point tl = topleft / 64 - 1;
	Point off0 = tl * 64 - topleft;
	Size nsz = window_size / 64 + 2;

	Size gsz = BlocksImg::granit().GetSize();
	Size rmsz = 2 * (gsz / 64);

	Size dsz = BlocksImg::dirt().GetSize();
	Size ltsz = 2 * (dsz / 64);

	Size rsz = GtImg::Rust().GetSize();
	Size rtsz = 2 * (rsz / 64);
	
	for(int x = 0; x <= nsz.cx; x++)
		for(int y = 0; y <= nsz.cy; y++) {
			Point xy(x, y);
			Point b = xy + tl;
			int pbx = b.x + 1024*1024*1024;
			int tx = training_size.cx + b.x;
			int bi = b.y < 0 ? 0 :
			         tx >= 0 && tx < training_size.cx && b.y >= 1 && b.y < training_size.cy ? blocks[b.y + 128][tx] :
			         b.x < 0 ? b.y > 100 ? rustmap.At(b.y % rtsz.cy).At(pbx % rtsz.cx)
			                 : b.y == 0 ? lefttop.At(pbx % ltsz.cx) : leftmap.At(b.y % ltsz.cy).At(pbx % ltsz.cx) :
			         b.x >= MAPX ? b.y == 0 ? righttop.At(b.x % rmsz.cx) : rightmap.At(b.y % rmsz.cy).At(b.x % rmsz.cx) :
			         b.y >= MAPY ? 0 :
			         blocks[b.y][b.x];
			if(bi) {
				Point p = 64 * xy + off0;
				DrawImage(p.x, p.y, block.Get(bi, Image()));
			}
		}
}

bool MapCollision(Point p)
{
	if(p.y < 0)
		return false;
	if(p.x < 0 || p.x >= BLOCKSIZE * MAPX)
		return true;
	p /= 16;
	int q = jetstory[p.y >> 2][p.x >> 2];
	return q && blockmap[q][p.y & 3][p.x & 3];
}

bool MapCollision(Rectf ar)
{
	for(Rect r : training_block)
		if(r.Intersects(ar))
			return true;
	Rect r;
	if(ar.left < 0 && ar.left >= training_left && ar.top > 0) {
		if(ar.right > 0 && !training)
			ar.left = 0; // ugly fix when going from training maze to real maze
		else
			ar.Offset(-training_left, MAPX * BLOCKSIZE);
	}
	   
	r.left = (int)floor(ar.left);
	r.top = (int)floor(ar.top);
	r.right = (int)floor(ar.right);
	r.bottom = (int)floor(ar.bottom);
	if(r.bottom < 0)
		return false;
	if(r.left < 0 || r.left >= BLOCKSIZE * MAPX)
		return true;
	r.right--;
	r.bottom--;
	r /= 16;
	for(int x = r.left; x <= r.right; x++)
		for(int y = r.top; y <= r.bottom; y++) {
			if(y >= 0 && y < 4 * MAPY && x >= 0 && x < 4 * MAPX) {
				int q = jetstory[y >> 2][x >> 2];
				if(q && blockmap[q][y & 3][x & 3]) {
					return true;
				}
			}
		}
	return false;
}

bool MapCollision(Pointf pos, Sizef sz)
{
	return MapCollision(Rectf(pos - sz / 2, sz));
}
