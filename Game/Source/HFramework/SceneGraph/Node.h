#pragma once

#include <string>
#include <vector>

namespace hf
{
	class Node
	{
	public:

		Node(const std::string& name = "") : m_Name(name) { }


		Node* GetParent() const { return m_Parent; }

	private:

		std::string m_Name = "";

		Node* m_Parent;
		std::vector<Node*> m_Children;
	};
}