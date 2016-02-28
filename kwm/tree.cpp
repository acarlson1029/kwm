#include "tree.h"
#include "helpers.h"
#include "display.h"
#include "space.h"
#include "window.h"
#include "border.h"

extern kwm_path KWMPath;
extern kwm_screen KWMScreen;
extern kwm_tiling KWMTiling;

node_container LeftVerticalContainerSplit(screen_info *Screen, tree_node *Node)
{
    Assert(Node, "LeftVerticalContainerSplit()")
    space_info *Space = GetActiveSpaceOfScreen(Screen);
    node_container LeftContainer;

    LeftContainer.X = Node->Container.X;
    LeftContainer.Y = Node->Container.Y;
    LeftContainer.Width = (Node->Container.Width * Node->Container.SplitRatio) - (Space->Offset.VerticalGap / 2);
    LeftContainer.Height = Node->Container.Height;

    return LeftContainer;
}

node_container RightVerticalContainerSplit(screen_info *Screen, tree_node *Node)
{
    Assert(Node, "RightVerticalContainerSplit()")
    space_info *Space = GetActiveSpaceOfScreen(Screen);
    node_container RightContainer;

    RightContainer.X = Node->Container.X + (Node->Container.Width * Node->Container.SplitRatio) + (Space->Offset.VerticalGap / 2);
    RightContainer.Y = Node->Container.Y;
    RightContainer.Width = (Node->Container.Width * (1 - Node->Container.SplitRatio)) - (Space->Offset.VerticalGap / 2);
    RightContainer.Height = Node->Container.Height;

    return RightContainer;
}

node_container UpperHorizontalContainerSplit(screen_info *Screen, tree_node *Node)
{
    Assert(Node, "UpperHorizontalContainerSplit()")
    space_info *Space = GetActiveSpaceOfScreen(Screen);
    node_container UpperContainer;

    UpperContainer.X = Node->Container.X;
    UpperContainer.Y = Node->Container.Y;
    UpperContainer.Width = Node->Container.Width;
    UpperContainer.Height = (Node->Container.Height * Node->Container.SplitRatio) - (Space->Offset.HorizontalGap / 2);

    return UpperContainer;
}

node_container LowerHorizontalContainerSplit(screen_info *Screen, tree_node *Node)
{
    Assert(Node, "LowerHorizontalContainerSplit()")
    space_info *Space = GetActiveSpaceOfScreen(Screen);
    node_container LowerContainer;

    LowerContainer.X = Node->Container.X;
    LowerContainer.Y = Node->Container.Y + (Node->Container.Height * Node->Container.SplitRatio) + (Space->Offset.HorizontalGap / 2);
    LowerContainer.Width = Node->Container.Width;
    LowerContainer.Height = (Node->Container.Height * (1 - Node->Container.SplitRatio)) - (Space->Offset.HorizontalGap / 2);

    return LowerContainer;
}

void CreateNodeContainer(screen_info *Screen, tree_node *Node, const container_type &ContainerType)
{
    Assert(Node, "CreateNodeContainer()")

    if(Node->Container.SplitRatio == 0)
        Node->Container.SplitRatio = KWMScreen.SplitRatio;

    switch(ContainerType)
    {
        case ContainerLeft:
        {
            Node->Container = LeftVerticalContainerSplit(Screen, Node->Parent);
        } break;
        case ContainerRight:
        {
            Node->Container = RightVerticalContainerSplit(Screen, Node->Parent);
        } break;
        case ContainerUpper:
        {
            Node->Container = UpperHorizontalContainerSplit(Screen, Node->Parent);
        } break;
        case ContainerLower:
        {
            Node->Container = LowerHorizontalContainerSplit(Screen, Node->Parent);
        } break;
        case ContainerRoot:
        default:
        {
            DEBUG("CreateNodeContainer() Trying to create node_container for invalid container_type: " << ContainerType)
        } break;
    }

    Node->Container.SplitMode = GetOptimalSplitMode(Node);
    Node->Container.Type = ContainerType;
}

