#include "JetStory.h"

ArrayMap<String, void (*)(One<Enemy>&)>& EnemyFactory()
{
	static ArrayMap<String, void (*)(One<Enemy>&)> h;
	return h;
}

Vector<bool>& DesignEnemy()
{
	static Vector<bool> b;
	return b;
}

VectorMap<String, String>& TypeId()
{
	static VectorMap<String, String> map;
	return map;
}

String Enemy::GetKindId() const
{
	return TypeId().Get(typeid(*this).name(), Null);
}

void RegisterEnemy(const char *id, const char *type_id, void (*factory)(One<Enemy>&), bool design)
{
	TypeId().Add(type_id, id);
	EnemyFactory().Add(id, factory);
	DesignEnemy().Add(design);
}

bool CreateEnemy(const String& id, One<Enemy>& h)
{
	void (*factory)(One<Enemy>&) = EnemyFactory().Get(id, NULL);
	if(factory) {
		factory(h);
		return true;
	}
	return false;
}

void AddEnemy(const String& id, Point pos)
{
	One<Enemy> h;
	CreateEnemy(id, h);
	enemy.Add(h.Detach());
	enemy.Top().Put(pos);
}

Array<Epos> emap;

void ResetEnemy()
{
	training_counter = 0;
	enemy.Clear();
	for(const auto& e : emap) {
		Point p = e.pos;
		AddEnemy(e.id, p);
	}
	training_total = training_counter;
}

// ENEMY(Pacman, 5864, 2136)
void LoadEnemy()
{
	enemy.Clear();
	#define ENEMY(eid, x, y) { Epos& p = emap.Add(); p.id = #eid; p.pos = Point(x, y); }
	#include "data.i"
	#undef  ENEMY
	
	#ifndef flagDEVEL
	for(int i = 0; i < 150; i++) {
		Epos& p = emap.Add();
		p.id = "Pacman";
		p.pos = Point(5874, 1368);
	}
	#endif

	for(auto& e : emap) {
		if(e.pos.y >= 128 * BLOCKSIZE) { // fix old training
			e.pos.y -= (128 - 16) * BLOCKSIZE;
			extern int training_left;
			e.pos.x += training_left;
		}
	}
	
	base_count_max = 0;
	for(const auto& e : emap)
		if(e.id == "Base" || e.id == "BasePlatform")
			base_count_max++;

	ResetEnemy();
}

void SaveEnemy()
{
	FileOut out(GetDataFile("data.i"));
	
	for(const auto& e : emap)
		if(e.id != "Pacman")
			out << "ENEMY(" << e.id << ", " << e.pos.x << ", " << e.pos.y << ")\r\n";
}
