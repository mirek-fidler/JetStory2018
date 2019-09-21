#include "JetStory.h"

#define IMAGECLASS JetStoryImg
#define IMAGEFILE <JetStoryA/JetStory.iml>
#include <Draw/iml_source.h>

SDL_Window *sdl_window;
GLDraw      gl_draw;
Size        window_size;
SDL_GLContext glcontext;

int64 msecs64()
{
	INTERLOCKED {
		static dword tm = SDL_GetTicks();
		static dword h = 0;
		dword t = SDL_GetTicks();
		if(t < tm)
			h++;
		tm = t;
		return (int64) MAKEQWORD(tm, h);
	}
	return 0;
}

extern int max_texture_memory;
extern int max_texture_count;

#ifdef PLATFORM_WIN32

Size GetDesktopResolution()
{
	int index = 0;
	DISPLAY_DEVICE dd;
	dd.cb = sizeof(DISPLAY_DEVICE);
	while (EnumDisplayDevices(NULL, index++, &dd, 0)) {
		if ((dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) &&
		    (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)) {
			DEVMODE mode;
			mode.dmSize = sizeof(mode);
			EnumDisplaySettings(dd.DeviceName, ENUM_CURRENT_SETTINGS, &mode);
			return Size(mode.dmPelsWidth, mode.dmPelsHeight);
		}
	}
	return Null;
}

void SetDPIAware()
{
	HRESULT (STDAPICALLTYPE *SetProcessDpiAwareness)(int);
	DllFn(SetProcessDpiAwareness, "Shcore.dll", "SetProcessDpiAwareness");
	if(SetProcessDpiAwareness)
		SetProcessDpiAwareness(1);
	else {
		BOOL (STDAPICALLTYPE * SetProcessDPIAware)(void);
		DllFn(SetProcessDPIAware, "User32.dll", "SetProcessDPIAware");
		if(SetProcessDPIAware)
			(*SetProcessDPIAware)();
	}
}

#endif

#define IMAGECLASS JsCtrlsImg
#define IMAGEFILE <JetStoryA/JsCtrls.iml>
#include <Draw/iml_header.h>

#define IMAGECLASS JsCtrlsImg
#define IMAGEFILE <JetStoryA/JsCtrls.iml>
#include <Draw/iml_source.h>

CONSOLE_APP_MAIN
{
	Override(CtrlsImg::Iml(), JsCtrlsImg::Iml());

#ifdef PLATFORM_WIN32
	if(GetDesktopResolution().cx < 2000)
		SetDPIAware();
#endif

	SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_AUDIO|SDL_INIT_JOYSTICK|SDL_INIT_HAPTIC);
	
	String p = ConfigFolder();
	RealizeDirectory(p);

#ifndef _DEBUG
	SetVppLogName(p);
#endif
	int max_texture_memory = 1024*1024*256;
	int max_texture_count = 32768*16;

	InitSoundSynth(false);

	ChReset();

#ifdef flagSS
//	Rect rect(0, 0, 1280, 720);
	Rect rect(0, 0, 1366, 768);
#else
	Rect rect(0, 0, 1920, 1080 - 64);
#endif
	sdl_window = SDL_CreateWindow("Jet Story", rect.left, rect.top, rect.GetWidth(), rect.GetHeight(),
	                                          SDL_WINDOW_SHOWN|SDL_WINDOW_OPENGL|SDL_WINDOW_ALLOW_HIGHDPI
#ifndef flagDEVEL
#ifndef flagSS
	                                         |SDL_WINDOW_BORDERLESS|SDL_WINDOW_FULLSCREEN_DESKTOP
#endif
#endif
	);

	SDL_GetWindowSize(sdl_window, &window_size.cx, &window_size.cy);
	
	int n = SDL_NumJoysticks();
	for(int i = 0; i < n; ++i) {
	    if(SDL_IsGameController(i)) {
			SDL_GameController *g = SDL_GameControllerOpen(i);
			if(g)
				GamePad.Add(g);
	    }
	    else {
			SDL_Joystick *j = SDL_JoystickOpen(i);
			if(j)
				Joystick.Add(j);
	    }
	}

	Sleep(200);
	
//	DDUMP(window_size);
	
	MemoryIgnoreLeaksBegin();
	glcontext = SDL_GL_CreateContext(sdl_window);
	MemoryIgnoreLeaksEnd();

	BlockMap();
	
	BeautifyMap();
	
	MakeFlames();

	gl_draw.Init(window_size, (uint64)glcontext);

	SDL_ShowCursor(false);

	Sound snd;
	snd.Load("130.81:L1e+100V0:L1e+100V115B:L1e+100V70N:L1e+100V90f4:L1e+100V60f3");
	SetChannel(0, snd);

	
	LoadEnemy();

	SDL_Event event;

