#include "JetStory.h"

bool hide_map;

void PaintScene(Size sz, Draw& w, Event<int, int, const Image&> di)
{
	GLDraw *gl = dynamic_cast<GLDraw *>(&w);

	int tick = GameTick();

	bool doflash = flash - tick > 0 && flash - tick < 25000;

	w.DrawRect(sz, doflash ? flash_color : Black());
	
	Size isz = JetStoryImg::ship().GetSize();

	Point topleft = (Point)ship.pos - sz / 2;

	Point sp = sz / 2 - isz / 2;

	gl->Finish();

	if(!doflash) {
		Image m = BlocksImg::back();
		Size isz = m.GetSize();
		for(int x = -4 * isz.cx; x < MAPX * BLOCKSIZE; x += isz.cx)
			for(int y = 0; y < MAPY * BLOCKSIZE; y += isz.cy) {
				Point p = Point(x, y) - topleft;
				if(p.x + sz.cx >= 0 && p.y + sz.cy >= 0 && p.x < sz.cx && p.y < sz.cy)
					DrawImage(p.x, p.y, m);
			}
	}
	DrawImage(7744 - topleft.x, 2304 - topleft.y, GtImg::Fish());

	int ani3 = (0x7fffffff & tick) / 5 % 3;
	for(auto& m : missile) {
		Image img = decode(m.kind, MISSILE_ROCKET, JetStoryImg::Get((m.accelx < 0 ? JetStoryImg::I_lhmissile0 : JetStoryImg::I_hmissile0) + ani3),
		                           MISSILE_BOMB, JetStoryImg::bomb(),
		                           MISSILE_DEBRIS, JetStoryImg::Get(JetStoryImg::I_debris0 + Random(4)),
		                           m.speed.x < 0 ? JetStoryImg::erif() : JetStoryImg::fire());
		                           
		Size msz = img.GetSize();
		Point p = (Point)m.pos - topleft - msz / 2;
		DrawImage(p.x, p.y, img);
	}

	static Vector<int> stars;
	static Vector<Color> star_color;
	auto StarColor = []() -> Color {
		const int l = 138;
		const int r = 117;
		return Color(Random(r) + l, Random(r) + l, Random(r) + l);
	};
	ONCELOCK {
		SeedRandom(0);
		for(int i = 0; i < MAPX * BLOCKSIZE; i++) {
			stars.Add(Random(MAPX * BLOCKSIZE));
			star_color.Add(StarColor());
		}
		SeedRandom();
	};
	for(int i = topleft.y; i < 0; i++) {
		if(i - topleft.y > sz.cy)
			break;
		int ii = (0 - i) % stars.GetCount();
		int x = (stars[ii] - topleft.x + 0x70000000) % (MAPX * BLOCKSIZE);
		if(x >= 0 && x < sz.cx)
			w.DrawRect(x, i - topleft.y, 1, 1, star_color[ii]);
		if(Random(10) == 0)
			star_color[ii] = StarColor();
	}

	gl->Finish();

	if(!gameover)
		ship.Paint(sp);

	for(auto& m : debris) {
		Point p = (Point)m.pos - topleft;
		w.DrawRect(p.x, p.y, m.sz.cx, m.sz.cy, Blend(m.c1, m.c2, (tick - m.start) * 255 / (m.end - m.start)));
	}
	gl->Finish();

	for(int z = 0; z < 3; z++)
		for(auto& m : enemy) {
			Size msz = m.GetSize();
			Point p = (Point)m.pos - topleft - msz / 2;
			if(Rect(p, msz).Intersects(sz) && m.z == z)
				m.Paint(p);
		}

	for(int i = explosion.GetCount() - 1; i >= 0; i--) {
		ExplosionImage& m = explosion[i];
		Size msz = JetStoryImg::boom1().GetSize();
		int ani = (tick - m.start) / 8;
		if(ani > 4)
			explosion.Remove(i);
		else
		if(ani >= 0) {
			Point p = (Point)m.pos - topleft - msz / 2;
			DrawImage(p.x, p.y, JetStoryImg::Get(JetStoryImg::I_boom1 + ani), m.image_flags);
		}
	}

	if(!hide_map)
		PaintMap(sz, topleft, di);
	
	if(gameover && tick - gameover_time > 100) {
		Font fnt = Arial(100).Bold().Italic();
		String text = "GAME OVER";
		Size tsz = GetTextSize(text, fnt);
		Point p = Rect(window_size).CenterPos(tsz);
		w.DrawText(p.x, p.y, text, fnt, Blend(White(), Red(), (GameTick() * 4) & 255));
		
		{
			extern int base_count_max;
			Font fnt = Arial(28).Bold().Italic();
			String text = String() << "You have eliminated " << base_count_max - base_count
			              << " / " << base_count_max << " bases";
			Size tsz = GetTextSize(text, fnt);
			Point p = Rect(window_size).CenterPos(tsz);
			for(int x = -1; x <= 1; x++)
				for(int y = -1; y <= 1; y++)
					w.DrawText(p.x + x, p.y + 80 + y, text, fnt, Black());
			w.DrawText(p.x, p.y + 80, text, fnt, White());
		}
		
		if(ship.fuel <= 0) {
			Font fnt = Arial(50).Bold().Italic();
			String text = "You have run out of fuel";
			Size tsz = GetTextSize(text, fnt);
			Point p = Rect(window_size).CenterPos(tsz);
			for(int x = -1; x <= 1; x++)
				for(int y = -1; y <= 1; y++)
					w.DrawText(p.x + x, p.y - 150 + y, text, fnt, Black());
			w.DrawText(p.x, p.y - 150, text, fnt, LtRed());
		}

		if(tick - gameover_time > 350) {
			Font fnt = Arial(50).Bold().Italic();
			String text = "Press any key to restart from the latest checkpoint";
			Size tsz = GetTextSize(text, fnt);
			Point p = Rect(window_size).CenterPos(tsz);
			for(int x = -1; x <= 1; x++)
				for(int y = -1; y <= 1; y++)
					w.DrawText(p.x + x, p.y + 150 + y, text, fnt, Black());
			w.DrawText(p.x, p.y + 150, text, fnt, LtGreen());
		}
	}

	const int pane_height = 32;
	
	if(checkpoint_info && tick - checkpoint_info < 1000) {
		Font fnt = Arial(50).Bold().Italic();
		String text = "CHECKPOINT REACHED";
		Size tsz = GetTextSize(text, fnt);
		Point p = Rect(window_size).CenterPos(tsz);
		w.DrawText(p.x, 2 * pane_height, text, fnt, LtCyan());
	}

	const int margin = 12;
	const int vmargin = 6;
	const int vheight = pane_height - 2 * vmargin;
	Font  valfont = Arial(vheight - 3).Italic().Bold();
	const int ty = (pane_height - valfont.GetHeight()) / 2;
	ChPaint(w, 0, 0, sz.cx, pane_height, JetStoryImg::TOP());
	
	int cx = sz.cx / 4;
	int xx = cx - margin;
	
	auto paint_val = [&](int x0, int val, int total, Color c, int w1, int w2, const char *text, bool ar = true) {
		x0 += margin;
		w.DrawRect(x0, vmargin, xx, vheight,
		           (ar || !training) && val < w1 && ((GameTick() / (val < w2 ? 60 : 120)) & 1) ? LtRed() : White());
		w.DrawRect(x0, vmargin, val * xx / total, vheight, c);
		for(int dx = -1; dx <= 1; dx++)
			for(int dy = -1; dy <= 1; dy++)
				if(dx || dy)
					w.DrawText(x0 + margin + dx, ty + dy, text, valfont, Black());
		w.DrawText(x0 + margin, ty, text, valfont, Color(192, 255, 197));
		ChPaint(w, x0, vmargin, xx, vheight, JetStoryImg::VALFRAME());
	};

	paint_val(0, ship.shield, SHIP_SHIELD, Color(56, 85, 150), 60, 30, "Shield");
	paint_val(cx, ship.fuel, 20000, Color(141, 42, 150), 6000, 3000, "Fuel");
	paint_val(2 * cx, ship.ammo, 500, Color(250, 151, 26), 100, 50, "Ammo", false);
	
	Font  nfont = Arial(vheight - 2).Bold();
	const int nty = (pane_height - nfont.GetCy()) / 2;
	int x0 = 3 * cx + vmargin + 2 * nty;
	auto paint_n = [&](const Image& m, int n, Color color = LtRed()) {
		Size isz = m.GetSize();
		w.DrawImage(x0, (pane_height - isz.cy) / 2, m);
		w.DrawText(x0 + isz.cx + vmargin, nty, AsString(n), nfont,
		           !training && n < 15 && ((GameTick() / 100) & 1) ? color : Black());
		x0 += 10 * ty;
	};
	
	paint_n(JetStoryImg::ROCKETS(), ship.rockets);
	paint_n(JetStoryImg::BOMBS(), ship.bombs);
	paint_n(JetStoryImg::BASES(), base_count, Green());
	
	Time tm = GetSysTime();
	nfont = Arial(vheight - 2);
	String text = Format("%2d:%02d%s", (int)tm.hour % 12, (int)tm.minute, tm.hour >= 12 ? "pm" : "am");
	int xt = sz.cx - GetTextSize(text, nfont).cx;
	w.DrawText(xt, nty, text, Arial(vheight - 2), Blue());
	
	int seconds = GameTick() * 4 / 1000;
	text = Format("%02d:%02d:%02d", seconds / 3600, seconds % 3600 / 60, seconds % 3600 % 60);
	int xt2 = xt - GetTextSize(text, nfont).cx - 2 * vmargin;
	if(xt2 >= x0)
		w.DrawText(xt2, nty, text, Arial(vheight - 2), Black());
	
/*	
	w.DrawRect(margin, vmargin, xx, vheight, ship.shield < 60 && ((GameTick() / (ship.shield < 30 ? 250 : 500)) & 1) ? LtRed() : White());
	w.DrawRect(margin, vmargin, ship.shield * xx / 200, vheight, Color(56, 85, 150));

	w.DrawRect(cx + margin, vmargin, xx, vheight, ship.fuel < 6000 && ((GameTick() / (ship.fuel < 3000 ? 250 : 500)) & 1) ? LtRed() : White());
	w.DrawRect(cx + margin, vmargin, ship.fuel * xx / 20000, vheight, );

	w.DrawRect(2 * cx + margin, vmargin, xx, vheight, White());
	w.DrawRect(2 * cx + margin, vmargin, ship.ammo * xx / 500, vheight, Color(141, 42, 0));
*/	
//	w.DrawText(3 * cx + margin, 6, Format("Shield: %3d   Fuel: %3d   Ammo:%5d   Rockets:%4d   Bombs:%4d",
//	                         ship.shield, ship.fuel, ship.ammo, ship.rockets, ship.bombs));
	                         


#ifdef flagDEVEL
	static BiVector<int> fpsq;

	fpsq.AddTail(msecs());
	while(fpsq.GetCount() > 20)
		fpsq.DropHead();
	
	double fps = 0;
	if(fpsq.GetCount())
		fps = 1000.0 / ((fpsq.Tail() - fpsq.Head()) / fpsq.GetCount());
	
	String jb;
	
	for(int i = 0; i < JoystickButton.GetCount(); i++)
		if(JoystickButton[i])
			jb << i << " ";

	w.DrawRect(0, window_size.cy - 24, sz.cx, 48, Gray());
	w.DrawText(0, window_size.cy - 24,
	           Format("X:%5.2f Y:%5.2f dx:%5.2f dy:%5.2f enemy:%5d missiles:%5d debris:%5d FPS:%3.2f %s TI:%d/%d Joystick: %s",
	           ship.pos.x, ship.pos.y, ship.speed.x, ship.speed.y,
	           enemy.GetCount(), missile.GetCount(), debris.GetCount(), fps,
	           cheating ? "CHEATING MODE" : "",
	           TrainingIndex(), training_total, jb),
	           Monospace(20), White());
#endif

	PaintMessage();
}

void PaintScene(Size sz, Draw& w)
{
	PaintScene(sz, w, [](int x, int y, const Image& m) { DrawImage(x, y, m); });
}

