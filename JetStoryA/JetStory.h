#ifndef _JetStory_JetStory_h_
#define _JetStory_JetStory_h_

#include <Core/Core.h>
#include <GLDraw/GLDraw.h>

#ifdef PLATFORM_POSIX
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

#include <Synth/Synth.h>

#include <VirtualGui/SDL2GL/SDL2GL.h>

#include <Painter/Painter.h>

using namespace Upp;

#define IMAGECLASS JetStoryImg
#define IMAGEFILE <JetStoryA/JetStory.iml>
#include <Draw/iml_header.h>

#define IMAGECLASS BlocksImg
#define IMAGEFILE <JetStoryA/Blocks.iml>
#include <Draw/iml_header.h>

#define IMAGECLASS GtImg
#define IMAGEFILE <JetStoryA/Gt.iml>
#include <Draw/iml_header.h>

#define IMAGECLASS OtherImg
#define IMAGEFILE <JetStoryA/Other.iml>
#include <Draw/iml_header.h>

#define BIGSHIP

const int ENEMY_LIMIT = 4000;

extern SDL_Window *sdl_window;
extern GLDraw gl_draw;
extern Size window_size;
extern SDL_GLContext glcontext;

extern bool design_mode;
extern bool cheating;
extern bool gameover;
extern int  gameover_time;
extern int  base_count0;
extern int  base_count;
extern int  base_count_max;
extern int  game_tick;
extern int  checkpoint_info;

extern int game_accomplished;

extern bool training;
extern int training_remains;
extern int training_total;
extern int training_counter;
extern Vector<Rect> training_block;

inline int TrainingIndex() { return training_total - training_remains; }

void SetMessage(const String& msg, bool fl = false);

void PaintMessage();
void TrainingPoint();

extern String checkpoint_save;

inline
int GameTick() { return game_tick; }

extern Vector<Image> lflame;
extern Vector<Image> sflame;
extern Vector<Image> bflame;

void MakeFlames();

void ResetGame();

#define MAX_CONTROLLERS 4

extern Vector<SDL_GameController *> GamePad;
extern Vector<SDL_Joystick *> Joystick;
extern Vector<bool> JoystickButton;

String ConfigFolder();

void   StoreSettings();
void   LoadSettings();

String StoreGame();
bool   LoadGame(const String& h);
Pointf LoadShipPosition(const String& h);

void   Checkpoint();

void BlockMap();

void BeautifyMap();

enum {
	KEY_LEFT,
	KEY_RIGHT,
	KEY_UP,
	KEY_BOMB,
	KEY_FIRE,
	KEY_ROCKET
};

extern int game_key[];

enum { MAPX = 256, MAPY = 512, BLOCKSIZE = 64 };

enum { MAPCX = MAPX * BLOCKSIZE };

void ReadGamepad(bool& left, bool& right, bool& up, bool& down, bool& bomb, bool& fire, bool& fire2);

extern int jetstory[MAPY][MAPX];
extern int blocks[MAPY][MAPX];

extern bool blockmap[64][4][4];
extern int training_left;

enum FLAGS {
	SWAPX = 1,
	SWAPY = 2,
	ROTATE = 4,
};

void DrawImage(int x, int y, const Image& img, dword flags = 0);
void DrawImage(int x, int y, float alpha, const Image& img, dword flags = 0);
void DrawImage(Point p, const Image& img, Color color);

void PaintMap(Size sz, Point topleft, Event<int, int, const Image&> DrawImage);

bool MapCollision(Point p);
bool MapCollision(Rectf r);
bool MapCollision(Pointf pos, Sizef sz);

struct Object {
	Pointf pos;
	Pointf speed = Pointf(0, 0);
	Pointf last_bounce;
	
	virtual Size  GetSize() const = 0;
	virtual Rectf GetRect() const                  { Sizef sz = GetSize(); return Rectf(pos - sz / 2, sz); }
	virtual void  Paint(Point sp) {}
	virtual void  Serialize(Stream& s)             { s % pos % speed % last_bounce; }

