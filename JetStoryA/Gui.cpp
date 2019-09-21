#include "JetStory.h"

#include "bin.brc"

#define TOPICFILE <JetStoryA/app.tpp/all.i>
#include <Core/topic_group.h>

Image TitleScreen()
{
	static Image graphics = StreamRaster::LoadStringAny(String(screen, screen_length));
	return graphics;
}

int game_key[6] {
	SDL_SCANCODE_LEFT,
	SDL_SCANCODE_RIGHT,
	SDL_SCANCODE_UP,
	SDL_SCANCODE_B,
	SDL_SCANCODE_SPACE,
	SDL_SCANCODE_M,
/*
	SDL_SCANCODE_Y,
	SDL_SCANCODE_U,
	SDL_SCANCODE_O,
	SDL_SCANCODE_I,
	SDL_SCANCODE_P,
	SDL_SCANCODE_T,
*/
};

int volume = 10;

void SerializeSettings(Stream& s)
{
	s.Magic();
	int version = 1;
	s / version;
	for(int i = 0; i < __countof(game_key); i++)
		s % game_key[i];
	if(version >= 1)
		s % volume;
	s.Magic();
}

String ConfigFolder()
{
#ifdef PLATFORM_POSIX
	return GetAppDataFolder() + "/.JetStory";
#else
	return GetAppDataFolder() + "/JetStory";
#endif
}

String SettingsPath()
{
	return ConfigFolder() + "/settings.bin";
}

void StoreSettings()
{
	String p = SettingsPath();
	RealizePath(p);
	FileOut out(p);
	Store([](Stream& s) { SerializeSettings(s); }, out);
}

void SyncGlobalVolume()
{
	SetGlobalVolume(float(volume / 10.0));
}

void LoadSettings()
{
	FileIn in(SettingsPath());
	if(in) {
		Load([](Stream& s) { SerializeSettings(s); }, in);
		SyncGlobalVolume();
	}
}

struct ActiveText : Button {
	virtual void Paint(Draw& w);
	virtual bool HotKey(dword key);
	
	ActiveText() { WantFocus(); }
};

void ActiveText::Paint(Draw& w)
{
	Rect sr = GetScreenRect();
	if(GetVisualState() == CTRL_HOT || HasFocus())
		DrawRect(sr.OffsetedHorz(-10), 0.3f, LtBlue());
	w.DrawText(2, 2, label, Arial(35), GetVisualState() == CTRL_DISABLED ? LtGray() : Gray());
	w.DrawText(0, 0, label, Arial(35), GetVisualState() == CTRL_HOT || HasFocus() ? Yellow() :
	                                   GetVisualState() == CTRL_DISABLED ? Gray() : White());
}

bool ActiveText::HotKey(dword key)
{
	int c = *label;
	if(c >= '0' && c < '9' && c - '0' + K_0 == key) {
		PseudoPush();
		return true;
	}
	return false;
}

struct IngameGuiCtrl : TopWindow {
	Array<ActiveText> button;
	int               x;
	int               y;
	bool              esc = true;

	virtual void Paint(Draw& w) {
		Image graphics = TitleScreen();
		Size sz = GetSize();
		Size isz = graphics.GetSize();
		w.DrawRect(sz, Black());
		w.DrawImage((sz.cx - isz.cx) / 2, 0, graphics);
	}
	
	void UpDown(int d) {
		if(button.GetCount() == 0)
			return;
		int q = -1;
		for(int i = 0; i < button.GetCount(); i++)
			if(button[i].HasFocus()) {
				q = i;
				break;
			}
		if(q < 0)
			button[d < 0 ? button.GetCount() - 1 : 0].SetFocus();
		else
			button[(q + d + button.GetCount()) % button.GetCount()].SetFocus();
	}
	
	virtual bool Key(dword key, int count) {
		if(key == K_ESCAPE && esc)
			Break(-1);
		if(key == K_UP)
			UpDown(-1);
		if(key == K_DOWN)
			UpDown(1);
		return false;
	}
	
	ActiveText& Add(const char *text, Event<> action)
	{
		ActiveText& h = button.Add();
		Ctrl::Add(h.TopPos(y, 40).HSizePos(x, 0));
		h << action;
		h.SetLabel(text);
		y += 40;
		return h;
	}
	
