#include "JetStory.h"

bool design_mode;
bool cheating;

Vector<SDL_GameController *> GamePad;
Vector<SDL_Joystick *> Joystick;

Vector<bool> JoystickButton;

int  base_count0;
int  base_count;
int  base_count_max;

int  game_accomplished;

int    checkpoint_info;
String checkpoint_save;

// ==== game status

bool left, right, up, down, fire, fire2;

int game_tick;

bool gameover;
int  gameover_time;

int           flash;
Color         flash_color;

int  lastfire;
bool prevdown;
bool prevfire2;

Ship ship;

Array<Missile> missile;
Array<ExplosionImage> explosion;
Array<Debris> debris;

Array<Enemy> enemy;

int boss_checkpoint = -1;

// ==== end of game status

int up_flame_ani = 0;
int back_flame_ani = 0;

Pointf LoadShipPosition(Stream& s)
{
	bool bool_dummy;
	int  int_dummy;
	Color color_dummy;
	Ship aship;
	s.Magic();
	int version = 0;
	s % version;
	s % bool_dummy % bool_dummy % bool_dummy % bool_dummy % bool_dummy % bool_dummy
	  % int_dummy
	  % bool_dummy
	  % int_dummy
	  % int_dummy
	  % color_dummy
	  % int_dummy
	  % bool_dummy % bool_dummy
	  % aship
	;
	return aship.pos;
}

Pointf LoadShipPosition(const String& h)
{
	StringStream ss(ZDecompress(h));
	ss.LoadThrowing();
	try {
		return LoadShipPosition(ss);
	}
	catch(LoadingError) {}
	return Null;
}

void SerializeGame(Stream& s)
{
	s.Magic();
	int version = 0;
	s % version;
	s % left % right % up % down % fire % fire2
	  % game_tick
	  % gameover
	  % gameover_time
	  % flash
	  % flash_color
	  % lastfire
	  % prevdown % prevfire2
	  % ship
	  % missile
	  % explosion
	  % debris
	  % training;
	int n = enemy.GetCount();
	s % n;
	if(s.IsStoring()) {
		for(Enemy& e : enemy) {
			String id = e.GetKindId();
			s % id;
			e.Serialize(s);
		}
	}
	else {
		enemy.Clear();
		for(int i = 0; i < n; i++) {
			String id;
			s % id;
			One<Enemy> e;
			if(!CreateEnemy(id, e))
				s.LoadError();
			e->Serialize(s);
			enemy.Add(e.Detach());
		}
	}
	s.Magic();
}

String StoreGame()
{
	StringStream ss;
	SerializeGame(ss);
	return ZCompress(ss);
}

bool LoadGame(const String& h)
{
	StringStream ss(ZDecompress(h));
	ss.LoadThrowing();
	try {
		SerializeGame(ss);
		return true;
	}
	catch(LoadingError) {}
	ResetEnemy();
	ResetGame();
	return false;
}

void ResetGame()
{
	game_tick = 0;
	checkpoint_info = 0;
	gameover = false;
	base_count = 0;
	missile.Clear();
	debris.Clear();
	explosion.Clear();
	flash = 0;
	ship.pos.x = 9000;
	ship.pos.y = -9;
	ship.left = true;
	prevdown = false;
	prevfire2 = false;
	lastfire = 0;
	boss_checkpoint = -1;
	training = true;
	up_flame_ani = 0;
	back_flame_ani = 0;
}

void Scatter(Pointf mpos)
{
	Vector<int> done;
	for(int i = 0; i < enemy.GetCount(); i++) {
		Enemy& e = enemy[i];
		if(!e.IsMissile() && e.damage >= 0) {
			e.shield -= (int)max(50 - Distance(mpos, e.pos), 0.0);
		}
	}
}

void Missile::Serialize(Stream& s)
{
	Object::Serialize(s);
	s % kind % accelx % damage % left % rect;
}

int    quiet;

void NotQuiet()
{
	if(CurrentMusic() == TUNE_QUIET)
		StopMusic();
	quiet = 0;
}

