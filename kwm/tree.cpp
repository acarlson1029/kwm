#include "tree.h"
#include "node.h" // trees contain nodes; the next abstraction level down
#include "container.h"

////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//
tree_node *CreateTree()
{
    return CreateRootNode();
}

template <typename T>
tree_node *CreateTreeFromElements(const std::vector<T*> &Elements)
{
    tree_node *RootNode = CreateTree();

    typename std::vector<T*>::iterator ElemIt, end;
    for(ElemIt = Elements.begin(), end = Elements.end(); ElemIt != end; ++ElemIt)
    {
        AddElementToTree(RootNode, *(*ElemIt));
    }
}

////////////////////////////////////////////////////////////////////////////////
// DESTRUCTORS
//
void DestroyTree(tree_node *RootNode)
{
    PostOrderMutation(RootNode, DestroyNode);
}

///////////////////////////////////////////////////////////////////////////////
//  FIND ELEMENT IN TREE
//
template <typename T>
tree_node *PreOrderSearch(const tree_node &RootNode, const T &Element)
{
	tree_node *Node;
	*Node = RootNode;
	if(Node->Element == Element)
		return Node;

	Node = PreOrderSearch(*RootNode.LeftChild, Element);
	if(Node->Element == Element)
		return Node;

	Node = PreOrderSearch(*RootNode.RightChild, Element);
	if(Node->Element == Element)
		return Node;

    return NULL;
}

template <typename T>
T *PreOrderSearch(const tree_node &RootNode, T *(*f)(const tree_node &Node))
{
	T *Element;

	Element = f(RootNode);
	if(Element)
		return Element;

	Element = PreOrderSearch(*RootNode.LeftChild, f);
	if(Element)
		return Element;

	Element = PreOrderSearch(*RootNode.RightChild, f);
	if(Element)
		return Element;

    return NULL;
}

template <typename T>
tree_node *PostOrderSearch(const tree_node &RootNode, const T &Element)
{
	tree_node *Node;

	Node = PostOrderSearch(*RootNode.LeftChild, Element);
	if(Node->Element == Element)
		return Node;

	Node = PostOrderSearch(*RootNode.RightChild, Element);
	if(Node->Element == Element)
		return Node;

	*Node = RootNode;
	if(Node->Element == Element)
		return Node;

    return NULL;
}

template <typename T>
T *PostOrderSearch(const tree_node &RootNode, T *(*f)(const tree_node &Node))
{
	T *Element;

	Element = PostOrderSearch(*RootNode.LeftChild, f);
	if(Element)
		return Element;

	Element = PostOrderSearch(*RootNode.RightChild, f);
	if(Element)
		return Element;

	Element = f(RootNode);
	if(Element)
		return Element;

    return NULL;
}

template <typename T>
tree_node *InOrderSearch(const tree_node &RootNode, const T &Element)
{
	tree_node *Node;

	Node = InOrderSearch(RootNode.LeftChild, Element);
	if(Node->Element == Element)
		return Node;

	*Node = RootNode;
	if(Node->Element == Element)
		return Node;

	Node = InOrderSearch(RootNode.RightChild, Element);
	if(Node->Element == Element)
		return Node;

    return NULL;
}

template <typename T>
T *InOrderSearch(const tree_node &RootNode, T *(*f)(const tree_node &Node))
{
    T *Element;

    Element = InOrderSearch(*RootNode.LeftChild, f);
    if(Element)
        return Element;

    Element = f(RootNode);
    if(Element)
        return Element;

    Element = InOrderSearch(*RootNode.RightChild, f);
    if(Element)
        return Element;

    return NULL;
}

template <typename T>
tree_node *LevelOrderSearch(const tree_node &RootNode, const T &Element) // Search for NODE from ELEMENT
{
    tree_node *Node;
    *Node = RootNode;
    std::queue<tree_node*> NodeQueue;
    for(NodeQueue.push(Node); !NodeQueue.empty(); NodeQueue.pop())
    {
        tree_node *Node = NodeQueue.front();

        if(Node->Element == Element)
            return Node;
        
        if(Node->LeftChild)
            NodeQueue.push(Node->LeftChild);

        if(Node->RightChild)
            NodeQueue.push(Node->RightChild);
    }
    return NULL;
}

