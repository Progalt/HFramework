#pragma once
#include "Platform.h"


namespace hf
{
	enum class KeyCode
	{
		Unknown, 
		A,
		B,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z,

		Space,
        Esc,

		__Count
	};

	inline KeyCode FromSDL2Scancode(SDL_Scancode scancode)
	{
		switch (scancode)
		{
		case SDL_SCANCODE_A:
			return KeyCode::A;
			break;
        case SDL_SCANCODE_B:
            return KeyCode::B;
            break;
        case SDL_SCANCODE_C:
            return KeyCode::C;
            break;
        case SDL_SCANCODE_D:
            return KeyCode::D;
            break;
        case SDL_SCANCODE_E:
            return KeyCode::E;
            break;
        case SDL_SCANCODE_F:
            return KeyCode::F;
            break;
        case SDL_SCANCODE_G:
            return KeyCode::G;
            break;
        case SDL_SCANCODE_H:
            return KeyCode::H;
            break;
        case SDL_SCANCODE_I:
            return KeyCode::I;
            break;
        case SDL_SCANCODE_J:
            return KeyCode::J;
            break;
        case SDL_SCANCODE_K:
            return KeyCode::K;
            break;
        case SDL_SCANCODE_L:
            return KeyCode::L;
            break;
        case SDL_SCANCODE_M:
            return KeyCode::M;
            break;
        case SDL_SCANCODE_N:
            return KeyCode::N;
            break;
        case SDL_SCANCODE_O:
            return KeyCode::O;
            break;
        case SDL_SCANCODE_P:
            return KeyCode::P;
            break;
        case SDL_SCANCODE_Q:
            return KeyCode::Q;
            break;
        case SDL_SCANCODE_R:
            return KeyCode::R;
            break;
        case SDL_SCANCODE_S:
            return KeyCode::S;
            break;
        case SDL_SCANCODE_T:
            return KeyCode::T;
            break;
        case SDL_SCANCODE_U:
            return KeyCode::U;
            break;
        case SDL_SCANCODE_V:
            return KeyCode::V;
            break;
        case SDL_SCANCODE_W:
            return KeyCode::W;
            break;
        case SDL_SCANCODE_X:
            return KeyCode::X;
            break;
        case SDL_SCANCODE_Y:
            return KeyCode::Y;
            break;
        case SDL_SCANCODE_Z:
            return KeyCode::Z;
            break;
        case SDL_SCANCODE_SPACE:
            return KeyCode::Space;
            break;
        case SDL_SCANCODE_ESCAPE:
            return KeyCode::Esc;
            break;
        default:
            return KeyCode::Unknown;
		}
	}
}