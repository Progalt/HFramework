#pragma once

#include <glm/glm.hpp>
#include "HFramework/Core/EventHandler.h"
#include <glm/gtc/matrix_transform.hpp>

class FPSCamera
{
public:

	void Update(float dt)
	{
		// Get mouse input

		if (m_MouseLock)
		{
			glm::fvec2 offset = hf::Mouse::GetRelativeMotion();
			offset.y = -offset.y;

			offset *= m_Sensitivity;

			m_Yaw += offset.x;
			m_Pitch += offset.y;

			if (m_Pitch > 89.0f) m_Pitch = 89.0f;
			if (m_Pitch < -89.0f) m_Pitch = -89.0f;

			glm::vec3 front;
			front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
			front.y = sin(glm::radians(m_Pitch));
			front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
			m_Front = glm::normalize(front);
		}

		// Get keyboard input

		float velocity = m_MoveSpeed * dt;

		if (hf::Keyboard::IsKeyHeld(hf::KeyCode::W))
			m_Position += velocity * m_Front;
		if (hf::Keyboard::IsKeyHeld(hf::KeyCode::S))
			m_Position -= velocity * m_Front;
		if (hf::Keyboard::IsKeyHeld(hf::KeyCode::A))
			m_Position -= velocity * glm::normalize(glm::cross(m_Front, m_Up));
		if (hf::Keyboard::IsKeyHeld(hf::KeyCode::D))
			m_Position += velocity * glm::normalize(glm::cross(m_Front, m_Up));

		if (hf::Keyboard::WasKeyPressed(hf::KeyCode::Esc))
		{
			hf::Log::Info("Switching mouse lock");
			m_MouseLock = !m_MouseLock;
			hf::Mouse::LockMouse(m_MouseLock);
		}

		
	} 

	const glm::mat4& GetMatrix() 
	{ 
		glm::mat4 view = glm::mat4(1.0f);

		view = glm::lookAt(m_Position, m_Position + m_Front, m_Up);

		return view;
	}

private:

	glm::vec3 m_Position = glm::vec3(0.0f);
	glm::vec3 m_Front = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 m_Up = glm::vec3(0.0f, 1.0f, 0.0f);

	float m_MoveSpeed = 3.0f;

	bool m_MouseLock = false;

	float m_Yaw = -90.0f;
	float m_Pitch = 0.0f;

	float m_Sensitivity = 0.5f;

};