void Player()
{
	game_tick++;

	const Uint8 *keystate = SDL_GetKeyboardState(NULL);

	left = keystate[game_key[KEY_LEFT]];
	right = keystate[game_key[KEY_RIGHT]];
	up = keystate[game_key[KEY_UP]];
	down = keystate[game_key[KEY_BOMB]];
	fire = keystate[game_key[KEY_FIRE]];
	fire2 = keystate[game_key[KEY_ROCKET]];
	
	bool dummy;

	ReadGamepad(left, right, up, dummy, down, fire, fire2);

	Size isz = JetStoryImg::ship().GetSize();

	double motvol = 0;

	if(ship.shield > 0) {
		if(ship.fuel > 0 && !game_accomplished) {
			if(left) {
				back_flame_ani++;
				if(back_flame_ani >= bflame.GetCount())
					back_flame_ani = bflame.GetCount() / 2;
				motvol++;
				ship.speed.x -= 0.01;
				ship.left = true;
				ship.fuel--;
			}
			else
			if(right) {
				back_flame_ani++;
				if(back_flame_ani >= bflame.GetCount())
					back_flame_ani = bflame.GetCount() / 2;
				motvol++;
				ship.speed.x += 0.01;
				ship.left = false;
				ship.fuel--;
			}
			else
				back_flame_ani = 0;
			if(up) {
				up_flame_ani++;
				if(up_flame_ani >= lflame.GetCount())
					up_flame_ani = lflame.GetCount() / 2;
				motvol += 2;
				ship.speed.y -= 0.02;
				ship.fuel--;
			}
			else
				up_flame_ani = 0;
			ship.fuel = max(ship.fuel, 0);
		}
		
		ship.speed.x = clamp(ship.speed.x, -12.0, 12.0);
		ship.speed.y = max(ship.speed.y, -12.0);
		
		if(game_accomplished && ship.speed.y < 0)
			ship.speed.y = 0;
		
		double h = ship.Move(isz, 0.2, 0.997);
		if(h && ship.fuel <= 0)
			ship.shield--;
			
		if(h > 0.1)
			PlaySound(3, 0.5, "87.31:L50V100D50S50R30:V120f7:V120f3r5B::", min(85 * sqrt(h), 100.0), Random(80) + 50);
	
		if(fire && ship.ammo > 0) {
			if(game_tick - lastfire > 10) {
				PlaySound(4, 0.5, "783.99:L50V82r-15R40:V100D40S20B:V90f0.50r-20T::");
				lastfire = game_tick;
				Missile& m = missile.Add();
				m.kind = MISSILE_GUN;
				m.pos = ship.pos;
				m.speed.y = ship.speed.y - 0.1;
				m.speed.x = ship.speed.x + (ship.left ? -1 : 1) * 5;
				m.left = ship.left;
				ship.ammo--;
			}
			NotQuiet();
		}

		SetChannelVolume(0, motvol ? 60 + 3 * motvol : 0);
		
		int kind = down && !prevdown ? MISSILE_BOMB : fire2 && !prevfire2 ? MISSILE_ROCKET : 0;
		if(kind && (kind == MISSILE_ROCKET ? ship.rockets : ship.bombs) > 0) {
			Missile& m = missile.Add();
			m.kind = kind;
			m.pos = ship.pos;
			m.pos.y += 10;
			m.speed = ship.speed;
			m.left = ship.left;
			m.damage = 50;
			if(kind == MISSILE_ROCKET) {
				m.accelx = ship.left ? -0.05 : 0.05;
				PlaySound(4, 0.5, "349.23:L20V85R37T:V130D45S20B:V100f0.50r-3T::");
				ship.rockets--;
			}
			if(kind == MISSILE_BOMB) {
				m.speed.y += 0.5;
				PlaySound(4, 0.5, "349.23:L20V85R38W:V110D45S20N:V90f0.50r-5::");
				ship.bombs--;
			}
			NotQuiet();
		}

		prevdown = down;
		prevfire2 = fire2;
	}
	else
		SetChannelVolume(0, 0);
	
	Vector<int> done;
	Size msz = JetStoryImg::fire().GetSize();
	Size msz1 = JetStoryImg::hmissile0().GetSize();
	Size msz2 = JetStoryImg::bomb().GetSize();
	for(int i = 0; i < missile.GetCount(); i++) {
		NotQuiet();
		Missile& m = missile[i];
		m.speed.x += m.accelx;
		Size sz = decode(m.kind, MISSILE_ROCKET, msz1, MISSILE_BOMB, msz2, MISSILE_DEBRIS, Size(8, 8), msz);
		if(Distance(m.pos, ship.pos) > 2500 && m.kind != 3)
			done.Add(i);
		else
		if(m.Move(sz, 0, 1, m.kind ? 0.01 : 0.001) || m.kind == MISSILE_DEBRIS && Random(50) == 0) {
			Explosion(2, m.pos, m.kind ? 120 : 4, 20);
			if(m.kind)
				Scatter(m.pos);
			done.Add(i);
			if(m.kind == MISSILE_DEBRIS)
				Flash(30, Blend(Yellow(), LtRed(), Random(255)));
		}
		if(Random(2) == 0 && m.kind == MISSILE_ROCKET) { // rocket trail
			Debris& d = debris.Add();
			d.pos = m.pos;
			d.sz = Size(1, 1);
			d.start = game_tick;
			d.end = d.start + 5 + Random(25);
			d.gravity = 0;
		}
		m.rect = Rect(m.pos - sz / 2, sz);
	}
	missile.Remove(done);
	
	done.Clear();
	for(int i = 0; i < debris.GetCount(); i++) {
		Debris& d = debris[i];
		if(game_tick > d.end)
			done.Add(i);
		else
			d.Move(d.sz, 0.4, 1, d.gravity);
	}
	debris.Remove(done);

	bool dosavecheckpoint = false;
	Vector<Pointf> scatter;
	Size ssz = JetStoryImg::ship().GetSize();
	Rectf sr = Rectf(ship.pos - ssz / 2, ssz).Deflated(1);
	base_count = base_count0;
	training_counter = 0;
	training_block.Clear();
	for(int i = 0; i < enemy.GetCount(); i++) {
		Enemy& e = enemy[i];
		double distance = Distance(ship.pos, e.pos);
		e.distance = gameover ? 1e100 : distance;
		if(e.global || e.IsMissile() || distance < 2000) {
			e.Do();
			Sizef esz = e.GetSize();
			if(esz.cx == 0)
				continue;
			Rectf r(e.pos - esz / 2, esz);
			if(!e.IsMissile() && e.damage >= 0) {
				for(int j = 0; j < missile.GetCount(); j++) {
					Missile& m = missile[j];
					if(r.Intersects(m.rect)) {
						e.shield -= m.damage;
						if(e.shield > 0)
							Explosion(m.pos, m.kind ? 100 : 5, m.kind ? 20 : 4);
						if(m.kind)
							scatter.Add(m.pos);
						missile.Remove(j);
						if(e.damage == TRAININGTARGET)
							TrainingPoint();
						break;
					}
				}
			}
			if(!gameover && sr.Intersects(r) && e.IsActive()) {
				if(e.damage < 0) {
					if(e.damage == TRAININGPOINT)
						TrainingPoint();
					else
					if(e.damage == CHECKPOINT) {
						dosavecheckpoint = true;
						SetMessage(Null);
					}
					else {
						PlaySound(5, 0.5, "2092.99:L250V100R30:V90f3D40S0:V110D50S0B:V70f4:");
						switch(e.damage) {
						case POWERUP_ENERGY: ship.shield = SHIP_SHIELD; break;
						case POWERUP_FUEL: ship.fuel = 20000; break;
						case POWERUP_AMMO: ship.ammo = 500; break;
						case POWERUP_ROCKETS: ship.rockets = 100; break;
						case POWERUP_BOMBS: ship.bombs = 100; break;
						}
					}
					e.shield = 0;
				}
				else {
					PlaySound(5, 0.5, "65.41:L50V100A90R40W:V100f7:V120D45S0B:V100f10:", 100, Random(20) + 50);
					e.shield -= 100;
					ship.shield -= e.damage;
					Flash(50, Color(255, 111, 0));
					NotQuiet();
				}
			}
		}
	}
	
	training_remains = training_counter;
	
	for(Pointf p : scatter)
		Scatter(p);

	done.Clear();
	
	Vector<Point> explode_up;

	for(int i = 0; i < enemy.GetCount(); i++) {
		Enemy& e = enemy[i];
		if(e.shield <= 0 || game_accomplished && e.distance < 1000) {
			if(e.checkpoint)
				boss_checkpoint = 900;
			e.Die();
			done.Add(i);
			if(dynamic_cast<Box *>(&e))
				explode_up.Add(Point(e.pos) - e.GetSize() / 2 - Size(0, 2));
			if(!e.IsMissile() && e.damage >= 0 || game_accomplished) {
				if(game_accomplished)
					Explosion(e.pos, 10, 10, 0);
				else {
					int ed = 0;
					if(e.explosion_debris) {
						Flash(100, White());
						ed = Random(e.explosion_debris);
					}
					Sizef esz = e.GetSize();
					Explosion(e.pos, min(e.explosion_size * 50, 1000), min(esz.cx, esz.cy), ed);
				}
			}
		}
	}

	enemy.Remove(done);
	
	for(Point p : explode_up)
		for(Enemy& e : enemy)
			if(e.GetRect().Contains(p))
				e.shield = 0;
	
	if(cheating && ship.shield <= 0)
		ship.shield = 1;
	
	static int soundc[2];
	bool wsound[2] = { false };

	for(int i = 0; i < 2; i++) {
		int n = game_tick / (120 + i * 120);
		if(soundc[i] != n) {
			soundc[i] = n;
			wsound[i] = true;
		}
	}
	
	int val = min(ship.shield, ship.fuel / 100);
	if(val < 60 && !gameover && wsound[val > 30]) {
		PlaySound(3, 0.5, "130.81:L50V100r20A90R42Q::::", 100, 1000);
		NotQuiet();
	}

	if(TrainingIndex() >= training_total && training)
		training = false;
	
	if(ship.shield <= 0 && !gameover) {
		gameover = true;
		gameover_time = game_tick;
		Explosion(ship.pos, 500, 40, 50);
		Flash(100, White());
		StartMusic(TUNE_GAMEOVER);
	}
		
	if(!gameover && (dosavecheckpoint || boss_checkpoint == 0)) {
		Checkpoint();
		checkpoint_info = GameTick();
		PlaySound(5, 0.5, "87.31:L250V100R30W:V100::V50W:");
	}

	boss_checkpoint = max(-1, boss_checkpoint - 1);
	
	quiet++;
	if(quiet > 200 && CurrentMusic() == TUNE_NONE && !training)
		StartMusic(TUNE_QUIET);
	
	base_count_max = max(base_count_max, base_count);
}