template <typename T>
T *LevelOrderSearch(const tree_node &RootNode, T *(*f)(const tree_node &Node)) // Search for ELEMENT from NODE
{
    tree_node *Node;
    std::queue<tree_node*> NodeQueue;
    T *Element;
    
    *Node = RootNode;
    for(NodeQueue.push(Node); !NodeQueue.empty(); NodeQueue.pop())
    {
        Node = NodeQueue.front();
        Element = f(*Node);

        if(Element)
            return Element;
        
        if(Node->LeftChild)
            NodeQueue.push(Node->LeftChild);

        if(Node->RightChild)
            NodeQueue.push(Node->RightChild);
    }
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
//  QUERIES
//
template <typename T>
bool IsElementInTree(const tree_node &RootNode, const T &Element)
{
    tree_node *Node = PreOrderSearch(RootNode, Element);
        
    return (Node && *Node->Element == Element);
}

////////////////////////////////////////////////////////////////////////////////
//  GET ELEMENT FROM TREE
//
template <typename T>
std::vector<T*> *LevelOrderCollection(const tree_node &RootNode, T *(*f)(const tree_node &Node))
{
    // Like LevelOrderSearch, but instead of returning the first item found, it returns all items found.
    // TODO Can I genericize this to always return the collection and just use the first returned value?
    std::queue<tree_node*> NodeQueue;
    std::vector<T*> Elements;
    tree_node *Node;
    T *Element;

    *Node = RootNode;
    for(NodeQueue.push(Node); !NodeQueue.empty(); NodeQueue.pop())
    {
        Element = f(*NodeQueue.front());

        if(Element)
            Elements.push_back(Element);

        if(Node->LeftChild)
            NodeQueue.push(Node->LeftChild);

        if(Node->RightChild)
            NodeQueue.push(Node->RightChild);
    }

    return &Elements;
}

template <typename T>
T *GetFirstElement(const tree_node &Node)
{
    tree_node *First = GetFirstLeafNode(Node);
    if(First)
        return First->Element;
    else
        return NULL;
}

template <typename T>
T *GetLastElement(const tree_node &Node)
{
    tree_node *Last = GetLastLeafNode(Node);
    if(Last)
        return Last->Element;
    else
        return NULL;
}

template <typename T>
T *GetNearestElementToTheLeft(const tree_node &Node)
{
    if(IsRootNode(Node))
        return Node.Element;
    
    if(IsLeftChild(Node))
        return GetNearestElementToTheLeft<T>(*Node.Parent);
            
    tree_node *Left = Node.Parent->LeftChild;
    if(IsLeafNode(*Left))
        return Left->Element;

    return GetLastElement<T>(*Left);
}

template <typename T>
T *GetNearestElementToTheRight(const tree_node &Node)
{

    if(IsRootNode(Node))
        return Node.Element;

    if(IsRightChild(Node))
        return GetNearestElementToTheRight<T>(Node.Parent);
            
    tree_node *Right = Node.Parent->RightChild;
    if(IsLeafNode(*Right))
        return Right;

    return GetFirstElement<T>(*Right);
}

template <typename T ,typename K, typename R>
R *GetElementByKey(const tree_node &Node, const K &Key, R *(*f)(const T &Element, const K &Key))
{
    R *FoundValue;

    FoundValue = f(Node.Element, Key);
    if(FoundValue)
        return FoundValue;

    FoundValue = GetElementByKey<R>(Node.LeftChild, Key, f);
    if(FoundValue)
        return FoundValue;

    FoundValue = GetElementByKey<R>(Node.RightChild, Key, f);
    if(FoundValue)
        return FoundValue;

    return NULL;
}

// TODO Move this function out of TREE
// It has nothing to do with a TREE ADT
// Using window_stack explicitly because we're breaking abstraction to access container information.
window_stack *GetElementAtPosition(const tree_node &Node, const int &X, const int &Y)
{
    if(IsLeafNode(Node))
    {
        if(IsPointInContainer(Node.Element->Container, X, Y))
            return Node.Element;
        else
            return NULL;
    }
    if(IsPointInContainer(Node.LeftChild->Element->Container, X, Y))
        return GetElementAtPosition(*Node.LeftChild, X, Y);

    if(IsPointInContainer(Node.RightChild->Element->Container, X, Y))
        return GetElementAtPosition(*Node.RightChild, X, Y);

    return NULL;
}

//window_stack *GetFirstPseudoLeafNode(tree_node *Node); // TODO balanced default position

////////////////////////////////////////////////////////////////////////////////
//  INSERT ELEMENT INTO TREE
//  TODO does this need to be boolean?
template <typename T>
bool InsertElementAtNode(tree_node *Node, const T &Element)
{
    // NOTE - Calling functions need to manage the containers of the elements accordingly
    
    if(!Node)
        return false;

    if(IsRootNode(*Node) && !Node->Element)
    {
        Node->Element = Element;
        return true;
    }

    tree_node *Left = CreateLeafNode(*Node);
    tree_node *Right = CreateLeafNode(*Node);

    // SET the values in the Tree.
    // TODO Add switch for spawn location
    Left->Element = Element;
    Right->Element = Node->Element;

    // Reset the value in Node
    Node->LeftChild = Left;
    Node->RightChild = Right;
    Node->Element = NULL;

    return true;
}

////////////////////////////////////////////////////////////////////////////////
//  ADD ELEMENT TO TREE
//
template <typename T>
void AddElementToTree(tree_node *RootNode, const T &Element)
{
    // TODO configuration for default add location
    // Will try to add it to the most balanced location.
    // LO does L->R; could implement a reverse LO search.
    tree_node *Destination = LevelOrderSearch(*RootNode, GetNodeIfLeaf);

    Assert(Destination, "AddElementToTree() Did not find any Leaves in the Tree!")

    InsertElementAtNode(Destination, Element);
}

////////////////////////////////////////////////////////////////////////////////
//  REMOVE ELEMENT FROM TREE
//
template <typename T>
bool RemoveElementFromTree(tree_node *RootNode, const T &Element)
{
    bool Found = false;
    tree_node *Leaf = PostOrderSearch(*RootNode, Element);

    if(Leaf)
    {
        Found = RemoveNodeFromTree(Leaf);
    }

    return Found;
}

////////////////////////////////////////////////////////////////////////////////
//  SWAP ELEMENTS IN TREE
//  TODO -- Calling function needs to process the containers themselves to resize
template <typename T>
void SwapElementsInTree(tree_node *RootNode, const T &ElementOne, const T &ElementTwo)
{
    tree_node *NodeOne = PostOrderSearch(*RootNode, ElementOne);
    tree_node *NodeTwo = PostOrderSearch(*RootNode, ElementTwo);

    if(!NodeOne || !NodeTwo)
        return;

    NodeOne->Element = ElementTwo;
    NodeTwo->Element = ElementOne;
}

////////////////////////////////////////////////////////////////////////////////
//  MUTATE TREE
//
void RebalanceTree(tree_node *RootNode)
{
    // TODO
    if(!RootNode || IsLeafNode(*RootNode))
        return;
}

void RotateTree(tree_node *RootNode, const rotation_target &Rotation)
{
    // TODO
    if(!RootNode || IsLeafNode(*RootNode))
        return;

    // Rotation can be LEFT or RIGHT
    // note -- a 180 degree rotation is just two left/right rotations
}

void FlipTree(tree_node *RootNode, const rotation_target &Rotation)
{
    // TODO
    if(!RootNode || IsLeafNode(*RootNode))
        return;

    // Rotation can be VERTICAL or HORIZONTAL
}

// Mutate every Node in the Tree.
void PreOrderMutation(tree_node *RootNode, void (*f)(tree_node *Node))
{
    if(!RootNode)
        return;

    f(RootNode);
    PreOrderMutation(RootNode->LeftChild, f);
    PreOrderMutation(RootNode->RightChild, f);
}

void PostOrderMutation(tree_node *RootNode, void (*f)(tree_node *Node))
{
    if(!RootNode)
        return;

    PostOrderMutation(RootNode->LeftChild, f);
    PostOrderMutation(RootNode->RightChild, f);
    f(RootNode);
}

void InOrderMutation(tree_node *RootNode, void (*f)(tree_node *Node))
{
    if(!RootNode)
        return;

    InOrderMutation(RootNode->LeftChild, f);
    f(RootNode);
    InOrderMutation(RootNode->RightChild, f);
}

void LevelOrderMutation(tree_node *RootNode, void (*f)(tree_node *Node))
{
    if(!RootNode)
        return;

    std::queue<tree_node*> NodeQueue;
    for(NodeQueue.push(RootNode); !NodeQueue.empty(); NodeQueue.pop())
    {
        tree_node *Node = NodeQueue.front();

        f(Node);
        
        if(Node->LeftChild)
            NodeQueue.push(Node->LeftChild);

        if(Node->RightChild)
            NodeQueue.push(Node->RightChild);
    }
}
