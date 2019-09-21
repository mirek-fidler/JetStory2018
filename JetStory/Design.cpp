#include "JetStory.h"

ArrayMap<String, Enemy> enemy_image;
static int current_enemy;

int FindEnemy(Point pos)
{
	for(int i = 0; i < emap.GetCount(); i++) {
		auto& m = emap[i];
		Size sz = enemy_image.Get(m.id).GetSize();
		if(Rect(m.pos - sz / 2, sz).Contains(pos))
			return i;
	}
	return -1;
}

Enemy& CurrentImage()
{
	static Wasp cancel;
	ONCELOCK {
		cancel.image = JetStoryImg::Cancel();
	};
	return current_enemy < 0 ? cancel : enemy_image[current_enemy];
}

void Design()
{
	ONCELOCK {
		const ArrayMap<String, void (*)(One<Enemy>&)>& h =  EnemyFactory();
		const Vector<bool>& b = DesignEnemy();
		for(int i = 0; i < h.GetCount(); i++) {
			if(b[i]) {
				One<Enemy> e;
				h[i](e);
				e->Put(Point(0, 0));
				e->Do();
				enemy_image.Add(h.GetKey(i), e.Detach());
			}
		}
	}
	
	Point pos;
	
	auto dopos = [&] {
		Point mouse_pos;
		SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
		Point p = mouse_pos + (Point)ship.pos - window_size / 2;
		pos = p + CurrentImage().GetSize() / 2;
		pos = pos / 4 * 4;
	};
	
	dopos();

	for(;;) {
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			dopos();
			switch(event.type) {
			case SDL_QUIT:
				return;
			case SDL_KEYDOWN:
				switch(event.key.keysym.sym) {
				case SDLK_F1:
				case SDLK_ESCAPE:
					return;
				case SDLK_UP: ship.pos.y -= 32; break;
				case SDLK_DOWN: ship.pos.y += 32; break;
				case SDLK_LEFT: ship.pos.x -= 32; break;
				case SDLK_RIGHT: ship.pos.x += 32; break;
				}
				break;
			case SDL_MOUSEWHEEL:
				current_enemy = clamp(current_enemy + sgn(event.wheel.y), 0, enemy_image.GetCount() - 1);
				break;
			case SDL_MOUSEBUTTONDOWN: {
				int ii = FindEnemy(pos);
				switch(event.button.button) {
				case SDL_BUTTON_LEFT:
					if(current_enemy < 0) {
						if(ii >= 0)
							emap.Remove(ii);
					}
					else
					if(current_enemy >= 0) {
						Epos& e = emap.Add();
						e.id = enemy_image.GetKey(current_enemy);
						e.pos = pos;
						break;
					}
					break;
				case SDL_BUTTON_RIGHT:
					if(ii < 0)
						current_enemy = -1;
					else
						current_enemy = enemy_image.Find(emap[ii].id);
					break;
				}
			}
			}
		}
		
		enemy.Clear();
		for(const auto& e : emap) {
			One<Enemy> h;
			EnemyFactory().Get(e.id)(h);
			enemy.Add(h.Detach());
			enemy.Top().Put(e.pos);
			enemy.Top().Do();
			enemy.Top().pos = e.pos;
		}
		PaintScene(window_size, gl_draw);

		Enemy& h = CurrentImage();
		Point p = pos - (Point)ship.pos + window_size / 2 - h.GetSize() / 2;
		h.Paint(p);

		SDL_GL_SwapWindow(sdl_window);
	}
}
