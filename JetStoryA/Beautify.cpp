#include "JetStory.h"

extern Vector<Image> block;
extern Vector<int> righttop;
extern Vector< Vector<int> > rightmap;

extern Vector< Vector<int> > leftmap;
extern Vector<int> lefttop;

extern Vector< Vector<int> > rustmap;

Image Symmetry(const Image& m)
{
	Size isz = m.GetSize();
	Image r = CreateImage(2 * isz, Black());
	Copy(r, Point(0, 0), m, isz);
	Image mh = MirrorHorz(m);
	Copy(r, Point(isz.cx, 0), mh, isz);
	Copy(r, Point(0, isz.cy), MirrorVert(m), isz);
	Copy(r, Point(isz.cx, isz.cy), MirrorVert(mh), isz);
	return r;
}

Image BlendTiles(const Image& m, Size bsz, bool v2, bool mirror)
{
	Size isz = m.GetSize();
	Image r = CreateImage(2 * isz, Black());
	Copy(r, Point(0, 0), m, isz);
	Copy(r, Point(isz.cx, 0), mirror ? MirrorVert(m) : m, isz);
	Copy(r, Point(0, isz.cy), mirror ? MirrorHorz(m) : m, isz);
	Copy(r, Point(isz.cx, isz.cy), mirror ? MirrorVert(MirrorHorz(m)) : m, isz);

	ImageBuffer ib(r);
	for(int y = 0; y < 2 * isz.cy; y++)
		for(int x = 0; x < bsz.cx; x++) {
			auto blendx = [&] (int x1, int x2) {
				Color c1 = ib[y][x1];
				Color c2 = ib[y][x2];
				Color c3 = ib[(x + y) % (2 * isz.cy)][x1];
				Color c4 = ib[(x + y) % (2 * isz.cy)][x2];
				ib[y][x1] = Blend(c1, v2 ? c4 : c2, 128 - 128 * x / bsz.cx);
				ib[y][x2] = Blend(c2, v2 ? c3 : c1, 128 - 128 * x / bsz.cx);
			};
			blendx(x, 2 * isz.cx - x - 1);
			blendx(isz.cx - x - 1, isz.cx + x);
		}
	for(int y = 0; y < bsz.cy; y++)
		for(int x = 0; x < 2 * isz.cx; x++) {
			auto blendy = [&] (int y1, int y2) {
				Color c1 = ib[y1][x];
				Color c2 = ib[y2][x];
				Color c3 = ib[y1][(x + y) % (2 * isz.cx)];
				Color c4 = ib[y2][(x + y) % (2 * isz.cx)];
				ib[y1][x] = Blend(c1, v2 ? c4 : c2, 128 - 128 * y / bsz.cy);
				ib[y2][x] = Blend(c2, v2 ? c3 : c1, 128 - 128 * y / bsz.cy);
			};
			blendy(y, 2 * isz.cy - y - 1);
			blendy(isz.cy - y - 1, isz.cy + y);
		}
	return ib;
}

enum {
	LINE_LEFT          = 0x1,
	LINE_TOP           = 0x2,
	LINE_RIGHT         = 0x4,
	LINE_BOTTOM        = 0x8,
	
	CORNER_TOPLEFT     = 0x10,
	CORNER_TOPRIGHT    = 0x20,
	CORNER_BOTTOMLEFT  = 0x40,
	CORNER_BOTTOMRIGHT = 0x80,
	
	TUNNEL             = 0x100,
	UPPERCUT           = 0x200,
	PLATFORM           = 0x400,
	
	ROUGH              = 0x8000,
};

void AlphaCut(Image& m, const Image *a, int n)
{
	Size sz = m.GetSize();
	ImageBuffer ib(m);
	const Image& alpha = a[n & 1];
	for(int y = 0; y < sz.cy; y++) {
		RGBA *t = ib[y];
		const RGBA *s = alpha[y];
		for(int x = 0; x < sz.cx; x++) {
			int a = s->a + (s->a >> 7);
			t->a = (a * t->a) >> 8;
			t->r = (a * t->r) >> 8;
			t->g = (a * t->g) >> 8;
			t->b = (a * t->b) >> 8;
			s++;
			t++;
		}
	}
	m = ib;
}

void AlphaCut(Image& m, const Image& a)
{
	AlphaCut(m, &a, 0);
}