	double Move(Size sz, Pointf bounce, Pointf friction, double gravity = 0.005);
	double Move(Size sz, double bounce = 0, double friction = 0.997, double gravity = 0.005);
	double Move()                                  { return Move(GetSize()); }
	
	virtual ~Object() {}
};

enum {
	SHIP_SHIELD = 300,
};

struct Ship : Object {
	bool left;
	int  shield;
	int  fuel;
	int  ammo;
	int  rockets;
	int  bombs;
	
	void Reset() { speed = Pointf(0, 0); left = false; shield = SHIP_SHIELD; fuel = 20000; ammo = 500; rockets = 100; bombs = 100; left = true; }

	virtual Size GetSize() const  { return JetStoryImg::ship().GetSize(); }
	virtual void Paint(Point sp);
	virtual void Serialize(Stream& s);
};

struct Missile : Object {
	int    kind = 0;
	double accelx = 0;
	int    damage = 1;
	bool   left;
	Rectf  rect;
	
	virtual Size GetSize() const { return rect.GetSize(); }
	virtual void Serialize(Stream& s);
};

struct Debris : Object {
	Size   sz;
	int    start;
	int    end;
	double gravity = 0.005;
	Color  c1 = LtYellow();
	Color  c2 = Red();

	virtual Size GetSize() const { return sz; }
	virtual void Serialize(Stream& s);
};

struct ExplosionImage {
	Pointf pos;
	int    start;
	dword  image_flags;
	
	void Serialize(Stream& s) { s % pos % start % image_flags; }
};

void NotQuiet();

void Player();

void PaintScene(Size sz, Draw& w, Event<int, int, const Image&> di);
void PaintScene(Size sz, Draw& w);

// extern bool left, right, up, down, fire;

void Explosion(int priority, Pointf pos, int n, double spread = 0, int debris = 0);
void Explosion(Pointf pos, int n, double spread = 0, int debrisn = 0);

double PanPos(Pointf pos);
void PlaySound(int priority, double pan, const char *s, double volume = Null, double freq = Null);

enum { ENEMY_MISSILE = 999999 };

struct Enemy : Object {
	Pointf startpos;
	Image  image;
	byte   image_flags = 0;
	int16  shield = 1;
	int8   z = 0;
	int16  randomize;

// set at the start of cycle, not serialized
	double distance;

// permanent flags, not serialized:
	int    damage = 30;
	int    explosion_size = 5;
	int    explosion_debris = 0;
	bool   global = false;
	bool   checkpoint = false;

	bool CanSee(Pointf ref) const;
	bool CanSee() const;
	
	void Missile()                { explosion_debris = ENEMY_MISSILE; }
	bool IsMissile() const        { return explosion_debris == ENEMY_MISSILE; }

	virtual void Serialize(Stream& s);
	virtual void SerializeKind(Stream& s) = 0;
	virtual Size GetSize() const { return image.GetSize(); }
	virtual void Paint(Point sp) { DrawImage(sp.x, sp.y, image, image_flags); }
	virtual bool IsActive() const { return true; }

	virtual void Put(Point pos);
	virtual void Do();
	virtual void Die() {}
	
	void Beep(int priority, const char *s, double volume = Null, double freq = Null, double max_distance = 1000);
	void Beep(const char *s, double volume = Null, double freq = Null) { Beep(0, s, volume, freq); }
	
	int  AniTick()                  { return GameTick() + randomize; }
	bool AniTick(int n)             { return AniTick() % n == 0; }

	String GetKindId() const;

	Enemy() { randomize = Random() & 0x3fff; }
};

struct Box : Enemy {
	virtual void Do() {}
	virtual void SerializeKind(Stream& s) {}
	Box() { image = JetStoryImg::box(); shield = explosion_size = 15; z = 2; }
};

void RegisterEnemy(const char *id, const char *type_id, void (*factory)(One<Enemy>&), bool design = true);

#define REGISTER(id) INITBLOCK { RegisterEnemy(#id, typeid(id).name(), [](One<Enemy>& enemy) { enemy.Create<id>(); }); }
#define REGISTER_(id) INITBLOCK { RegisterEnemy(#id, typeid(id).name(), [](One<Enemy>& enemy) { enemy.Create<id>(); }, false); }

