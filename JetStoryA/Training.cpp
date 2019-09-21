#include "JetStory.h"

#define IMAGECLASS OtherImg
#define IMAGEFILE <JetStoryA/Other.iml>
#include <Draw/iml_source.h>

bool training;
int  training_counter;
int  training_remains;
int  training_total;

Vector<Rect> training_block;

const char *training_step[] {
	"[@G=/* Welcome to the basic combat training.&]Please use #LEFT key to go left to the checkpoint.",

	"[@G/* Well done.&]Now go to the next checkpoint using #UP, #RIGHT, #LEFT keys!",

	"[@G/* Good!&]Please navigate the labyrinth!",

	"[@G/* Perfect!&]Please navigate the labyrinth to the next checkpoint.&[/@R Be careful, there are now mines that kill you instantly!]",

	"[@G/* Nice work!&]Good, now comes the hard part.&[/@R Be careful, it is now quite difficult!]",

	"[@G/* Perfect!&]Please navigate the rest of the labyrinth.",
	
	"[@G/* Well done!&]All that maneuvring around the labyrinth have cost you a lot of fuel.&Please refuel your ship by picking the fuel power-up.&[/@R Running out of fuel results in certain death!]",

	"[@G/* Very well!&]You are now cleared for live fire practice.&Please pickup the gun ammo and destroy the practice targets using #GUN key!&"
	"[/@R WARNING: colliding with practice targets kills you instantly!",

	"[@G/* Good shooting!&]Please destroy the remaing two targets!",

	"[@G/* Good shooting!&]Please destroy the remaing three targets!",

	"[@G/* Good shooting!&]Please destroy the remaing target!",

	"[@G/* Good shooting!&]Please move to the missile training range.",
	"Some targets are hardened by shields and are only destroyed after many gun hits OR by "
	"high explosive missiles.&"
	"Please pickup missiles and destroy targets in this area using #MISSILE key.",

	"[@G/* Good shooting!&]Please destroy the remaing two targets!",

	"[@G/* Good shooting!&]Please destroy the remaing target!",

	"[@G/* Good shooting!&]Please move to the bomb training range.",
	"If missile trajectory is not suitable because the target is below you, use bombs.&"
	"Please pickup bombs and destroy targets in this area using #BOMB key.",

	"[@G/* Good bombing!&]Please destroy the remaing target!",

	"[@G/* Congratulations!&]You have now finished the basic combat training. Gear up and go give them hell!",
};

struct TPoint : Enemy {
	int index = 0;
	
	virtual bool IsActive() const { return index == TrainingIndex(); }

	virtual void SerializeKind(Stream& s) { s % index; }
	virtual Size GetSize() {
		return IsActive() ? image.GetSize() : Size(0, 0);
	}
	virtual void Paint(Point sp) {
		if(IsActive() || design_mode) {
			Enemy::Paint(sp);
			int n = min((AniTick() >> 4) % 12, 10);
			Image m = OtherImg::TArrow();
			Size isz = m.GetSize();
			int x1 = sp.x + n;
			int y1 = sp.y + n;
			int x2 = sp.x + 64 - n - isz.cx;
			int y2 = sp.y + 64 - n - isz.cy;
			DrawImage(x1, y1, m);
			DrawImage(x2, y1, m, SWAPX);
			DrawImage(x1, y2, m, SWAPY);
			DrawImage(x2, y2, m, SWAPX|SWAPY);
		}
		else
			DrawImage(sp.x, sp.y, OtherImg::TPointBlocking());
	}

	virtual void Do() { training_counter++; if(!IsActive()) training_block.Add(GetRect().Inflated(2)); }
	TPoint() { index = training_counter++; global = true; image = OtherImg::TPoint(); damage = TRAININGPOINT; z = 1; }
};

REGISTER(TPoint);

struct TTarget : Enemy {
	virtual void SerializeKind(Stream& s) {}
	virtual void Do() {
		training_counter++;
		image = (AniTick() >> 6) & 1 ? OtherImg::Target1() : OtherImg::Target2();
	}
	TTarget() { shield = 1; explosion_size = 20; global = true; damage = TRAININGTARGET; training_counter++; }
};

REGISTER(TTarget);

struct HTarget : Enemy {
	virtual void SerializeKind(Stream& s) {}
	virtual void Do() {
		training_counter++;
		image = (AniTick() >> 6) & 1 ? OtherImg::HardTarget1() : OtherImg::HardTarget2();
	}
	HTarget() {
		damage = TRAININGTARGET;
		training_counter++;
	    shield = 50;
		explosion_size = 150;
		global = true;
	}
};

REGISTER(HTarget);

struct TMine : Enemy {
	virtual void Do() {}
	virtual void SerializeKind(Stream& s) {}
	TMine() { image = OtherImg::Mine(); global = true; shield = explosion_size = 150000; damage = 1000; z = 2; }
};

REGISTER(TMine);

Image message_image;

void SetMessage(const String& msg, bool fl)
{
	static String message;
	if(msg != message) {
		message = msg;
		if(IsNull(msg))
			message_image = Null;
		else {
			RichText txt = ParseQTF(String("[@W= ") + message);
			txt.ApplyZoom(Zoom(3, 10));
			int cx = min(txt.GetWidth(), window_size.cx - 16);
			ImageBuffer ib(cx + 16, txt.GetHeight(cx) + 16);
			{
				BufferPainter w(ib);
				w.Clear(RGBAZero());
				txt.Paint(w, 8, 8, cx);
			}
			message_image = ib;
			flash = fl * 255;
		}
	}
}

void PaintMessage()
{
	if(gameover)
		return;
	static int ri = -1;
	if(ship.pos.y < 0) {
		bool b = ship.pos.x < -2000 || ship.pos.x > MAPX * 64 + 2000;
		static bool bb;
		if(bb != b) {
			bb = b;
			if(b)
				SetMessage("Our sensors detect no bases in this direction.&Please go back before you run out of fuel.", false);
			else
				SetMessage(Null);
		}
		ri = -1;
	}
	else
	if(training && !design_mode) {
		if(TrainingIndex() < __countof(training_step)) {
			if(ri != TrainingIndex()) {
				ri = TrainingIndex();
				VectorMap<String, String> repl;
				static const char *k[] = { "#LEFT", "#RIGHT", "#UP", "#BOMB", "#GUN", "#MISSILE" };
				for(int i = 0; i < 6; i++)
					repl.Add(k[i], String() << "[@W$k  "
					               << SDL_GetScancodeName((SDL_Scancode)game_key[i])
					               << " ]");
				SetMessage(Replace(training_step[ri], repl), true);
			}
		}
		else
			SetMessage(Null);
	}
	if(!IsNull(message_image)) {
		Size isz = message_image.GetSize();
		DrawRect(Rect(Point((window_size.cx - isz.cx) / 2, window_size.cy - isz.cy),
		         message_image.GetSize()), 0.7 + 0.3 * flash / 255.0,
		         Blend(Blue(), Yellow(), flash));
		DrawImage((window_size.cx - isz.cx) / 2, window_size.cy - isz.cy, message_image);
		if(flash > 0)
			flash =  max(flash - 5, 0);
	}
}

void TrainingPoint()
{
	if(gameover)
		return;
//	StartMusic(TUNE_BASE);
	PlaySound(5, 0.5, "261.62:L20V100R35T::::");
	Checkpoint();
}