int GetBlock(const Image& src, int x, int y, dword flags)
{
	static VectorMap<int64, Image> tiled;
	static VectorMap<Tuple<int64, Point, dword>, int> blocki;
	
	int64 id = src.GetSerialId();

	Image m = tiled.Get(id, Image());
	if(IsNull(m)) {
		Size sz = src.GetSize();
		m = Copy(src, Size(sz.cx & ~31, sz.cy & ~31));
		m = BlendTiles(src, Size(32, 32));
		tiled.Add(src.GetSerialId(), m);
	}
	
	id = m.GetSerialId();
	Size isz = m.GetSize();
	Size n = isz / 64;
	x %= n.cx;
	y %= n.cy;
	
	Tuple<int64, Point, dword> key = MakeTuple(id, Point(x, y), flags);
	int q = blocki.Find(key);
	if(q >= 0)
		return blocki[q];
	
	m = Copy(m, RectC(x * 64, y * 64, 64, 64));
	static Image top[2], bottom[2], left[2], right[2], tl, tr, bl, br, rough[2];
	ONCELOCK {
		Image l = BlocksImg::lining();
		top[0] = l;
		top[1] = MirrorHorz(l);
		bottom[0] = MirrorVert(top[0]);
		bottom[1] = MirrorVert(top[1]);
		l = RotateClockwise(l);
		right[0] = l;
		right[1] = MirrorVert(l);
		left[0] = MirrorHorz(right[0]);
		left[1] = MirrorHorz(right[1]);
		
		tl = BlocksImg::corner();
		tr = MirrorHorz(tl);
		bl = MirrorVert(tl);
		br = MirrorHorz(bl);
		
		rough[0] = BlocksImg::rough();
		rough[1] = MirrorHorz(rough[0]);
	};

// #define PLASTIC

	if(flags & LINE_LEFT) {
	#ifdef PLASTIC
		ImageBuffer ib(m);
		for(int i = 0; i < 16; i++) {
			RGBA *t = ib[0] + i;
			int c = 5 * (16 - i);
			auto Do = [&](byte& ch) {
				ch = min(ch + c, 255);
				ch = min(ch, t->a);
			};
			for(int y = 0; y < 64; y++) {
				Do(t->r);
				Do(t->g);
				Do(t->b);
				t += 64;
			}
		}
		m = ib;
	#endif
		AlphaCut(m, left, y);
	}
	if(flags & LINE_RIGHT) {
	#ifdef PLASTIC
		ImageBuffer ib(m);
		for(int i = 0; i < 16; i++) {
			RGBA *t = ib[0] + 63 - i;
			int c = i + 5;
			auto Do = [&](byte& ch) {
				ch = c * ch / 20;
			};
			for(int y = 0; y < 64; y++) {
				Do(t->r);
				Do(t->g);
				Do(t->b);
				t += 64;
			}
		}
		m = ib;
	#endif
		AlphaCut(m, right, y);
	}
	if(flags & LINE_TOP) {
	#ifdef PLASTIC
		ImageBuffer ib(m);
		for(int i = 0; i < 16; i++) {
			RGBA *t = ib[i];
			int c = 5 * (16 - i);
			auto Do = [&](byte& ch) {
				ch = min(ch + c, 255);
				ch = min(ch, t->a);
			};
			for(int x = 0; x < 64; x++) {
				Do(t->r);
				Do(t->g);
				Do(t->b);
				t++;
			}
		}
		m = ib;
	#endif
		AlphaCut(m, top, x);
	}
	if(flags & LINE_BOTTOM) {
	#ifdef PLASTIC
		ImageBuffer ib(m);
		for(int i = 0; i < 16; i++) {
			RGBA *t = ib[63 - i];
			int c = i + 5;
			auto Do = [&](byte& ch) {
				ch = c * ch / 20;
			};
			for(int x = 0; x < 64; x++) {
				Do(t->r);
				Do(t->g);
				Do(t->b);
				t++;
			}
		}
		m = ib;
	#endif
		AlphaCut(m, flags & ROUGH ? rough : bottom, x);
	}

	if(flags & CORNER_TOPLEFT)
		AlphaCut(m, &tl, 0);
	if(flags & CORNER_TOPRIGHT)
		AlphaCut(m, &tr, 0);
	if(flags & CORNER_BOTTOMLEFT)
		AlphaCut(m, &bl, 0);
	if(flags & CORNER_BOTTOMRIGHT)
		AlphaCut(m, &br, 0);

	if(flags & TUNNEL)
		AlphaCut(m, BlocksImg::tunnel());

	if(flags & UPPERCUT)
		AlphaCut(m, BlocksImg::bottom());
	
	if(flags & PLATFORM) {
		Image mm = BlocksImg::platform();
		Over(mm, m);
		m = mm;
	}
	
	blocki.Add(key, block.GetCount());
	block.Add(m);
	
	return block.GetCount() - 1;
}

inline
int Get(int x, int y)
{
	return x >= 0 && x < MAPX && y >= 0 && y < MAPY ? jetstory[y][x] : 0;
}

inline
bool Empty(int x, int y)
{
	int c = Get(x, y);
	return c == 0 && (y < 0 || x >= 0 && x < MAPX);
}