extern Ship ship;
extern Array<Enemy> enemy;
extern Array<Missile> missile;

extern Array<ExplosionImage> explosion;
extern Array<Debris> debris;
extern int   flash;
extern Color flash_color;

void Flash(int ms, Color c);

struct Epos {
	String id;
	Point  pos;
};

extern Array<Epos> emap;

ArrayMap<String, void (*)(One<Enemy>&)>& EnemyFactory();
Vector<bool>& DesignEnemy();

bool CreateEnemy(const String& id, One<Enemy>& h);

void ResetEnemy();
void LoadEnemy();
void SaveEnemy();

void SetMouseCursor(const Image& image);

void Design();
void DesignMap();

struct Wasp : Enemy {
	Pointf accel = Pointf(0, 0);
	int    dir = 0;
	bool   homing = false;

	virtual void Do();
	virtual void SerializeKind(Stream& s) { s % accel % dir % homing; }
	
	Wasp() { damage = 20; }
};


void DrawImage(Point vertex[4], const Image& img);

void DrawTriangle(Point p1, Point p2, Point p3, Color c1);
void DrawRect(const Rect& r, Color c1);
void DrawRect(const Rect& r, float alpha, Color c1);

enum {
	POWERUP_ENERGY = -100,
	POWERUP_FUEL,
	POWERUP_AMMO,
	POWERUP_ROCKETS,
	POWERUP_BOMBS,
	CHECKPOINT,
	TRAININGPOINT,
	
	TRAININGTARGET = 17865,
};

enum {
	MISSILE_GUN = 0,
	MISSILE_ROCKET = 1,
	MISSILE_BOMB = 2,
	MISSILE_DEBRIS = 3,
};

struct WaspMissile : Enemy {
	int tick0;

	virtual void SerializeKind(Stream& s) { s % tick0; }

	virtual void Do() {
		if(Move(image.GetSize(), 0, 0, 0.0001) || distance > 2000 || GameTick() - tick0 > 1200)
			shield = 0;
	}

	WaspMissile() { image = JetStoryImg::wasp_missile(); Missile(); damage = 10; }
};

void FireFireWork(Pointf pos, double speed = 1);

struct Spark : Enemy {
	int    sparks;
	Pointf xx[12];

	virtual void SerializeKind(Stream& s) { s % sparks; for(int i = 0; i < 12; i++) s % xx[i]; }
	virtual void Paint(Point sp);
	virtual void Do();
	
	void Gen();

	Spark();
};

struct FireWork : Enemy {
	int flash;

	virtual void SerializeKind(Stream& s) {}
	virtual void Do();
	virtual void Paint(Point sp);

	FireWork();
};

struct Projector : Enemy {
	bool lr = false;

	virtual void SerializeKind(Stream& s) { s % lr; }
	virtual void Do();
	Projector();
};

struct Launcher : Enemy {
	bool launch;
	int  tick0;
	int  phase;

	virtual void SerializeKind(Stream& s) { s % launch % tick0 % phase; }
	virtual void Paint(Point sp);
	virtual void Do();

	Launcher();
};

struct Base : Enemy {
	virtual void SerializeKind(Stream& s) {}
	virtual void Do();
	virtual void Die();
	Base();
};

Image BlendTiles(const Image& m, Size bsz, bool v2 = true, bool mirror = true);
Image Symmetry(const Image& m);

String IngameGui(bool esc);

void MissionAccomplished();

enum {
	TUNE_NONE,
	TUNE_QUIET,
	TUNE_BASE,
	TUNE_BOSS,
	TUNE_GAMEOVER,
	TUNE_TITLE,
	TUNE_COUNT,
};

void StartMusic(int m);
int  CurrentMusic();
void StopMusic();

Image TitleScreen();

struct CreditsCtrl : Ctrl {
	RichText text;
	TimeCallback tm;
	int y = 0;
	int limity = 0;
	Image top, bottom;

	virtual void Layout();
	virtual void Paint(Draw& w);
	CreditsCtrl();
};

#endif
