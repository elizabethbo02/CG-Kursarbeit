#pragma once

#include <string>

enum NodeType
{
	nt_Node = 0,
	nt_GroupNode,
	nt_TransformNode,
	nt_GeometryNode
};

class Node
{
	static unsigned int genID;

protected:
	std::string name;
	unsigned int ID;
	NodeType type;

public:
	Node()
	{
		name = "";
		ID = ++genID;
		type = nt_Node;
	}

	Node(const std::string& name, NodeType t = nt_Node)
	{
		this->name = name;
		ID = ++genID;
	}

	virtual void Traverse() = 0;

	/*
	//find node by name
	virtual Node* FindNodeByName(const std::string& nameToFind);
*/
};

/*
//find node by name
inline Node* Node::FindNodeByName(const std::string& nameToFind) {
	if (name == nameToFind) {
		return this;
	}
	return nullptr;
}
*/