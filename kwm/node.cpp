#include "node.h"
#include "container.h"
#include "windowref.h" // TODO remove ResizeWindowToContainerSize and get rid of this include.

extern kwm_screen KWMScreen;
extern kwm_tiling KWMTiling;

tree_node *CreateRootNode(screen_info *Screen)
{
    tree_node Clear = {0};
    tree_node *RootNode = (tree_node*) malloc(sizeof(tree_node));
    *RootNode = Clear;

    RootNode->WindowID = -1;
    RootNode->Parent = NULL;
    RootNode->LeftChild = NULL;
    RootNode->RightChild = NULL;
    SetRootNodeContainer(Screen, &RootNode->Container);

    return RootNode;
}

tree_node *CreateLeafNode(screen_info *Screen, tree_node *Parent, int WindowID, const container_type &ContainerType)
{
    Assert(Parent, "CreateLeafNode()")

    tree_node Clear = {0};
    tree_node *Leaf = (tree_node*) malloc(sizeof(tree_node));
    *Leaf = Clear;

    Leaf->Parent = Parent;
    Leaf->WindowID = WindowID;

    Leaf->Container = CreateNodeContainer(Screen, Parent->Container, ContainerType);

    Leaf->LeftChild = NULL;
    Leaf->RightChild = NULL;

    return Leaf;
}

void CreateLeafNodePair(screen_info *Screen, tree_node *Parent, int FirstWindowID, int SecondWindowID, const split_mode &SplitMode)
{
    Assert(Parent, "CreateLeafNodePair()")

    Parent->WindowID = -1;
    Parent->Container.SplitMode = SplitMode == SplitModeOptimal ? GetOptimalSplitMode(Parent->Container) : SplitMode;
    Parent->Container.SplitRatio = KWMScreen.SplitRatio;

    int LeftWindowID = KWMTiling.SpawnAsLeftChild ? SecondWindowID : FirstWindowID;
    int RightWindowID = KWMTiling.SpawnAsLeftChild ? FirstWindowID : SecondWindowID;

    switch(Parent->Container.SplitMode)
    {
        case SplitModeVertical:
        {
            Parent->LeftChild = CreateLeafNode(Screen, Parent, LeftWindowID, ContainerLeft);
            Parent->RightChild = CreateLeafNode(Screen, Parent, RightWindowID, ContainerRight);
        } break;
        case SplitModeHorizontal:
        {
            Parent->LeftChild = CreateLeafNode(Screen, Parent, LeftWindowID, ContainerUpper);
            Parent->RightChild = CreateLeafNode(Screen, Parent, RightWindowID, ContainerLower);
        } break;
        default:
        {
            DEBUG("CreateLeafNodePair() Invalid SplitMode given: " << SplitMode)
            DEBUG("CreateLeafNodePair() Setting Parent to NULL")
            Parent->Parent = NULL;
            Parent->LeftChild = NULL;
            Parent->RightChild = NULL;
            Parent = NULL;
        } break;
    }
}

bool IsLeafNode(tree_node *Node)
{
    return Node->LeftChild == NULL && Node->RightChild == NULL ? true : false;
}

bool IsLeftChild(tree_node *Node)
{
    if (Node && Node->Parent)
    {
        return Node->Parent->LeftChild == Node;
    }

    return false;
}

bool IsLeftLeaf(tree_node *Node)
{
    return IsLeftChild(Node) && IsLeafNode(Node);
}

bool IsRightChild(tree_node *Node)
{
    if (Node && Node->Parent)
    {
        return Node->Parent->RightChild == Node;
    }

    return false;
}

bool IsRightLeaf(tree_node *Node)
{
    return IsRightChild(Node) && IsLeafNode(Node);
}

void SwapNodeWindowIDs(tree_node *A, tree_node *B)
{
    if(A && B)
    {
        DEBUG("SwapNodeWindowIDs() " << A->WindowID << " with " << B->WindowID)
        int TempWindowID = A->WindowID;
        A->WindowID = B->WindowID;
        B->WindowID = TempWindowID;
        ResizeWindowToContainerSize(A);
        ResizeWindowToContainerSize(B);
    }
}

// Note - in Monocle Mode, every Node is a "Root" (i.e. no parent),so
// every node is resized to the RootNodeContainer.
// TODO -- Does this need an "OptimalSplitMode" boolean argument?
void ResizeNodeContainer(screen_info *Screen, tree_node *Node)
{
    Assert(Node, "ResizeNodeContainer()")

    // BSP Root Node or Monocle Node
    if (Node && !Node->Parent)
        SetRootNodeContainer(Screen, &Node->Container);
    else
        ResizeContainer(Screen, &Node->Container);
}

bool ModifyNodeSplitRatio(tree_node *Node, const double &Offset)
{
    if(!Node)
        return false;

    return ModifyContainerSplitRatio(&Node->Container, Offset);
}

void ToggleNodeSplitMode(tree_node *Node)
{
    if(Node)
    {
        ToggleContainerSplitMode(&Node->Container);
    }
}

void SetElementInNode(screen_info *Screen, tree_node *Node, const int &WindowID)
{
    if(Node)
    {
        Node->WindowID = WindowID;
        ResizeNodeContainer(Screen, Node);
    }
}

void ClearElementInNode(screen_info *Screen, tree_node *Node)
{
    if(Node)
    {
        Node->WindowID = -1;
        ResizeNodeContainer(Screen, Node);
    }
}
