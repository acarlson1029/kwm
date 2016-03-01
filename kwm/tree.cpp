#include "tree.h"
#include "node.h"
#include "space.h" // for GetActiveSpaceOfScreen()
#include "window.h" // remove ResizeWindowToContainerSize

extern kwm_path KWMPath;
extern kwm_screen KWMScreen;
extern kwm_tiling KWMTiling;

tree_node *CreateTreeFromWindowIDList(screen_info *Screen, const std::vector<window_info*> &Windows)
{
    if(IsSpaceFloating(Screen->ActiveSpace))
        return NULL;

    tree_node *RootNode = CreateRootNode(Screen);

    bool Result = false;
    space_info *Space = GetActiveSpaceOfScreen(Screen);

    if(Space->Mode == SpaceModeBSP)
        Result = CreateBSPTree(RootNode, Screen, Windows);
    else if(Space->Mode == SpaceModeMonocle)
        Result = CreateMonocleTree(RootNode, Screen, Windows);

    if(!Result)
    {
        free(RootNode);
        RootNode = NULL;
    }

    return RootNode;
}

bool CreateBSPTree(tree_node *RootNode, screen_info *Screen, const std::vector<window_info*> &Windows)
{
    Assert(RootNode, "CreateBSPTree()")

    bool Result = false;

    if(Windows.size() >= 2)
    {
        tree_node *Root = RootNode;
        Root->WindowID = Windows[0]->WID;
        for(std::size_t WindowIndex = 1; WindowIndex < Windows.size(); ++WindowIndex)
        {
            while(!IsLeafNode(Root))
            {
                if(!IsLeafNode(Root->LeftChild) && IsLeafNode(Root->RightChild))
                    Root = Root->RightChild;
                else
                    Root = Root->LeftChild;
            }

            DEBUG("CreateBSPTree() Create pair of leafs")
            CreateLeafNodePair(Screen, Root, Root->WindowID, Windows[WindowIndex]->WID, SplitModeOptimal);
            Root = RootNode;
        }

        Result = true;
    }
    else if(Windows.size() == 1)
    {
        RootNode->WindowID = Windows[0]->WID;
        Result = true;
    }

    return Result;
}

bool CreateMonocleTree(tree_node *RootNode, screen_info *Screen, const std::vector<window_info*> &Windows)
{
    Assert(RootNode, "CreateMonocleTree()")

    bool Result = false;

    if(!Windows.empty())
    {
        tree_node *Root = RootNode;
        Root->WindowID = Windows[0]->WID;

        for(std::size_t WindowIndex = 1; WindowIndex < Windows.size(); ++WindowIndex)
        {
            tree_node *Next = CreateRootNode(Screen);
            Next->WindowID = Windows[WindowIndex]->WID;

            Root->RightChild = Next;
            Next->LeftChild = Root;
            Root = Next;
        }

        Result = true;
    }

    return Result;
}

// TODO Add traversal function
// foreach node in tree, if windowID != -1, ResizeWindowToContainerSize(Node)
void ApplyNodeContainer(tree_node *Node, space_tiling_option Mode)
{
    if(Node)
    {
        if(Node->WindowID != -1)
            ResizeWindowToContainerSize(Node);

        if(Mode == SpaceModeBSP && Node->LeftChild)
            ApplyNodeContainer(Node->LeftChild, Mode);

        if(Node->RightChild)
            ApplyNodeContainer(Node->RightChild, Mode);
    }
}

void ResizeTreeNodes(screen_info *Screen, tree_node *Root)
{
    PreOrderTraversal(ResizeNodeContainer, Screen, Root);
}

void DestroyNodeTree(tree_node *Node, space_tiling_option Mode)
{
    if(Node)
    {
        if(Mode == SpaceModeBSP && Node->LeftChild)
            DestroyNodeTree(Node->LeftChild, Mode);

        if(Node->RightChild)
            DestroyNodeTree(Node->RightChild, Mode);

        free(Node);
        Node = NULL;
    }
}

void RotateTree(tree_node *Node, int Deg)
{
    if (Node == NULL || IsLeafNode(Node))
        return;

    DEBUG("RotateTree() " << Deg << " degrees")

    if((Deg == 90 && Node->Container.SplitMode == SplitModeVertical) ||
       (Deg == 270 && Node->Container.SplitMode == SplitModeHorizontal) ||
       Deg == 180)
    {
        tree_node *Temp = Node->LeftChild;
        Node->LeftChild = Node->RightChild;
        Node->RightChild = Temp;
        Node->Container.SplitRatio = 1 - Node->Container.SplitRatio;
    }

    if(Deg != 180)
        Node->Container.SplitMode = Node->Container.SplitMode == SplitModeHorizontal ? SplitModeVertical : SplitModeHorizontal;

    RotateTree(Node->LeftChild, Deg);
    RotateTree(Node->RightChild, Deg);


}

