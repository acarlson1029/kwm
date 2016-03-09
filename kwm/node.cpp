#include "node.h"

////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
tree_node *CreateRootNode()
{
    tree_node *RootNode = new tree_node;

    RootNode->Element = NULL;
    RootNode->Parent = NULL;
    RootNode->LeftChild = NULL;
    RootNode->RightChild = NULL;

    return RootNode;
}

tree_node *CreateLeafNode(const tree_node &Parent)
{
    Assert(Parent, "CreateLeafNode()")

    tree_node *Leaf = CreateRootNode();
    *Leaf->Parent = Parent;

    return Leaf;
}

////////////////////////////////////////////////////////////////////////////////
// DESTRUCTORS
void DestroyNode(tree_node *Node)
{
    if(Node)
        delete Node;
}

void RemoveNodeFromTree(tree_node *Leaf)
{
    if(!Leaf || !IsLeafNode(*Leaf))
    {
        return;
    }

    // Case 1: Leaf is ROOT
    // Note: don't check against param RootNode because it can just be a subtree root
    if(IsRootNode(*Leaf))
    {
        // doesn't make sense to remove this node
        DEBUG("RemoveNodeFromTree() Tried to remove Tree Root - did you mean to Destroy Tree?")
        return; 
    } 


    tree_node *Parent = Leaf->Parent;
    tree_node *Sibling = IsLeftChild(*Leaf) ? Parent->RightChild : Parent->LeftChild;

    // TODO is there a better (memory management) way of doing this copy?
    Parent->Element = Sibling->Element;
    Parent->LeftChild = Sibling->LeftChild;
    Parent->RightChild = Sibling->RightChild;
    DestroyNode(Leaf);
    DestroyNode(Sibling);

    return;
}

////////////////////////////////////////////////////////////////////////////////
// QUERIES
bool IsLeafNode(const tree_node &Node)
{
    return (!Node.LeftChild) && (!Node.RightChild);
}

bool IsLeftChild(const tree_node &Node)
{
    if (Node.Parent)
        return Node.Parent->LeftChild == &Node;
    else
        return false;
}

bool IsLeftLeaf(const tree_node &Node)
{
    return IsLeftChild(Node) && IsLeafNode(Node);
}

bool IsRightChild(const tree_node &Node)
{
    if (Node.Parent)
        return Node.Parent->RightChild == &Node;
    else
        return false;
}

bool IsRightLeaf(const tree_node &Node)
{
    return IsRightChild(Node) && IsLeafNode(Node);
}

bool IsRootNode(const tree_node &Node)
{
    return !Node.Parent;
}

////////////////////////////////////////////////////////////////////////////////
// GET
tree_node *GetNodeIfLeaf(const tree_node &Node)
{
    tree_node *Leaf;
    *Leaf = Node;
    if(IsLeafNode(*Leaf))
        return Leaf;
    else
        return NULL;
}

tree_node *GetFirstLeafNode(const tree_node &Node)
{
    // TODO: Can probably implement this as an in-order search
    tree_node *First;
    *First = Node;
    while(First && !IsLeafNode(*First))
        First = First->LeftChild;

    return First;
}

tree_node *GetLastLeafNode(const tree_node &Node)
{
    tree_node *Last;
    *Last = Node;
    while(Last && !IsLeafNode(*Last))
        Last = Last->RightChild;

    return Last;
}

tree_node *GetNearestLeafNodeToTheLeft(const tree_node &Node)
{
    if(Node.Parent)
    {
        if(IsLeftChild(Node))
            return GetNearestLeafNodeToTheLeft(*Node.Parent);
        
        tree_node *Left = Node.Parent->LeftChild;
        if(IsLeafNode(*Left))
            return Left;

        return GetLastLeafNode(*Left);
    }

    return NULL;
}

tree_node *GetNearestLeafNodeToTheRight(const tree_node &Node)
{
    if(Node.Parent)
    {
        if(IsRightChild(Node))
            return GetNearestLeafNodeToTheRight(*Node.Parent);

        tree_node *Right = Node.Parent->RightChild;
        if(IsLeafNode(*Right))
            return Right;

        return GetFirstLeafNode(*Right);
    }

    return NULL;
}

tree_node *GetNearestLeafNeighbour(const tree_node &Node)
{
	if(IsLeafNode(Node))
	{
		return IsLeftLeaf(Node) ? GetNearestLeafNodeToTheRight(Node) : GetNearestLeafNodeToTheLeft(Node);
	}

	return NULL;
}

////////////////////////////////////////////////////////////////////////////////
// MUTATORS