void CreateNodeContainerPair(screen_info *Screen, tree_node *LeftNode, tree_node *RightNode, const split_mode &SplitMode)
{
    Assert(LeftNode, "CreateNodeContainerPair() Left Node")
    Assert(RightNode, "CreateNodeContainerPair() Right Node")

    switch(SplitMode)
    {
        case SplitModeVertical:
        {
            CreateNodeContainer(Screen, LeftNode, ContainerLeft);
            CreateNodeContainer(Screen, RightNode, ContainerRight);
        } break;
        case SplitModeHorizontal:
        {
            CreateNodeContainer(Screen, LeftNode, ContainerUpper);
            CreateNodeContainer(Screen, RightNode, ContainerLower);
        } break;
        default:
        {
            DEBUG("CreateNodeContainerPair() Invalid SplitMode given: " << SplitMode)
        } break;

    }
}

tree_node *CreateLeafNode(screen_info *Screen, tree_node *Parent, int WindowID, const container_type &ContainerType)
{
    Assert(Parent, "CreateLeafNode()")

    tree_node Clear = {0};
    tree_node *Leaf = (tree_node*) malloc(sizeof(tree_node));
    *Leaf = Clear;

    Leaf->Parent = Parent;
    Leaf->WindowID = WindowID;

    CreateNodeContainer(Screen, Leaf, ContainerType);

    Leaf->LeftChild = NULL;
    Leaf->RightChild = NULL;

    return Leaf;
}

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

