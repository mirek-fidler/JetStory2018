#include "JetStory.h"

struct Star {
	Point   p;
	int     time0;
	int     size;
	Color   color;
};

void MissionAccomplished()
{
	Array<Star> star;
	
	Array<Image> star_image;
	for(int i = 0; i < 50; i++) {
		ImageBuffer ib(15, 15);
		{
			BufferPainter iw(ib);
			iw.Clear(RGBAZero());
			iw.Circle(10, 10, 7 * i / 50.0).Fill(White());
		}
		star_image.Add(ib);
	}

	StopMusic();
	StartMusic(TUNE_BOSS);

	Image title = TitleScreen();

	SDL_Event event;
	int time0 = msecs();
	Point center = window_size / 2;
	Point wcenter = 6 * window_size / 10;
	Pointf shippos = center - ship.GetSize() / 2;
	Pointf speed(0, 0);
	if(ship.pos.y < -10)
		ship.pos.y = -10;
	int ptm = time0;
	for(;;) {
		int tm = msecs() - time0;
		int interval = tm - ptm;
		ptm = tm;
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
			case SDL_KEYDOWN:
			case SDL_JOYBUTTONDOWN:
			#ifndef flagDEVEL
				if(tm > 8000)
			#endif
					return;
			}
		}
		
		if(sdl_window == SDL_GetKeyboardFocus()) {
			double angle0 = 0;
			if(tm > 6000)
				angle0 = max(0.0000015 * (tm - 6000) * (tm - 6000), 0.0);
			double out = clamp(1.0 - (tm - 14000) / 5000.0, 0.0, 1.0);
			DrawRect(window_size, Blend(Color(6, 255, 118), Black(), clamp(tm / 10, 0, 255)));

			int n = min((int)Random(10 * interval + 1), 100);
			for(int i = 0; i < n; i++)
				if(star.GetCount() < 10000) {
					Star& s = star.Add();
					s.p = Point(Random(200000) - 100000, Random(100000) - 50000);
					s.time0 = tm;
					s.size = Random(450) + 100;
					s.color = Color(128 + Random(128), 128 + Random(128), 128 + Random(128));
				}
		
			Vector<int> done;
			for(int i = 0; i < star.GetCount(); i++) {
				Star& s = star[i];
				double z = 0.1 * (1500 - (tm - s.time0));
				Pointf p = Pointf(s.p) / max(z, 0.00001) + Pointf(center);
				if(p.x < 0 || p.x >= window_size.cx || p.y < 0 || p.y >= window_size.cy)
					done.Add(i);
				else
					DrawImage(Point(p) - Point(7, 7),
					          star_image[(clamp(int(s.size / z), 0, star_image.GetCount() - 1))],
					          s.color);
			}
			star.Remove(done);


			PaintMap(window_size, (Point)ship.pos - center, [&](int x, int y, const Image& img) {
				Size isz = img.GetSize();
				Rect r(Point(x, y), isz);
				Point vertex[4] = { r.TopLeft(), r.TopRight(), r.BottomLeft(), r.BottomRight() };
				for(int i = 0; i < 4; i++) {
					Point& v = vertex[i];
					double distance = Distance(v, wcenter);
					double angle = Bearing(Point(v - wcenter));
					double shift = tm / 200.0 * sin(tm / 100.0 + distance / 100.0);
					v = wcenter + Point(Polar(angle + angle0) * (shift + distance) * out);
				}
				DrawImage(vertex, img);
			});

			extern bool left, right, up;
			extern int up_flame_ani;
			extern int back_flame_ani;
			up = false;
			left = false;
			right = false;
			ship.fuel = 100;
			ship.left = true;

			if(tm < 20000) {
				if(tm > 5500) {
					if(++back_flame_ani >= bflame.GetCount())
						back_flame_ani = bflame.GetCount() / 2;
					if(++up_flame_ani >= lflame.GetCount())
						up_flame_ani = lflame.GetCount() / 2;
					speed.x -= 0.1;
					speed.y -= 0.03;
					shippos += speed;
					up = true;
					left = true;
				}
		
				ship.Paint(Point(shippos));
			}

			if(tm > 15000) {
				DrawImage((window_size.cx - title.GetWidth()) / 2, 0, clamp((tm - 15000) / 5000.0, 0.0, 1.0), title);
				Font fnt = Serif(100).Bold().Italic();
				String text = "MISSION ACCOMPLISHED";
				Size tsz = GetTextSize(text, fnt);
				Point p = Rect(window_size).CenterPos(tsz);
				p.y += 100;
				for(int x = -1; x <= 1; x++)
					for(int y = -1; y <= 1; y++)
						if(x || y)
							gl_draw.DrawText(p.x + x, p.y + y, text, fnt, Black());
				gl_draw.DrawText(p.x, p.y, text, fnt, LtGreen());
			}
			SDL_GL_SwapWindow(sdl_window);
		}
		else {
			Sleep(20);
		}
	}
}
