#include "JetStory.h"

struct Ball : Enemy {
	void SpeedX() {
		if(pos.x < -10 * BLOCKSIZE)
			speed.x = Randomf();
		else
		if(pos.x > (MAPX + 10) * BLOCKSIZE)
			speed.x = -Randomf();
		else
			speed.x = (Randomf() - 0.5) * 2;
	}
	
	virtual void Do() {
		double h = Move(image.GetSize(), 0.9);
		if(h > 0.1)
			Beep("123.47:L20V100r-2A80D60S5R35:V100D50S0:V100f7D30:V80f4:", min(100 * h + 50, 100.0), 80 + Random(40));
		if(abs(speed.x) + abs(speed.y) < 0.1 && Random(500) == 0 && h) {
			speed.y = -Randomf() * 2 - 1;
			SpeedX();
			Beep(1, "2092.99:L250V100r-15R40W:V80N:V120f3r-15:V120f5:", 80, 1800 + Random(400));
		}
	}

	virtual void SerializeKind(Stream& s) {}
	
	Ball() { image = JetStoryImg::ball(); speed.y = 0; SpeedX(); global = true; }
};

REGISTER(Ball);

struct BigBall : Enemy {
	void SpeedX() {
		if(pos.x < -10 * BLOCKSIZE)
			speed.x = Randomf();
		else
		if(pos.x > (MAPX + 10) * BLOCKSIZE)
			speed.x = -Randomf();
		else
			speed.x = (Randomf() - 0.5) * 4;
	}

	virtual void Die() {
		NotQuiet();
		int n = Random(30) + 30;
		for(int i = 0; i < n; i++) {
			Ball& w = enemy.Create<Ball>();
			w.Put(pos);
			w.SpeedX();
			w.speed.y = Randomf() - 0.5;
			w.speed *= 3;
		}
	}

	virtual void Do() {
		double h = Move(image.GetSize(), 0.9);
		if(h > 0.1)
			Beep("87.31:L50V100r-2A80R38:V100D50S0B:V100f7D30:V80f4:", min(100 * h + 70, 100.0), 80 + Random(20));
		if(abs(speed.x) + abs(speed.y) < 0.1 && Random(500) == 0 && h) {
			speed.y = -Randomf() * 4 - 1;
			SpeedX();
			Beep(2, "2092.99:L250V100r-15R40W:V80N:V120f3r-15:V120f5:", 90, 800 + Random(200));
		}
	}

	virtual void SerializeKind(Stream& s) {}
	
	BigBall() { image = JetStoryImg::bigball(); speed.y = 0; SpeedX(); shield = 50; explosion_size = 10; }
};

REGISTER(BigBall);
