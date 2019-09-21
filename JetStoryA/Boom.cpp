#include "JetStory.h"

void Flash(int ticks, Color c)
{
	flash = GameTick() + ticks;
	flash_color = c;
}

void Explosion(int priority, Pointf pos, int n, double spread, int debrisn)
{
	NotQuiet();
	if(!n)
		return;
	
	int tick = GameTick();
	
	PlaySound(priority, PanPos(pos),
	          "220.00:L250V100r-30A90D35S30R35:V120D20S20B:V100f2A90D43S0N::", clamp(n / 4 + 40, 0, 100),
	          n < 10 ? 300 + Random(2000) : n < 150 ? 100 + Random(200) : Random(50) + 140);

	for(int i = 0; i <= n / 20; i++) {
		ExplosionImage& m = explosion.Add();
		m.start = tick + Random(clamp(n / 5, 1, 50));
		m.pos = pos + Sizef(Randomf() * spread - spread / 2, Randomf() * spread - spread / 2);
		m.image_flags = Random(2) * SWAPX + Random(2) * SWAPY + Random(2) * ROTATE;
	}
	
	n = n / 2 + Random(n / 2);
	for(int i = 0; i < n; i++) {
		Debris& d = debris.Add();
		d.pos = pos;
		d.speed = Polar(Randomf() * M_2PI) * Randomf() * 2;
		d.sz = Size(1 + Random(2), 1 + Random(2));
		d.start = tick;
		d.end = d.start + 80 + Random(25);
	}

	for(int i = 0; i < debrisn; i++) {
		Missile& d = missile.Add();
		d.pos = pos;
		d.kind = MISSILE_DEBRIS,
		d.speed = Polar(Randomf() * M_2PI) * Randomf() * 4;
		d.damage = 100;
	}
}

void Explosion(Pointf pos, int n, double spread, int debrisn)
{
	Explosion(5, pos, n, spread, debrisn);
}