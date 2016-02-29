/* #include "tree.h" */
#include "container.h"
#include "window.h"

extern kwm_screen KWMScreen;
extern kwm_tiling KWMTiling;

tree_node *CreateRootNode()
{
    tree_node Clear = {0};
    tree_node *RootNode = (tree_node*) malloc(sizeof(tree_node));
    *RootNode = Clear;

    RootNode->WindowID = -1;
    RootNode->Parent = NULL;
    RootNode->LeftChild = NULL;
    RootNode->RightChild = NULL;
    RootNode->Container.SplitRatio = KWMScreen.SplitRatio;
    RootNode->Container.SplitMode = SplitModeUnset;

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
    Parent->Container.SplitMode = SplitMode;
    Parent->Container.SplitRatio = KWMScreen.SplitRatio;

    int LeftWindowID = KWMTiling.SpawnAsLeftChild ? SecondWindowID : FirstWindowID;
    int RightWindowID = KWMTiling.SpawnAsLeftChild ? FirstWindowID : SecondWindowID;

    switch(SplitMode)
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