void SetRootNodeContainer(screen_info *Screen, tree_node *Node)
{
    Assert(Node, "SetRootNodeContainer()")

    space_info *Space = GetActiveSpaceOfScreen(Screen);

    Node->Container.X = Screen->X + Space->Offset.PaddingLeft;
    Node->Container.Y = Screen->Y + Space->Offset.PaddingTop;
    Node->Container.Width = Screen->Width - Space->Offset.PaddingLeft - Space->Offset.PaddingRight;
    Node->Container.Height = Screen->Height - Space->Offset.PaddingTop - Space->Offset.PaddingBottom;
    Node->Container.SplitMode = GetOptimalSplitMode(Node);

    Node->Container.Type = ContainerRoot;
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

tree_node *GetFirstLeafNode(tree_node *Node)
{
    if(Node)
    {
        while(Node->LeftChild)
            Node = Node->LeftChild;

        return Node;
    }

    return NULL;
}

tree_node *GetLastLeafNode(tree_node *Node)
{
    if(Node)
    {
        while(Node->RightChild)
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

tree_node *CreateTreeFromWindowIDList(screen_info *Screen, std::vector<window_info*> *WindowsPtr)
{
    if(IsSpaceFloating(Screen->ActiveSpace))
        return NULL;

    tree_node *RootNode = CreateRootNode();
    SetRootNodeContainer(Screen, RootNode);

    bool Result = false;
    space_info *Space = GetActiveSpaceOfScreen(Screen);

    if(Space->Mode == SpaceModeBSP)
        Result = CreateBSPTree(RootNode, Screen, WindowsPtr);
    else if(Space->Mode == SpaceModeMonocle)
        Result = CreateMonocleTree(RootNode, Screen, WindowsPtr);

    if(!Result)
    {
        free(RootNode);
        RootNode = NULL;
    }

    return RootNode;
}

bool CreateBSPTree(tree_node *RootNode, screen_info *Screen, std::vector<window_info*> *WindowsPtr)
{
    Assert(RootNode, "CreateBSPTree()")

    bool Result = false;
    std::vector<window_info*> &Windows = *WindowsPtr;

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
            CreateLeafNodePair(Screen, Root, Root->WindowID, Windows[WindowIndex]->WID, GetOptimalSplitMode(Root));
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

bool CreateMonocleTree(tree_node *RootNode, screen_info *Screen, std::vector<window_info*> *WindowsPtr)
{
    Assert(RootNode, "CreateMonocleTree()")

    bool Result = false;
    std::vector<window_info*> &Windows = *WindowsPtr;

    if(!Windows.empty())
    {
        tree_node *Root = RootNode;
        Root->WindowID = Windows[0]->WID;

        for(std::size_t WindowIndex = 1; WindowIndex < Windows.size(); ++WindowIndex)
        {
            tree_node *Next = CreateRootNode();
            SetRootNodeContainer(Screen, Next);
            Next->WindowID = Windows[WindowIndex]->WID;

            Root->RightChild = Next;
            Next->LeftChild = Root;
            Root = Next;
        }

        Result = true;
    }

    return Result;
}

split_mode GetOptimalSplitMode(tree_node *Node)
{
    return (Node->Container.Width / Node->Container.Height) >= 1.618 ? SplitModeVertical : SplitModeHorizontal;
}

void ChangeSplitRatio(double Value)
{
    if(Value > 0.0 && Value < 1.0)
    {
        DEBUG("ChangeSplitRatio() New Split-Ratio is " << Value)
        KWMScreen.SplitRatio = Value;
    }
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

void ResizeNodeContainer(screen_info *Screen, tree_node *Node)
{
    if(Node)
    {
        if(Node->LeftChild)
        {
            CreateNodeContainer(Screen, Node->LeftChild, Node->LeftChild->Container.Type);
            ResizeNodeContainer(Screen, Node->LeftChild);
        }

        if(Node->RightChild)
        {
            CreateNodeContainer(Screen, Node->RightChild, Node->RightChild->Container.Type);
            ResizeNodeContainer(Screen, Node->RightChild);
        }
    }
}

tree_node *GetNearestNodeToTheLeft(tree_node *Node, space_tiling_option Mode)
{
    if(Node)
    {
        if(Mode == SpaceModeMonocle)
            return Node->LeftChild;

        if(Mode == SpaceModeBSP)
        {
            if(IsLeftChild(Node))
                return GetNearestNodeToTheLeft(Node->Parent, Mode);
            
            tree_node *Left = Node->Parent->LeftChild;
            if(IsLeafNode(Left))
                return Left;

            // TODO -- break into a WalkRight traversal function?
            while(!IsLeafNode(Left))
                Left = Left->RightChild;

            return Left;
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

        if(Mode == SpaceModeBSP)
        {
            if(Node->Parent)
            {
                if(IsRightChild(Node))
                    return GetNearestNodeToTheRight(Node->Parent, Mode);

                tree_node *Right = Node->Parent->RightChild;
                if(IsLeafNode(Right))
                    return Right;

                // TODO -- break into a WalkLeft traversal function?
                while(!IsLeafNode(Right))
                    Right = Right->LeftChild;

                return Right;
            }
        }
    }

    return NULL;
}

void CreateNodeContainers(screen_info *Screen, tree_node *Node, bool OptimalSplit)
{
    if(Node && Node->LeftChild && Node->RightChild)
    {
        Node->Container.SplitMode = OptimalSplit ? GetOptimalSplitMode(Node) : Node->Container.SplitMode;
        CreateNodeContainerPair(Screen, Node->LeftChild, Node->RightChild, Node->Container.SplitMode);

        CreateNodeContainers(Screen, Node->LeftChild, OptimalSplit);
        CreateNodeContainers(Screen, Node->RightChild, OptimalSplit);
    }
}

void ToggleNodeSplitMode(screen_info *Screen, tree_node *Node)
{
    if(!Node || IsLeafNode(Node))
        return;

    Node->Container.SplitMode = Node->Container.SplitMode == SplitModeVertical ? SplitModeHorizontal : SplitModeVertical;
    CreateNodeContainers(Screen, Node, false);
    ApplyNodeContainer(Node, SpaceModeBSP);
}

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

void CreateDeserializedNodeContainer(tree_node *Node)
{
    split_mode SplitMode = Node->Parent->Container.SplitMode;
    container_type ContainerType;

    switch(SplitMode)
    {
        case SplitModeHorizontal:
        {
            ContainerType = IsLeftLeaf(Node) ? ContainerLeft : ContainerRight;
        } break;
        case SplitModeVertical:
        {
            ContainerType = IsLeftLeaf(Node) ? ContainerUpper : ContainerLower;
        } break;
        default:
        {
            DEBUG("CreateDeserializedNodeContainer() Invalid SplitMode given: " << SplitMode)
        } break;
    }

    CreateNodeContainer(KWMScreen.Current, Node, ContainerType);
}

void FillDeserializedTree(tree_node *RootNode)
{
    std::vector<window_info*> Windows = GetAllWindowsOnDisplay(KWMScreen.Current->ID);
    tree_node *Current = GetFirstLeafNode(RootNode);

    std::size_t Counter = 0, Leafs = 0;
    while(Current)
    {
        if(Counter < Windows.size())
            Current->WindowID = Windows[Counter++]->WID;

        Current = GetNearestNodeToTheRight(Current, SpaceModeBSP);
        ++Leafs;
    }

    if(Leafs < Windows.size() && Counter < Windows.size())
    {
        tree_node *Root = RootNode;
        for(; Counter < Windows.size(); ++Counter)
        {
            while(!IsLeafNode(Root))
            {
                if(!IsLeafNode(Root->LeftChild) && IsLeafNode(Root->RightChild))
                    Root = Root->RightChild;
                else
                    Root = Root->LeftChild;
            }

            DEBUG("FillDeserializedTree() Create pair of leafs")
            CreateLeafNodePair(KWMScreen.Current, Root, Root->WindowID, Windows[Counter]->WID, GetOptimalSplitMode(Root));
            Root = RootNode;
        }
    }
}

void SerializeParentNode(tree_node *Parent, std::string Role, std::vector<std::string> &Serialized)
{
    Serialized.push_back("kwmc tree root create " + Role);
    Serialized.push_back("kwmc tree split-mode " + std::to_string(Parent->Container.SplitMode));
    Serialized.push_back("kwmc tree split-ratio " + std::to_string(Parent->Container.SplitRatio));

    if(IsLeafNode(Parent->LeftChild))
    {
        Serialized.push_back("kwmc tree child");
        Serialized.push_back("kwmc tree leaf create left");
    }
    else
    {
        Serialized.push_back("kwmc tree child");
        SerializeParentNode(Parent->LeftChild, "left", Serialized);
    }

    if(IsLeafNode(Parent->RightChild))
    {
        Serialized.push_back("kwmc tree child");
        Serialized.push_back("kwmc tree leaf create right");
    }
    else
    {
        Serialized.push_back("kwmc tree child");
        SerializeParentNode(Parent->RightChild, "right", Serialized);
    }
}

unsigned int DeserializeParentNode(tree_node *Parent, std::vector<std::string> &Serialized, unsigned int Index)
{
    unsigned int LineNumber = Index;
    for(;LineNumber < Serialized.size(); ++LineNumber)
    {
        std::string Line = Serialized[LineNumber];
        std::vector<std::string> Tokens = SplitString(Line, ' ');

        if(Tokens[2] == "split-mode")
        {
            Parent->Container.SplitMode = static_cast<split_mode>(ConvertStringToInt(Tokens[3]));
            DEBUG("Root: SplitMode Found " + Tokens[3])
        }
        else if(Tokens[2] == "split-ratio")
        {
            Parent->Container.SplitRatio = ConvertStringToDouble(Tokens[3]);
            DEBUG("Root: SplitRatio Found " + Tokens[3])
        }
        else if(Tokens[2] == "child")
        {
            DEBUG("Root: Child Found")
            DEBUG("Parent: " << Parent->Container.SplitMode << "|" << Parent->Container.SplitRatio)
            LineNumber = DeserializeChildNode(Parent, Serialized, LineNumber+1);
        }

        if(Parent->RightChild)
            return LineNumber;
    }

    return LineNumber;
}

unsigned int DeserializeChildNode(tree_node *Parent, std::vector<std::string> &Serialized, unsigned int Index)
{
    unsigned int LineNumber = Index;
    for(;LineNumber < Serialized.size(); ++LineNumber)
    {
        std::string Line = Serialized[LineNumber];
        if(Line == "kwmc tree root create left")
        {
            DEBUG("Child: Create root")
            Parent->LeftChild = CreateLeafNode(KWMScreen.Current, Parent, -1, ContainerLeft);
            CreateDeserializedNodeContainer(Parent->LeftChild);
            LineNumber = DeserializeParentNode(Parent->LeftChild, Serialized, LineNumber+1);
            return LineNumber;
        }
        else if(Line == "kwmc tree root create right")
        {
            DEBUG("Child: Create root")
            Parent->RightChild = CreateLeafNode(KWMScreen.Current, Parent, -1, ContainerRight);
            CreateDeserializedNodeContainer(Parent->RightChild);
            LineNumber = DeserializeParentNode(Parent->RightChild, Serialized, LineNumber+1);
            return LineNumber;
        }
        else if(Line == "kwmc tree leaf create left")
        {
            DEBUG("Child: Create left leaf")
            Parent->LeftChild = CreateLeafNode(KWMScreen.Current, Parent, -1, ContainerLeft);
            CreateDeserializedNodeContainer(Parent->LeftChild);
            return LineNumber;
        }
        else if(Line == "kwmc tree leaf create right")
        {
            DEBUG("Child: Create right leaf")
            Parent->RightChild = CreateLeafNode(KWMScreen.Current, Parent, -1, ContainerRight);
            CreateDeserializedNodeContainer(Parent->RightChild);
            return LineNumber;
        }
    }

    return LineNumber;
}

tree_node *DeserializeNodeTree(std::vector<std::string> &Serialized)
{
    if(Serialized.empty() || Serialized[0] != "kwmc tree root create parent")
        return NULL;

    DEBUG("Deserialize: Create Master")
    tree_node *RootNode = CreateRootNode();
    SetRootNodeContainer(KWMScreen.Current, RootNode);
    DeserializeParentNode(RootNode, Serialized, 1);
    return RootNode;
}

void SaveBSPTreeToFile(screen_info *Screen, std::string Name)
{
    if(IsSpaceInitializedForScreen(Screen))
    {
        space_info *Space = GetActiveSpaceOfScreen(Screen);
        if(Space->Mode != SpaceModeBSP || IsLeafNode(Space->RootNode))
            return;

        struct stat Buffer;
        std::string TempPath = KWMPath.EnvHome + "/" + KWMPath.ConfigFolder + "/" + KWMPath.BSPLayouts;
        if (stat(TempPath.c_str(), &Buffer) == -1)
            mkdir(TempPath.c_str(), 0700);

        std::ofstream OutFD(TempPath + "/" + Name);
        if(OutFD.fail())
            return;

        tree_node *Root = Space->RootNode;
        std::vector<std::string> SerializedTree;
        SerializeParentNode(Root, "parent", SerializedTree);

        for(std::size_t LineNumber = 0; LineNumber < SerializedTree.size(); ++LineNumber)
            OutFD << SerializedTree[LineNumber] << std::endl;

        OutFD.close();
    }
}

void LoadBSPTreeFromFile(screen_info *Screen, std::string Name)
{
    if(IsSpaceInitializedForScreen(Screen))
    {
        space_info *Space = GetActiveSpaceOfScreen(Screen);
        if(Space->Mode != SpaceModeBSP)
            return;

        std::string TempPath = KWMPath.EnvHome + "/" + KWMPath.ConfigFolder + "/" + KWMPath.BSPLayouts;
        std::ifstream InFD(TempPath + "/" + Name);
        if(InFD.fail())
            return;

        std::string Line;
        std::vector<std::string> SerializedTree;
        while(std::getline(InFD, Line))
            SerializedTree.push_back(Line);

        DestroyNodeTree(Space->RootNode, SpaceModeBSP);
        Space->RootNode = DeserializeNodeTree(SerializedTree);
        FillDeserializedTree(Space->RootNode);
        ApplyNodeContainer(Space->RootNode, SpaceModeBSP);
        UpdateBorder("focused");
        UpdateBorder("marked");
    }
}
