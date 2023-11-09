
#include "EventHandler.h"

namespace hf
{
	ButtonState Keyboard::m_KeyStates[(int)KeyCode::__Count];

	glm::uvec2 Mouse::m_CurrentPosition;
	glm::ivec2 Mouse::m_RelativeMotion;
}