void ToggleSubtreeSplitMode(screen_info *Screen, tree_node *Node)
{
    if(!Node || IsLeafNode(Node))
        return;

    ToggleNodeSplitMode(Node);
    ResizeTreeNodes(Screen, Node);
    ApplyNodeContainer(Node, SpaceModeBSP);
}

tree_node *GetNodeFromWindowID(tree_node *Node, int WindowID, space_tiling_option Mode)
{
    if(Node)
    {
        tree_node *CurrentNode = GetFirstLeafNode(Node);;
        while(CurrentNode)
        {
            if(CurrentNode->WindowID == WindowID)
                return CurrentNode;

            CurrentNode = GetNearestNodeToTheRight(CurrentNode, Mode);
        }
    }

    return NULL;
}

tree_node *GetFirstLeafNode(tree_node *Node)
{
    if(Node)
    {
        while(!IsLeafNode(Node))
            Node = Node->LeftChild;

        return Node;
    }

    return NULL;
}

tree_node *GetLastLeafNode(tree_node *Node)
{
    if(Node)
    {
        while(!IsLeafNode(Node))
            Node = Node->RightChild;

        return Node;
    }

    return NULL;
}

tree_node *GetFirstPseudoLeafNode(tree_node *Node)
{
    tree_node *Leaf = GetFirstLeafNode(Node);
    while(Leaf && Leaf->WindowID != -1)
        Leaf = GetNearestNodeToTheRight(Leaf, SpaceModeBSP);

    return Leaf;
}

tree_node *GetNearestLeafNeighbour(tree_node *Node, space_tiling_option Mode)
{
    if(Node && IsLeafNode(Node))
    {
        if(Mode == SpaceModeBSP)
            return IsLeftLeaf(Node) ? GetNearestNodeToTheRight(Node, Mode) : GetNearestNodeToTheLeft(Node, Mode);
        else if(Mode == SpaceModeMonocle)
            return Node->LeftChild ? Node->LeftChild : Node->RightChild;
    }

    return NULL;
}

tree_node *GetNearestNodeToTheLeft(tree_node *Node, space_tiling_option Mode)
{
    if(Node)
    {
        if(Mode == SpaceModeMonocle)
            return Node->LeftChild;

        if((Mode == SpaceModeBSP) && Node->Parent)
        {
            if(IsLeftChild(Node))
                return GetNearestNodeToTheLeft(Node->Parent, Mode);
            
            tree_node *Left = Node->Parent->LeftChild;
            if(IsLeafNode(Left))
                return Left;

            return GetLastLeafNode(Left);
        }
    }

    return NULL;
}

tree_node *GetNearestNodeToTheRight(tree_node *Node, space_tiling_option Mode)
{
    if(Node)
    {
        if(Mode == SpaceModeMonocle)
            return Node->RightChild;

        if((Mode == SpaceModeBSP) && Node->Parent)
        {
            if(IsRightChild(Node))
                return GetNearestNodeToTheRight(Node->Parent, Mode);

            tree_node *Right = Node->Parent->RightChild;
            if(IsLeafNode(Right))
                return Right;

            return GetFirstLeafNode(Right);
        }
    }

    return NULL;
}

void PreOrderTraversal(void (*f)(screen_info *Screen, tree_node *Root), screen_info *Screen, tree_node *Root)
{
    if(Root)
    {
        f(Screen, Root);
        PreOrderTraversal(f, Screen, Root->LeftChild);
        PreOrderTraversal(f, Screen, Root->RightChild);
    }
}

tree_node *LevelOrderSearch(bool (*is_match)(tree_node *), tree_node *Root)
{
    std::queue<tree_node*> qNode;
    
    if(Root)
    {
        for(qNode.push(Root); !qNode.empty(); qNode.pop())
        {
            tree_node *Node = qNode.front();

            if(is_match(Node))
                return Node;
            
            if(Node->LeftChild)
                qNode.push(Node->LeftChild);

            if(Node->RightChild)
                qNode.push(Node->RightChild);
        }
    }

    return NULL;
}

void AddElementToBSPTree(screen_info *Screen, tree_node *NewParent, int WindowID, const split_mode &SplitMode) // TODO replace WindowID with element
{
    if(IsLeafNode(NewParent))
    {
        Assert((NewParent->WindowID !=-1), "AddNodeToTree()")

        CreateLeafNodePair(Screen, NewParent, NewParent->WindowID, WindowID, SplitMode);
    }
}

void AddElementToMonocleTree(screen_info *Screen, tree_node *NewParent, int WindowID) // TODO replace WindowID with element
{
    Assert(!NewParent->RightChild, "AddElementToMonocleTree()")

    tree_node *NewNode = CreateRootNode(Screen);

    NewNode->WindowID = WindowID;
    NewNode->LeftChild = NewParent;
    NewParent->RightChild = NewNode;
}

