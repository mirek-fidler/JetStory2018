#include "JetStory.h"

void ReadGamepad(bool& left, bool& right, bool& up, bool& down, bool& bomb, bool& fire, bool& fire2)
{
	for(auto g : GamePad)
	    if(SDL_GameControllerGetAttached(g)) {
	        // NOTE: We have a controller with index ControllerIndex.
	        bool Up = SDL_GameControllerGetButton(g, SDL_CONTROLLER_BUTTON_DPAD_UP);
	        bool Down = SDL_GameControllerGetButton(g, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
	        bool Left = SDL_GameControllerGetButton(g, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
	        bool Right = SDL_GameControllerGetButton(g, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
	        bool Start = SDL_GameControllerGetButton(g, SDL_CONTROLLER_BUTTON_START);
	        bool Back = SDL_GameControllerGetButton(g, SDL_CONTROLLER_BUTTON_BACK);
	        bool LeftShoulder = SDL_GameControllerGetButton(g, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
	        bool RightShoulder = SDL_GameControllerGetButton(g, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
	        bool AButton = SDL_GameControllerGetButton(g, SDL_CONTROLLER_BUTTON_A);
	        bool BButton = SDL_GameControllerGetButton(g, SDL_CONTROLLER_BUTTON_B);
	        bool XButton = SDL_GameControllerGetButton(g, SDL_CONTROLLER_BUTTON_X);
	        bool YButton = SDL_GameControllerGetButton(g, SDL_CONTROLLER_BUTTON_Y);
	
	        int16 StickX = SDL_GameControllerGetAxis(g, SDL_CONTROLLER_AXIS_LEFTX);
	        int16 StickY = SDL_GameControllerGetAxis(g, SDL_CONTROLLER_AXIS_LEFTY);
	        
	        left = left || Left;
	        right = right || Right;
	        up = up || Up;
	        down = down || Down;
	        fire = fire || RightShoulder;
	        fire2 = fire2 || BButton || XButton;
	        bomb = bomb || AButton || YButton;
	    }
	
	for(auto j : Joystick) {
		if(SDL_JoystickNumAxes(j)) {
			int x_move = SDL_JoystickGetAxis(j, 0);
			int y_move = SDL_JoystickGetAxis(j, 1);
			
			if(x_move < -20000)
				left = true;
			if(x_move > 20000)
				right = true;
			if(y_move < -20000)
				up = true;
			if(y_move > 20000)
				down = true;
		}
		
		int n = SDL_JoystickNumButtons(j);
		for(int i = 0; i < n; i++)
			JoystickButton.At(i) = SDL_JoystickGetButton(j, i);
		
		auto is = [&](int i) -> bool { return i < JoystickButton.GetCount() ? JoystickButton.At(i) : false; };
		
		fire = fire || is(5);
		fire2 = fire2 || is(7) || is(3) || is(1);
		bomb = bomb || is(4) || is(0) || is(2);
	}
}