void Beautify(const Image& raster, int ii, dword flags = 0)
{
	for(int x = 0; x < MAPX; x++)
		for(int y = 0; y < MAPY; y++) {
			if(jetstory[y][x] == ii) {
				dword f = flags;
				bool bottom = (y < 0 || x >= 0 && x < MAPX) && findarg(Get(x, y + 1), 0, 12) >= 0;
				if(bottom)
					f |= LINE_BOTTOM;
				bool top = (y < 0 || x >= 0 && x < MAPX) && findarg(Get(x, y - 1), 0, 17) >= 0;
				if(top)
					f |= LINE_TOP;
				if(Empty(x - 1, y))
					f |= LINE_LEFT;
				if(Empty(x + 1, y))
					f |= LINE_RIGHT;

				if(Empty(x - 1, y) && top && Empty(x - 1, y - 1))
					f |= CORNER_TOPLEFT;
				if(Empty(x + 1, y) && top && Empty(x + 1, y - 1))
					f |= CORNER_TOPRIGHT;
				if(Empty(x - 1, y) && bottom && Empty(x - 1, y + 1))
					f |= CORNER_BOTTOMLEFT;
				if(Empty(x + 1, y) && bottom && Empty(x + 1, y + 1))
					f |= CORNER_BOTTOMRIGHT;
			
				blocks[y][x] = GetBlock(raster, x, y, f);
			}
		}

	LOG("Block count: " << block.GetCount());
}

void BeautifyMap()
{
	for(int i = 0; i < 30; i++)
		block.Add(decode(i, 13, BlocksImg::A13(),
		                    14, BlocksImg::A14(),
		                    15, BlocksImg::A15(),
		                    16, BlocksImg::A16(),
		                    17, BlocksImg::A17(),
		                    18, BlocksImg::A18(),
		                    20, BlocksImg::A20(),
		                    22, BlocksImg::A22(),
		                    23, BlocksImg::A23(),
		                    24, BlocksImg::A24(),
		                    25, BlocksImg::A25(),
		                    26, BlocksImg::A26(),
		                    28, BlocksImg::A28(),
		                    Upscale2x(BlocksImg::Get(i))));

	BlocksImg::Set(BlocksImg::I_back, BlendTiles(BlocksImg::back(), Size(32, 32)));
	
	memcpy(blocks, jetstory, sizeof(jetstory));

	Beautify(BlocksImg::crystal(), 9);

	Beautify(BlocksImg::sandstone(), 6);
	
	Beautify(BlocksImg::stone(), 7);
	Beautify(BlocksImg::stone(), 8);
	
	Beautify(BlocksImg::dirt(), 3);
	Beautify(BlocksImg::dirt(), 4);
	Beautify(BlocksImg::dirt(), 5, TUNNEL);
	
	Beautify(BlocksImg::brick(), 12, UPPERCUT);
	Beautify(BlocksImg::brick(), 19, UPPERCUT|PLATFORM);
	
	Beautify(BlocksImg::crystal(), 9);
	
	Beautify(BlocksImg::Stones(), 1);
	Beautify(BlocksImg::Stones(), 2);

	Beautify(BlocksImg::granit(), 10);
	Beautify(BlocksImg::granit(), 11);

	Beautify(BlocksImg::raster(), 21);

	Beautify(BlocksImg::diamant(), 29);

	Beautify(GtImg::Rust(), 30);
	Beautify(GtImg::Rust(), 31, TUNNEL);

	Size gsz = BlocksImg::granit().GetSize();
	Size rmsz = 2 * (gsz / 64);
	
	for(int x = 0; x <= rmsz.cx; x++) {
		righttop.At(x) = GetBlock(BlocksImg::granit(), x, 0, LINE_TOP);
		for(int y = 0; y <= rmsz.cy; y++) {
			rightmap.At(y).At(x) = GetBlock(BlocksImg::granit(), x, y, 0);
		}
	}
	
	Size rsz = GtImg::Rust().GetSize();
	Size rtsz = 2 * (rsz / 64);
	for(int x = 0; x <= rtsz.cx; x++)
		for(int y = 0; y <= rtsz.cy; y++)
			rustmap.At(y).At(x) = GetBlock(GtImg::Rust(), x, y, 0);

	Size dsz = BlocksImg::dirt().GetSize();
	Size ltsz = 2 * (dsz / 64);
	
	for(int x = 0; x <= ltsz.cx; x++) {
		lefttop.At(x) = GetBlock(BlocksImg::dirt(), x, 0, LINE_TOP);
		for(int y = 0; y <= ltsz.cy; y++) {
			leftmap.At(y).At(x) = GetBlock(BlocksImg::dirt(), x, y, 0);
		}
	}

	LOG("Final block count: " << block.GetCount());

	return;

#if 0	
	h = BlendTiles(BlocksImg::art(), Size(32, 32));
	BlocksImg::Set(7, h);
	BlocksImg::Set(8, h);
	
	
	BlocksImg::Set(6, BlendTiles(BlocksImg::sandstone(), Size(32, 32)));
	
	h = BlendTiles(, Size(16, 16));
	
//	ship.pos.x = 8200;

	h = BlendTiles(BlocksImg::art2(), Size(32, 32));
	BlocksImg::Set(10, h);
	BlocksImg::Set(11, h);
	
	BlocksImg::Set(29, BlendTiles(BlocksImg::crystal2(), Size(32, 32)));
#endif
}