	IngameGuiCtrl() {
		if(window_size.cx >= 1920 && window_size.cy >= 1000) {
			x = 1100;
			y = 600;
		}
		else {
			x = window_size.cx - 630;
			y = window_size.cy - 280;
		}
		FullScreen();
	}
};

struct RedefineKeys : IngameGuiCtrl {
	int n;
	int keyi;
	
	TimeCallback tm;
	
	virtual void Paint(Draw& w) {
		static const char *t[] = {
			"Left",
			"Right",
			"Up",
			"Bomb",
			"Gun",
			"Missile",
		};

		IngameGuiCtrl::Paint(w);

		int y = IngameGuiCtrl::y;
		for(int i = 0; i < 6; i++) {
			
			w.DrawText(x + 2, y + 2, t[i], Arial(35), Gray());
			w.DrawText(x, y, t[i], Arial(35), White());

			String k = SDL_GetScancodeName((SDL_Scancode)game_key[i]);
			if(keyi != i || (n & 1)) {
				w.DrawText(x + 222, y + 2, k, Arial(35), Gray());
				w.DrawText(x + 220, y, k, Arial(35), keyi == i ? Yellow() : White());
			}
			y += 40;
		}
	}
	
	virtual bool Key(dword key, int count) {
		if(key & K_KEYUP)
			return true;
		if(key < K_DELTA && key != K_SPACE && key != K_ENTER)
			return true;
		if(key == K_ESCAPE) {
			Break();
			return true;
		}
		const Uint8 *keystate = SDL_GetKeyboardState(NULL);
		for(int i = 0; i < SDL_NUM_SCANCODES; i++)
			if(keystate[i]) {
				const char *s = SDL_GetScancodeName((SDL_Scancode)i);
				if(s && *s) {
					game_key[keyi] = i;
					if(++keyi >= 6)
						Break();
					Refresh();
				}
			}
		return true;
	}
	
	RedefineKeys() {
		tm.Set(-300, [=] { Refresh(); n++; });
		keyi = 0;
	}
};

struct Sav {
	Time   time;
	int    game_time;
	String path;
};

struct GUI : SDL2GUI {
	virtual void        HandleSDLEvent(SDL_Event* event);
	
	bool pup = false;
	bool pdown = false;
	bool pfire = false;
};

void GUI::HandleSDLEvent(SDL_Event* event)
{
	bool left = false, right = false, up = false, down = false, bomb = false, fire = false, fire2 = false;
	ReadGamepad(left, right, up, down, bomb, fire, fire2);
	if(pup != up) {
		if(up)
			Ctrl::DoKeyFB(K_UP, 1);
		pup = up;
	}
	if(pdown != down) {
		if(down)
			Ctrl::DoKeyFB(K_DOWN, 1);
		pdown = down;
	}
	if(pfire != fire) {
		if(fire) {
			Ctrl::DoKeyFB(K_ENTER, 1);
			Ctrl::DoKeyFB(K_ENTER|K_KEYUP, 1);
		}
		pfire = fire;
	}
	
	SDL2GUI::HandleSDLEvent(event);
}