void AddElementToTree(screen_info *Screen, tree_node *NewParent, int WindowID, const split_mode &SplitMode, const space_tiling_option &Mode)
{
    switch(Mode)
    {
        case SpaceModeBSP:
            AddElementToBSPTree(Screen, NewParent, WindowID, SplitMode);
            break;
        case SpaceModeMonocle:
            AddElementToMonocleTree(Screen, NewParent, WindowID);
            break;

        default:
            Assert(false, "AddElementToTree()")
            return;
    }
    ApplyNodeContainer(NewParent, Mode);
}

void RemoveElementFromBSPTree(screen_info *Screen, tree_node *Node)
{
    if(Node)
    {
        space_info *Space = GetActiveSpaceOfScreen(Screen);
        tree_node *Parent = Node->Parent;
        tree_node *ResizeRoot = NULL;

        // case 1: Node == Root
        if(!Parent)
        {
            Space->RootNode = NULL;
            free(Node);
            return;
        }

        tree_node *Sibling = IsLeftChild(Node) ? Parent->RightChild : Parent->LeftChild;
        // case 2: Node->Parent == Root
        if (Parent == Space->RootNode)
        {
            Space->RootNode = Sibling;
            ResizeRoot = Space->RootNode;
        }
        else
        {
            tree_node *Grandparent = Parent->Parent;
            if(IsLeftChild(Parent))
                Grandparent->LeftChild = Sibling;
            else
                Grandparent->RightChild = Sibling;

            ResizeRoot = Grandparent;

        }

        free(Node);
        free(Parent);

        ResizeTreeNodes(Screen, ResizeRoot);
        ApplyNodeContainer(ResizeRoot, Space->Mode);
    }
}

void RemoveElementFromMonocleTree(screen_info *Screen, tree_node *Node)
{
    if(Node)
    {
        space_info *Space = GetActiveSpaceOfScreen(Screen);

        tree_node *Prev = Node->LeftChild;
        tree_node *Next = Node->RightChild;

        if(Prev)
            Prev->RightChild = Next;

        if(Next)
            Next->LeftChild = Prev;

        if(!Prev) // root node of monocle tree
            Space->RootNode = Next;

        free(Node);
    }
}

void RemoveElementFromTree(screen_info *Screen, tree_node *Root, int WindowID, const space_tiling_option &Mode)
{
    if(Root)
    {
        tree_node *Node = GetNodeFromWindowID(Root, WindowID, Mode);
        
        switch(Mode)
        {
            case SpaceModeBSP:
                RemoveElementFromBSPTree(Screen, Node);
                break;
            case SpaceModeMonocle:
                RemoveElementFromMonocleTree(Screen, Node);
                break;

            default:
                Assert (false, "RemoveElementFromTree()")
                return;
        }
    }
}

void ModifySubtreeSplitRatio(screen_info *Screen, tree_node *Root, const double &Offset)
{
    if(Root && ModifyNodeSplitRatio(Root->Parent, Offset))
    {
        space_info *Space = GetActiveSpaceOfScreen(Screen);

        ResizeTreeNodes(Screen, Root->Parent);
        ApplyNodeContainer(Root->Parent, Space->Mode);
    }
}

bool ToggleElementInTree(screen_info *Screen, tree_node *Root, const int &WindowID, const space_tiling_option &Mode)
{
    if(Mode != SpaceModeBSP)
        return false;

    tree_node *Node = GetNodeFromWindowID(Root, WindowID, SpaceModeBSP);

    if(Node && Node->Parent)
    {
        if(IsLeafNode(Node) && Node->Parent->WindowID == -1) // TODO Add a function to check if node has elements or is just a parent.
        {
            DEBUG("ToggleElementInTree() Set Element In Node")
            SetElementInNode(Screen, Node, WindowID);
        }
        else
        {
            DEBUG("ToggleElementInTree() Restore Element In Node")
            ClearElementInNode(Screen, Node);
        }
        return true;
    }

    return false;
}

bool ToggleElementInRoot(screen_info *Screen, tree_node *Root, const int &WindowID, const space_tiling_option &Mode)
{
    if(Mode != SpaceModeBSP || IsLeafNode(Root))
        return false;

    if(Root->WindowID == -1)
    {
        // TODO Should calling functions be trying to toggle invalid WindowIDs?
        // This wastes cycles trying to find it.
        tree_node *Node = GetNodeFromWindowID(Root, WindowID, Mode);
        if(Node) // if it was found
        {
            DEBUG("ToggleElementInRoot() Set root element")
            SetElementInNode(Screen, Root, WindowID);
        }
    }
    else
    {
        DEBUG("ToggleElementInRoot() Clear root element")
        ClearElementInNode(Screen, Root);
        tree_node *Node = GetNodeFromWindowID(Root, WindowID, Mode);
        SetElementInNode(Screen, Node, WindowID); // resize the window back to fit its container
    }
    return true;
}