//	checkpoint_save = LoadFile(ConfigFile("jetstory.sav"));
	
	LoadSettings();

#ifdef flagWIN
	MissionAccomplished();
	return;
#endif


again0:
	String c;
#ifndef flagDEVEL
	c = IngameGui(true);
	if(c == "Q") {
		SDL_GL_DeleteContext(glcontext);
		SDL_DestroyWindow(sdl_window);
		SDL_Quit();
		return;
	}
#endif

load_checkpoint:
	if(c.GetCount() > 1)
		checkpoint_save = ZDecompress(LoadFile(c));

again:
	game_accomplished = 0;
	base_count0 = 0;
	int64 time0 = msecs64();
	ResetEnemy();
	ResetGame();
	SetMessage(Null);
	ship.Reset();
	
	StopMusic();

#if defined(flagDEVEL) && !defined(flagTRAINING)
	ship.pos.x = 108;
	ship.pos.y = 121;
	training = false;
#else
	ship.pos.x = -2569;
#ifdef BIGSHIP
	ship.pos.y = 1784 - 8;
#else
	ship.pos.y = 1784;
#endif
	ship.ammo = 0;
	ship.bombs = 0;
	ship.rockets = 0;
#endif
	if(checkpoint_save.GetCount())
		LoadGame(checkpoint_save);
	
	SDL_GL_SetSwapInterval(1);
	
	bool fast = false;
	
	for(;;) {
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
			case SDL_QUIT:
				SDL_GL_DeleteContext(glcontext);
				SDL_DestroyWindow(sdl_window);
				SDL_Quit();
				return;
			case SDL_KEYDOWN:
			#ifdef flagDEVEL
				if(event.key.keysym.sym == SDLK_F9) {
					base_count0 = -base_count;
					break;
				}
				if(event.key.keysym.sym == SDLK_F5) {
					Checkpoint();
					break;
				}

				if(event.key.keysym.sym == SDLK_F1) {
					design_mode = true;
					Design();
					design_mode = false;
					SaveEnemy();
					ship.Reset();
					gameover = false;
					cheating = true;
					StopMusic();
					break;
				}
				if(event.key.keysym.sym == SDLK_F4) {
					StopMusic();
					DesignMap();
					StopMusic();
					BlockMap();
					BeautifyMap();
					break;
				}
				if(event.key.keysym.sym == SDLK_F3) {
					extern bool hide_map;
					hide_map = !hide_map;
					break;
				}
				if(event.key.keysym.sym == SDLK_F2) {
					ship.Reset();
					gameover = false;
					cheating = !cheating;
					break;
				}
				if(event.key.keysym.sym == SDLK_F8) {
					fast = !fast;
					break;
				}
		#if 1
				if(event.key.keysym.sym == SDLK_ESCAPE) {
					SDL_GL_DeleteContext(glcontext);
					SDL_DestroyWindow(sdl_window);
					SDL_Quit();
					return;
				}
		#endif
			#endif
				if(findarg(event.key.keysym.sym, SDLK_ESCAPE, SDLK_F10) >= 0) {
					c = IngameGui(false);
					if(c == "Q") {
						SDL_GL_DeleteContext(glcontext);
						SDL_DestroyWindow(sdl_window);
						SDL_Quit();
						return;
					}
					if(c == "N") {
						checkpoint_save.Clear();
						goto again;
					}
					if(c.GetCount())
						goto load_checkpoint;
					time0 = msecs64();
					break;
				}
			case SDL_JOYBUTTONDOWN:
				if(gameover && GameTick() - gameover_time > 350)
					goto again;
			}
		}
		
		if(sdl_window == SDL_GetKeyboardFocus()) {
			int64 time = msecs64();
			
			int frames = clamp(int(time - time0) / 5, 0, 16);
			if(fast)
				frames *= 32;
			int f = frames;
			while(frames-- > 0) {
				Player();
				time0 = time;
			}
	
			PaintScene(window_size, gl_draw);
	
			SDL_GL_SwapWindow(sdl_window);

			if(base_count_max && base_count <= 0 && !gameover && f && ++game_accomplished > 270) {
				SetChannel(0, snd);
				MissionAccomplished();
				checkpoint_save.Clear();
				goto again0;
			}
		}
		else {
			Sleep(20);
			time0 = msecs64();
		}
	}

	SDL_DestroyWindow(sdl_window);

	CloseSoundSynth(false);

	for(auto g : GamePad)
		SDL_GameControllerClose(g);

	for(auto j : Joystick)
		SDL_JoystickClose(j);

//	SDL_RemoveTimer(waketimer_id);
	SDL_Quit();
}