void Checkpoint()
{
	Time tm = GetSysTime();
	String p = AppendFileName(ConfigFolder(),
	                          Format("%04d%02d%02d_%02d%02d%02d_%d.sav", (int)tm.year, (int)tm.month, (int)tm.day,
	                                 (int)tm.hour, (int)tm.minute, (int)tm.second, GameTick()));
	RealizePath(p);
	checkpoint_save = StoreGame();
	SaveFile(p, ZCompress(checkpoint_save));
}

void Ship::Serialize(Stream& s)
{
	Object::Serialize(s);
	s % left % shield % fuel % ammo % rockets % bombs;
}

void Debris::Serialize(Stream& s)
{
	Object::Serialize(s);
	s % sz % start % end % gravity % c1 % c2;
}

void Ship::Paint(Point sp)
{
	Size sz = JetStoryImg::ship().GetSize();

#ifdef BIGSHIP
	if(up && fuel > 0) {
		Image l = lflame[up_flame_ani];
		DrawImage(sp.x + (left ? 34 : 16) - l.GetWidth() / 2, sp.y + sz.cy - 2, l);
		Image s = sflame[up_flame_ani];
		DrawImage(sp.x + (left ? 14 : 35) - s.GetWidth() / 2, sp.y + sz.cy - 2, s);
	}

	if((::left || ::right) && fuel > 0) {
		Image m = bflame[back_flame_ani];
		DrawImage(sp.x + (left ? 47 : 1 - m.GetWidth()), sp.y - 5, m, left * SWAPX);
	}
#else
	if(up && fuel > 0) {
		Image l = lflame[up_flame_ani];
		DrawImage(sp.x + (left ? 23 : 11) - l.GetWidth() / 2, sp.y + sz.cy - 2, l);
		Image s = sflame[up_flame_ani];
		DrawImage(sp.x + (left ? 11 : 23) - s.GetWidth() / 2, sp.y + sz.cy - 2, s);
	}

	if((::left || ::right) && fuel > 0) {
		Image m = bflame[back_flame_ani];
		DrawImage(sp.x + (left ? 31 : 1 - m.GetWidth()), sp.y - 2, m, left * SWAPX);
	}
#endif

	DrawImage(sp.x, sp.y, JetStoryImg::ship(), ship.left * SWAPX);
}