String IngameGui(bool esc)
{
	StopMusic();
	StartMusic(TUNE_TITLE);
	Array<Sav> sf;
	FindFile ff(AppendFileName(ConfigFolder(), "*.sav"));
	while(ff) {
		if(ff.IsFile()) {
			Vector<String> h = Split(ff.GetName(), '_');
			if(h.GetCount() == 3) {
				Sav& s = sf.Add();
				s.game_time = atoi(h.Top());
				s.time = ff.GetLastWriteTime();
				s.path = ff.GetPath();
			}
		}
		ff.Next();
	}
	
	Sort(sf, [](const Sav& a, const Sav& b) { return a.time > b.time; });
	
	GUI gui;
	gui.Attach(sdl_window, glcontext);
	String result;
	SColorLabel_Write(White());
	RunVirtualGui(gui, [&] {
		auto& sbs = ScrollBar::StyleDefault().Write();
		sbs.arrowsize = 9;
		sbs.barsize = 9;
		sbs.thumbmin = 32;
		sbs.bgcolor = Null;

		IngameGuiCtrl h;

		h.Add("1. Resume from the latest checkpoint", [&] { if(sf.GetCount()) result = sf[0].path; h.Break(); })
		 .Enable(sf.GetCount());
		h.Add("2. Start the new game", [&] { result = "N"; h.Break(); });
		h.Add("3. Resume from the selected checkpoint", [&] {
			ImageCtrl preview;
			Size preview_size = window_size / 2 - 40;
			IngameGuiCtrl sg;
			ArrayCtrl list;
			list.NoHeader().NoGrid().AddColumn();
			list.NoBackground();
			list.SetLineCy(Arial(25).GetCy());
			list.SetFrame(NullFrame());
			for(int i = 0; i < sf.GetCount(); i++) {
				const Sav& s = sf[i];
				int seconds = s.game_time * 4 / 1000;
				list.Add(AttrText(Format("%04d-%02d-%02d %02d:%02d:%02d, game time %02d:%02d:%02d",
				                  (int)s.time.year, (int)s.time.month, (int)s.time.day,
				                  (int)s.time.hour, (int)s.time.minute, (int)s.time.second,
				                  seconds / 3600, seconds % 3600 / 60, seconds % 3600 % 60))
				         .SetFont(Arial(25)).NormalInk(White()));
			}
			
			list.WhenLeftDouble = [&] {
				int i = list.GetCursor();
				if(i >= 0 && i < sf.GetCount()) {
					result = sf[i].path;
					sg.Break();
				}
			};
			list.WhenSel = [&] {
				int i = list.GetCursor();
				Image m;
				if(i >= 0 && i < sf.GetCount()) {
					Pointf pos = LoadShipPosition(ZDecompress(LoadFile(sf[i].path)));
					if(!IsNull(pos)) {
						Size psz = preview_size * 2;
						ImageDraw w(psz);
						w.DrawRect(psz, Black());
						PaintMap(psz, Point(pos - psz / 2), [&](int x, int y, const Image& m) { w.DrawImage(x, y, m); });
						Point p = preview_size - JetStoryImg::ship().GetSize() / 2;
						w.DrawImage(p.x, p.y, JetStoryImg::ship());
						m = Rescale((Image)w, preview_size);
					}
				}
				preview.SetImage(m);
			};
			int lx = window_size.cx / 2 + 140;
			int ly = window_size.cy / 2 + 100;
			sg.Ctrl::Add(list.LeftPos(lx, 512).TopPos(ly, window_size.cy - ly - 100));
			
			Button load;
			load.Ok();
			load.SetLabel("Load checkpoint").SetFont(Arial(20));
			load << list.WhenLeftDouble;
			sg.Ctrl::Add(load.LeftPos(lx + 512 - 160, 160).TopPos(window_size.cy - 100 + 10, 35));

			Button cancel;
			cancel.SetLabel("Cancel").SetFont(Arial(20));
			cancel << [&] { sg.Break(); };
			sg.Ctrl::Add(cancel.LeftPos(lx, 160).TopPos(window_size.cy - 100 + 10, 35));
			
			sg.Ctrl::Add(preview.LeftPos(20, preview_size.cx).TopPos(window_size.cy / 2 + 20, preview_size.cy));

			list.GoBegin();
			sg.ActiveFocus(list);
			sg.Execute();
			if(result.GetCount())
				h.Break();
		});
		h.Add("4. Redefine keys", [&] {
			RedefineKeys().Execute();
			StoreSettings();
		});
/*		h.Add("5. Info", [&] {
			IngameGuiCtrl sg;
			RichTextCtrl text;
			text.TextColor(White());
			sg.Ctrl::Add(text.RightPos(0, 800).BottomPos(0, 600));
			text <<= GetTopic("topic://JetStory/app/info_en-us").text;
			sg.Execute();
		});*/
		auto vol = [=]()->String { return "5. Sound volume " + AsString(volume) + " / 10"; };
		Button *b = &h.Add(vol(), [&] {
			volume = volume + 1;
			if(volume > 10)
				volume = 0;
			b->SetLabel(vol());
			SyncGlobalVolume();
			StoreSettings();
		});

		h.Add("6. Quit", [&] { result = "Q"; h.Break(); });

		h.Open();

		CreditsCtrl c;
		h.Ctrl::Add(c.LeftPos(100, 600).VSizePos(window_size.cy / 2 + 10, 10));
		c.Layout();

		if(h.Execute() == -1 && esc)
			result = "Q";
	});
	gui.Detach();
	SDL_ShowCursor(false);
	StopMusic();
	return result;
}
