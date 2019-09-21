#include "JetStory.h"

struct FlameGenerator {
	struct Particle : Moveable<Particle> {
		Pointf pos;
		Pointf speed;
		Color  from;
		Color  to;
		double time;
	};

	Vector<Particle> particles;
	
	Image Generate();

	Size size = Size(128, 128);
	Pointf speed = Pointf(2, 2);
	Pointf accel = Pointf(0.0005, 0.01);
	double gen = 2;
	int fade = 25;
	int alpha = 4;
};

Image FlameGenerator::Generate()
{
	ImageBuffer ib(size);
	Fill(ib, ib.GetSize(), RGBAZero());
	Vector<int> remove;
	for(int i = 0; i < particles.GetCount(); i++) {
		Particle& p = particles[i];
		p.pos += p.speed;
		p.speed.x += accel.x * (size.cx / 2 - p.pos.x);
		p.speed.y -= accel.y * abs(p.speed.x);
		p.time += abs(size.cx / 2 - p.pos.x) / fade + 1;
		if(p.time > 64)
			remove.Add(i);
		else {
			Point pp = p.pos;
			if(Rect(size).Contains(pp)) {
				Color c = Blend(p.from, p.to, 4 * (int)p.time);
				RGBA b = c;
				b.a = (64 - (int)p.time) / alpha;
				Premultiply(&b, &b, 1);
				AlphaBlend(&ib[pp.y][pp.x], &b, 1);
//				ib[pp.y][pp.x] = c;
			}
			else
				remove.Add(i);
		}
	}
	particles.Remove(remove);
	int n = int(gen * (Random(size.cx) + 4 * size.cx));
//	int n = 20 + Random(10);
	for(int i = 0; i < 8 * n; i++) {
		Particle& p = particles.Add();
		p.pos.x = Random(size.cx / 2) + size.cx / 4;
		p.pos.y = Randomf();
		p.speed.x = 0;
		p.speed.x = speed.x * (Randomf() - 0.5);
		p.speed.y = speed.y * (Randomf() + 0.5);
		p.from = Color(255 - Random(50), 255 - Random(50), Random(100));
		p.to = Color(Random(100) + 155, 0, 0);
		p.time = 0;
	}
	return ib;
};

Vector<Image> lflame, sflame, bflame;

void MakeFlames()
{
	{
		FlameGenerator l, s, b;

	#ifdef BIGSHIP
		b.size = Size(28, 64);
		b.speed = Pointf(0.7, 1);
		b.fade = 8;
		b.gen = 0.9;

		l.size = Size(30, 64);
		l.speed = Pointf(0.7, 1);
		l.fade = 6;
		l.gen = 0.7;

		s.size = Size(17, 64);
		s.speed = Pointf(0.2, 0.4);
		s.accel = Pointf(0.0009, 0.02);
		s.fade = 4;
		s.gen = 0.5;
	#else
		b.size = Size(18, 64);
		b.speed = Pointf(0.7, 1);
		b.fade = 8;
		b.gen = 0.9;

		l.size = Size(15, 64);
		l.speed = Pointf(0.5, 0.5);
		l.fade = 7;
		l.gen = 0.7;

		s.size = Size(10, 64);
		s.speed = Pointf(0.2, 0.4);
		s.accel = Pointf(0.0009, 0.02);
		s.fade = 4;
		s.gen = 0.5;
	#endif
		
		for(int pass = 0; pass < 2; pass++) {
			SeedRandom(0);
			for(int i = 0; i < 32; i++) {
				l.Generate();
				lflame.Add(l.Generate());
				s.Generate();
				sflame.Add(s.Generate());
				b.Generate();
				bflame.Add(RotateClockwise(b.Generate()));
			}
		}
	}
	
	SeedRandom();
}
