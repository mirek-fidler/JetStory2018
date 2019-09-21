#include "JetStory.h"

#if 0
bool DoMove(Pointf& pos, double& x, double& dx, double bounce, double& last_bounce, double& dy, double friction, Size isz, double gravity)
{
	double xx = (int)x;
	double xx0 = x;
	x += dx;
	if(MapCollision(pos, isz) && dx) {
		x = xx;
		if(dx > 0) {
			while(dx > 0) {
				x = x + 1;
				if(MapCollision(pos, isz)) {
					x = xx;
					break;
				}
				dx -= x - xx;
				xx = x;
			}
		}
		else {
			while(dx < 0) {
				x = x - 1;
				if(MapCollision(pos, isz)) {
					x = xx;
					break;
				}
				dx += xx - x;
				xx = x;
			}
		}
		if(abs(dx) < 2 * gravity)
			dx = 0;
		last_bounce = dx;
		dx *= -bounce;
		dy *= friction;
		return true;
	}
	return false;
}

double Object::Move(Size sz, Pointf bounce, Pointf friction, double gravity)
{
	speed.y += gravity;
	
	last_bounce = Pointf(0, 0);
	
	return DoMove(pos, pos.x, speed.x, bounce.x, last_bounce.x, speed.y, 0 * friction.y, sz, gravity) +
	       DoMove(pos, pos.y, speed.y, bounce.y, last_bounce.y, speed.x, friction.x, sz, gravity);
}

double Object::Move(Size sz, double bounce, double friction, double gravity)
{
	return Move(sz, Pointf(bounce, bounce), Pointf(friction, 0), gravity);
}

#else

bool DoMove(Pointf& pos, double dx, double dy, Size isz)
{
	Pointf p2 = pos;
	p2.x += dx;
	p2.y += dy;

	if(MapCollision(p2, isz)) {
		double l = 0;
		double h = 1;
		for(int i = 0; i < 10 && abs(h - l) > 0.01; i++) {
			double p = (h + l) / 2;

			Pointf p2 = pos;
			p2.x += p * dx;
			p2.y += p * dy;

			if(MapCollision(p2, isz))
				h = p;
			else
				l = p;
		}
		pos.x += l * dx;
		pos.y += l * dy;
		return true;
	}
	pos = p2;
	return false;
}

double Object::Move(Size sz, Pointf bounce, Pointf friction, double gravity)
{
	speed.y += gravity;

	last_bounce = Pointf(0, 0);

	double r = 0;
	if(DoMove(pos, speed.x, 0, sz)) {
		r = abs(speed.x);
		speed.x *= -bounce.x;
		last_bounce.x = speed.x;
		speed.y *= friction.y;
	}
	if(DoMove(pos, 0, speed.y, sz)) {
		r = max(r, abs(speed.y));
		speed.y *= -bounce.y;
		last_bounce.y = speed.y;
		speed.x *= friction.x;
	}
	return r;
}

double Object::Move(Size sz, double bounce, double friction, double gravity)
{
	return Move(sz, Pointf(bounce, bounce), Pointf(friction, friction), gravity);
}
#endif