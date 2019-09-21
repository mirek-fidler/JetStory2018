#include "JetStory.h"

bool Enemy::CanSee(Pointf ref) const
{
	if(distance > 1000)
		return false;
	Pointf unit16 = 16 * (ref - pos) / distance;
	double ch = 0;
	Pointf p = pos + unit16;
	while(ch < distance - 32) {
		if(MapCollision(p))
			return false;
		p += unit16;
		ch += 16;
	}
	return true;
}

bool Enemy::CanSee() const
{
	return CanSee(ship.pos);
}

void Enemy::Serialize(Stream& s)
{
	Object::Serialize(s);
	s % startpos
	  % image_flags
	  % shield
	  % z
	  % randomize
	;
	
	static Index<int64> image_serial;
	static Index<String> image_id;
	ONCELOCK {
		for(int i = 0; i < JetStoryImg::GetCount(); i++) {
			image_id.Add(JetStoryImg::GetId(i));
			image_serial.Add(JetStoryImg::Get(i).GetSerialId());
		}
	}
	
	String id;
	if(s.IsStoring()) {
		int q = image_serial.Find(image.GetSerialId());
		if(q >= 0)
			id = image_id[q];
	}
	s % id;
	if(IsNull(id))
		s % image;
	else
	if(s.IsLoading()) {
		int q = image_id.Find(id);
		if(q >= 0)
			image = JetStoryImg::Get(q);
	}
	
	SerializeKind(s);
}

void Enemy::Do()
{
	Move(image.GetSize(), 0.4);
}

void Enemy::Beep(int priority, const char *s, double volume, double freq, double max_distance)
{
	if(distance < max_distance)
		PlaySound(priority, PanPos(pos), s, clamp((max_distance - distance) / max_distance * 2, 0.0, 1.0) * volume, freq);
}

void Enemy::Put(Point pos_)
{
	startpos = pos = pos_;
}

REGISTER(Box);

struct BigBox : Box {
	BigBox() { image = JetStoryImg::bigbox(); shield = explosion_size = 20; z = 2; }
};

REGISTER(BigBox);
