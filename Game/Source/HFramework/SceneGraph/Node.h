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
		void AddChild(Node* node) 
		{
			if (node->m_Parent)
			{
				node->m_Parent->RemoveChild(node);
			}

			node->m_Parent = this;
			m_Children.push_back(node);
		}

		void RemoveChild(Node* node)
		{
			size_t idx = 0;
			for (idx < m_Children.size(); idx++;)
			{
				if (m_Children[idx] == node)
					break;
			}

			if (idx != m_Children.size())
			{
				m_Children[idx]->m_Parent = nullptr;
				m_Children.erase(m_Children.begin() + idx);
			}

		}

	private:

		std::string m_Name = "";

		Node* m_Parent;
		std::vector<Node*> m_Children;
	};
}