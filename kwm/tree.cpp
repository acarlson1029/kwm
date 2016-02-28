#include "tree.h"
#include "helpers.h"
#include "display.h"
#include "space.h"
#include "window.h"
#include "border.h"

extern kwm_path KWMPath;
extern kwm_screen KWMScreen;
extern kwm_tiling KWMTiling;

/* Note on parameters and const:
    const type* const VarName  -> can't change pointer, can't change data.
    type* const VarName        -> can't change pointer, can change data.
                                  `-> needed for dereferencing struct elements.
*/

node_container LeftVerticalContainerSplit(screen_info* const Screen, const tree_node* const Node)
{
    space_info *Space = &Screen->Space[Screen->ActiveSpace];
    node_container LeftContainer;

    LeftContainer.X = Node->Container.X;
    LeftContainer.Y = Node->Container.Y;
    LeftContainer.Width = (Node->Container.Width * Node->SplitRatio) - (Space->Offset.VerticalGap / 2);
    LeftContainer.Height = Node->Container.Height;
    
    return LeftContainer;
}

node_container RightVerticalContainerSplit(screen_info* const Screen, const tree_node* const Node)
{
    space_info *Space = &Screen->Space[Screen->ActiveSpace];
    node_container RightContainer;

    RightContainer.X = Node->Container.X + (Node->Container.Width * Node->SplitRatio) + (Space->Offset.VerticalGap / 2);
    RightContainer.Y = Node->Container.Y;
    RightContainer.Width = (Node->Container.Width * (1 - Node->SplitRatio)) - (Space->Offset.VerticalGap / 2);
    RightContainer.Height = Node->Container.Height;

    return RightContainer;
}

node_container UpperHorizontalContainerSplit(screen_info* const Screen, const tree_node* const Node)
{
    space_info *Space = &Screen->Space[Screen->ActiveSpace];
    node_container UpperContainer;

    UpperContainer.X = Node->Container.X;
    UpperContainer.Y = Node->Container.Y;
    UpperContainer.Width = Node->Container.Width;
    UpperContainer.Height = (Node->Container.Height * Node->SplitRatio) - (Space->Offset.HorizontalGap / 2);

    return UpperContainer;
}

node_container LowerHorizontalContainerSplit(screen_info* const Screen, const tree_node* const Node)
{
    space_info *Space = &Screen->Space[Screen->ActiveSpace];
    node_container LowerContainer;

    LowerContainer.X = Node->Container.X;
    LowerContainer.Y = Node->Container.Y + (Node->Container.Height * Node->SplitRatio) + (Space->Offset.HorizontalGap / 2);
    LowerContainer.Width = Node->Container.Width;
    LowerContainer.Height = (Node->Container.Height * (1 - Node->SplitRatio)) - (Space->Offset.HorizontalGap / 2);

    return LowerContainer;
}

void CreateNodeContainer(screen_info* const Screen, tree_node* const Node, const container_type &ContainerType)
{
    if(Node->SplitRatio == 0)
        Node->SplitRatio = KWMScreen.SplitRatio;

    switch(ContainerType)
    {
        case LeftVertical:
        {
            Node->Container = LeftVerticalContainerSplit(Screen, Node->Parent);
        } break;
        case RightVertical:
        {
            Node->Container = RightVerticalContainerSplit(Screen, Node->Parent);
        } break;
        case UpperHorizontal:
        {
            Node->Container = UpperHorizontalContainerSplit(Screen, Node->Parent);
        } break;
        case LowerHorizontal:
        {
            Node->Container = LowerHorizontalContainerSplit(Screen, Node->Parent);
        } break;
    }

    Node->SplitMode = GetOptimalSplitMode(Node);
    Node->Container.Type = ContainerType;
}

void CreateNodeContainerPair(screen_info* const Screen, tree_node* const LeftNode, tree_node* const RightNode, const split_mode &SplitMode)
{
    if(SplitMode == SplitVertical)
    {
        CreateNodeContainer(Screen, LeftNode, LeftVertical);
        CreateNodeContainer(Screen, RightNode, RightVertical);
    }
    else
    {
        CreateNodeContainer(Screen, LeftNode, UpperHorizontal);
        CreateNodeContainer(Screen, RightNode, LowerHorizontal);
    }
}

tree_node *CreateLeafNode(screen_info* const Screen, tree_node* const Parent, const std::vector<window_info*> Windows, const container_type &ContainerType)
{
    tree_node *Leaf = (tree_node*) malloc(sizeof(tree_node));
    Leaf->Parent = Parent;
    Leaf->Windows = Windows;

    CreateNodeContainer(Screen, Leaf, ContainerType);

    Leaf->LeftChild = NULL;
    Leaf->RightChild = NULL;

    return Leaf;
}

tree_node *CreateRootNode()
{
    tree_node *RootNode = (tree_node*) malloc(sizeof(tree_node));
    std::memset(RootNode, '\0', sizeof(tree_node));

    RootNode->Windows = std::vector<window_info*>();
    RootNode->Parent = NULL;
    RootNode->LeftChild = NULL;
    RootNode->RightChild = NULL;
    RootNode->SplitRatio = KWMScreen.SplitRatio;
    RootNode->SplitMode = SplitModeUnset;

    return RootNode;
}

void SetRootNodeContainer(screen_info* const Screen, tree_node* const Node)
{
    space_info *Space = &Screen->Space[Screen->ActiveSpace];

    Node->Container.X = Screen->X + Space->Offset.PaddingLeft;
    Node->Container.Y = Screen->Y + Space->Offset.PaddingTop;
    Node->Container.Width = Screen->Width - Space->Offset.PaddingLeft - Space->Offset.PaddingRight;
    Node->Container.Height = Screen->Height - Space->Offset.PaddingTop - Space->Offset.PaddingBottom;
    Node->SplitMode = GetOptimalSplitMode(Node);

    Node->Container.Type = ContainerUnset;
}

void CreateLeafNodePair(screen_info* const Screen, tree_node* const Parent, const std::vector<window_info*> &FirstNodeWindows, const std::vector<window_info*> &SecondNodeWindows, const split_mode &SplitMode)
{
    Parent->Windows = std::vector<window_info*>();
    Parent->SplitMode = SplitMode;
    Parent->SplitRatio = KWMScreen.SplitRatio;

    std::vector<window_info*> LeftNodeWindows  = KWMTiling.SpawnAsLeftChild ? SecondNodeWindows : FirstNodeWindows;
    std::vector<window_info*> RightNodeWindows = KWMTiling.SpawnAsLeftChild ? FirstNodeWindows  : SecondNodeWindows;

    if(SplitMode == SplitVertical)
    {
        Parent->LeftChild = CreateLeafNode(Screen, Parent, LeftNodeWindows, LeftVertical);
        Parent->RightChild = CreateLeafNode(Screen, Parent, RightNodeWindows, RightVertical);
    }
    else
    {
        Parent->LeftChild = CreateLeafNode(Screen, Parent, LeftNodeWindows, UpperHorizontal);
        Parent->RightChild = CreateLeafNode(Screen, Parent, RightNodeWindows, LowerHorizontal);
    }
}

bool IsLeafNode(const tree_node* const Node)
{
    return Node->LeftChild == NULL && Node->RightChild == NULL ? true : false;
}

const tree_node *GetFirstLeafNode(const tree_node* const Node)
{
    const tree_node *Leaf = Node;
    if(Leaf)
    {
        while(Leaf->LeftChild)
            Leaf = Leaf->LeftChild;

        return Leaf;
    }

    return NULL;
}

const tree_node *GetLastLeafNode(const tree_node* const Node)
{
    const tree_node *Leaf = Node;
    if(Leaf)
    {
        while(Leaf->RightChild)
            Leaf = Leaf->RightChild;

        return Leaf;
    }

    return NULL;
}

// Find the first node without a window
const tree_node *GetFirstPseudoLeafNode(const tree_node* const Node)
{
    const tree_node *Leaf = GetFirstLeafNode(Node);
    while(Leaf && !Leaf->Windows.empty())
        Leaf = GetNearestNodeToTheRight(Leaf, SpaceModeBSP);

    return Leaf;
}

bool IsLeftChild(const tree_node* const Node)
{
    if(Node && IsLeafNode(Node))
    {
        tree_node *Parent = Node->Parent;
        return Parent->LeftChild == Node;
    }

    return false;
}

bool IsRightChild(const tree_node* const Node)
{
    if(Node && IsLeafNode(Node))
    {
        tree_node *Parent = Node->Parent;
        return Parent->RightChild == Node;
    }

    return false;
}

tree_node *CreateTreeFromWindowIDList(screen_info* const Screen, const std::vector<window_info*>* const WindowsPtr)
{
    if(IsSpaceFloating(Screen->ActiveSpace))
        return NULL;

    tree_node *RootNode = CreateRootNode();
    SetRootNodeContainer(Screen, RootNode);

    bool Result = false;
    space_info *Space = &Screen->Space[Screen->ActiveSpace];

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

bool CreateBSPTree(tree_node* const RootNode, screen_info* const Screen, const std::vector<window_info*>* const WindowsPtr)
{
    bool Result = false;
    std::vector<window_info*> Windows = *WindowsPtr;

    if(Windows.size() >= 2)
    {
        tree_node *Root = RootNode;
        bool FoundValidWindow = false;
        std::vector<window_info*>::iterator It;

        // TODO Need to bucketize windows into collections before creating nodes

        for(It = Windows.begin(); It != Windows.end(); ++It)
        {
            if(!IsWindowFloating((*It)->WID, NULL))
            {
                Root->Windows.push_back(*It);
                FoundValidWindow = true;
                break;
            }
        }

        if(!FoundValidWindow)
            return false;

        for(++It; It != Windows.end(); ++It)
        {
            if(!IsWindowFloating((*It)->WID, NULL))
            {
                while(!IsLeafNode(Root))
                {
                    if(!IsLeafNode(Root->RightChild) && IsLeafNode(Root->LeftChild))
                        Root = Root->LeftChild;
                    else
                        Root = Root->RightChild;
                }
                DEBUG("CreateBSPTree() Create pair of leafs")
                std::vector<window_info*> LeafWindows;
                LeafWindows.push_back(*It);
                CreateLeafNodePair(Screen, Root, Root->Windows, LeafWindows, GetOptimalSplitMode(Root));
                Root = RootNode;
            }
        }
    }
    else if(Windows.size() == 1 && !IsWindowFloating(Windows[0]->WID, NULL))
    {
        RootNode->Windows = Windows;
        Result = true;
    }

    return Result;
}

bool CreateMonocleTree(tree_node* const RootNode, screen_info* const Screen, const std::vector<window_info*>* const WindowsPtr)
{
    bool Result = false;
    std::vector<window_info*> Windows = *WindowsPtr;

    if(!Windows.empty())
    {
        tree_node *Root = RootNode;
        Root->Windows.push_back(Windows[0]);

        std::vector<window_info*>::iterator It;
        for(It = Windows.begin(); It != Windows.end(); ++It)
        {
            tree_node *Next = CreateRootNode();
            SetRootNodeContainer(Screen, Next);
            Next->Windows.push_back(*It);

            Root->RightChild = Next;
            Next->LeftChild = Root;
            Root = Next;
        }
        Result = true;
    }

    return Result;
}

split_mode GetOptimalSplitMode(const tree_node* const Node)
{
    return (Node->Container.Width / Node->Container.Height) >= 1.618 ? SplitVertical : SplitHorizontal;
}

void ChangeSplitRatio(const double Value)
 {
    if(Value > 0.0 && Value < 1.0)
    {
        DEBUG("ChangeSplitRatio() New Split-Ratio is " << Value)
        KWMScreen.SplitRatio = Value;
    }
}

void SwapNodeWindows(tree_node* const A, tree_node* const B)
 {
    if(A && B)
    {
        // Strings for DEBUG
        std::string WindowsA, WindowsB;
        std::vector<window_info*>::iterator It;
        for(It = A->Windows.begin(); It != A->Windows.end(); ++It)
            if (It != A->Windows.end())
                WindowsA.append(std::to_string((*It)->WID) + ", ");
        for(It = B->Windows.begin(); It != B->Windows.end(); ++It)
            if (It != B->Windows.end())
                WindowsB.append(std::to_string((*It)->WID) + ", ");

        DEBUG("SwapNodeWindows() " << WindowsA << " with " WindowsB)
        std::vector<window_info*> TempWindows = A->Windows;
        A->Windows = B->Windows;
        B->Windows = A->Windows;
        ResizeNodeToContainerSize(A);
        ResizeNodeToContainerSize(B);
    }
}

const tree_node *GetNearestLeafNeighbour(const tree_node* const Node, const space_tiling_option &Mode)
{
    if(Node && IsLeafNode(Node))
    {
        if(Mode == SpaceModeBSP)
            return IsLeftChild(Node) ? GetNearestNodeToTheRight(Node, Mode) : GetNearestNodeToTheLeft(Node, Mode);
        else if(Mode == SpaceModeMonocle)
        {
            return Node->LeftChild ? Node->LeftChild : Node->RightChild;
        }
    }

    return NULL;
}

const tree_node *GetNodeFromWindow(const tree_node* const Node, const window_info* const Window, const space_tiling_option &Mode)
{
    if(Node)
    {
        const tree_node *CurrentNode = GetFirstLeafNode(Node);;
        while(CurrentNode)
        {
            if(IsWindowInNode(Window,CurrentNode))
            {
                    DEBUG("GetNodeFromWindowID() " << CurrentNode->Windows[0]->WID)
                    return CurrentNode;
            }
            CurrentNode = GetNearestNodeToTheRight(CurrentNode, Mode);
        }
    }

    return NULL;
}

void ResizeNodeContainer(screen_info* const Screen, const tree_node *Node)
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

const tree_node *GetNearestNodeToTheLeft(const tree_node* const Node, const space_tiling_option &Mode)
{
    if(Node)
    {
        if(Mode == SpaceModeBSP)
        {
            if(Node->Parent)
            {
                tree_node *Root = Node->Parent;
                if(Root->LeftChild == Node)
                    return GetNearestNodeToTheLeft(Root, Mode);

                if(IsLeafNode(Root->LeftChild))
                    return Root->LeftChild;

                Root = Root->LeftChild;
                while(!IsLeafNode(Root->RightChild))
                    Root = Root->RightChild;

                return Root->RightChild;
            }
        }
        else if(Mode == SpaceModeMonocle)
        {
            return Node->LeftChild;
        }
    }

    return NULL;
}

const tree_node *GetNearestNodeToTheRight(const tree_node* const Node, const space_tiling_option &Mode)
{
    if(Node)
    {
        if(Mode == SpaceModeBSP)
        {
            if(Node->Parent)
            {
                tree_node *Root = Node->Parent;
                if(Root->RightChild == Node)
                    return GetNearestNodeToTheRight(Root, Mode);

                if(IsLeafNode(Root->RightChild))
                    return Root->RightChild;

                Root = Root->RightChild;
                while(!IsLeafNode(Root->LeftChild))
                    Root = Root->LeftChild;

                return Root->LeftChild;
            }
        }
        else if(Mode == SpaceModeMonocle)
        {
            return Node->RightChild;
        }
    }

    return NULL;
}

void CreateNodeContainers(screen_info* const Screen, tree_node* const Node, const bool &OptimalSplit)
{
    if(Node && Node->LeftChild && Node->RightChild)
    {
        Node->SplitMode = OptimalSplit ? GetOptimalSplitMode(Node) : Node->SplitMode;
        CreateNodeContainerPair(Screen, Node->LeftChild, Node->RightChild, Node->SplitMode);

        CreateNodeContainers(Screen, Node->LeftChild, OptimalSplit);
        CreateNodeContainers(Screen, Node->RightChild, OptimalSplit);
    }
}

void ToggleNodeSplitMode(screen_info* const Screen, tree_node* const Node)
{
    if(!Node || IsLeafNode(Node))
        return;

    Node->SplitMode = Node->SplitMode == SplitVertical ? SplitHorizontal : SplitVertical;
    CreateNodeContainers(Screen, Node, false);
    ApplyNodeContainer(Node, SpaceModeBSP);
}

void ApplyNodeContainer(const tree_node* const Node, const space_tiling_option Mode)
{
    if(Node)
    {
        if(Node->Windows.size() != 0)
            ResizeNodeToContainerSize(Node);

        if(Mode == SpaceModeBSP && Node->LeftChild)
            ApplyNodeContainer(Node->LeftChild, Mode);

        if(Node->RightChild)
            ApplyNodeContainer(Node->RightChild, Mode);
    }
}

void DestroyNodeTree(tree_node *Node, const space_tiling_option &Mode)
{
    if(Node)
    {
        if(Mode == SpaceModeBSP && Node->LeftChild)
            DestroyNodeTree(Node->LeftChild, Mode);

        if(Node->RightChild)
            DestroyNodeTree(Node->RightChild, Mode);

        free(Node);
    }
}

void RotateTree(tree_node* const Node, const int &Deg)
{
    if (Node == NULL || IsLeafNode(Node))
        return;

    DEBUG("RotateTree() " << Deg << " degrees")

    if((Deg == 90 && Node->SplitMode == SplitVertical) ||
       (Deg == 270 && Node->SplitMode == SplitHorizontal) ||
       Deg == 180)
    {
        tree_node *Temp = Node->LeftChild;
        Node->LeftChild = Node->RightChild;
        Node->RightChild = Temp;
        Node->SplitRatio = 1 - Node->SplitRatio;
    }

    if(Deg != 180)
        Node->SplitMode = Node->SplitMode == SplitHorizontal ? SplitVertical : SplitHorizontal;

    RotateTree(Node->LeftChild, Deg);
    RotateTree(Node->RightChild, Deg);
}

void CreateDeserializedNodeContainer(tree_node* const Node)
{
    split_mode SplitMode = Node->Parent->SplitMode;
    container_type ContainerType = ContainerUnset;

    if(SplitMode == SplitVertical)
        ContainerType = IsLeftChild(Node) ? LeftVertical : RightVertical;
    else
        ContainerType = IsLeftChild(Node) ? UpperHorizontal : LowerHorizontal;

    CreateNodeContainer(KWMScreen.Current, Node, ContainerType);
}

// TODO refactor
void FillDeserializedTree(const tree_node* const RootNode)
{
    std::vector<window_info*> Windows = GetAllWindowsOnDisplay(KWMScreen.Current->ID);
    const tree_node *Current = GetFirstLeafNode(RootNode);

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
        const tree_node *Root = RootNode;
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

void SerializeParentNode(const tree_node* const Parent, const std::string &Role, std::vector<std::string> &Serialized)
{
    Serialized.push_back("kwmc tree root create " + Role);
    Serialized.push_back("kwmc tree split-mode " + std::to_string(Parent->SplitMode));
    Serialized.push_back("kwmc tree split-ratio " + std::to_string(Parent->SplitRatio));

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

unsigned int DeserializeParentNode(tree_node* const Parent, const std::vector<std::string> &Serialized, const unsigned int &Index)
{
    unsigned int LineNumber = Index;
    for(;LineNumber < Serialized.size(); ++LineNumber)
    {
        std::string Line = Serialized[LineNumber];
        std::vector<std::string> Tokens = SplitString(Line, ' ');

        if(Tokens[2] == "split-mode")
        {
            Parent->SplitMode = static_cast<split_mode>(ConvertStringToInt(Tokens[3]));
            DEBUG("Root: SplitMode Found " + Tokens[3])
        }
        else if(Tokens[2] == "split-ratio")
        {
            Parent->SplitRatio = ConvertStringToDouble(Tokens[3]);
            DEBUG("Root: SplitRatio Found " + Tokens[3])
        }
        else if(Tokens[2] == "child")
        {
            DEBUG("Root: Child Found")
            DEBUG("Parent: " << Parent->SplitMode << "|" << Parent->SplitRatio)
            LineNumber = DeserializeChildNode(Parent, Serialized, LineNumber+1);
        }

        if(Parent->RightChild)
            return LineNumber;
    }

    return LineNumber;
}

unsigned int DeserializeChildNode(tree_node* const Parent, const std::vector<std::string> &Serialized, const unsigned int &Index)
{
    unsigned int LineNumber = Index;
    for(;LineNumber < Serialized.size(); ++LineNumber)
    {
        std::string Line = Serialized[LineNumber];
        if(Line == "kwmc tree root create left")
        {
            DEBUG("Child: Create root")
            Parent->LeftChild = CreateLeafNode(KWMScreen.Current, Parent, std::vector<window_info*>(), LeftVertical);
            CreateDeserializedNodeContainer(Parent->LeftChild);
            LineNumber = DeserializeParentNode(Parent->LeftChild, Serialized, LineNumber+1);
            return LineNumber;
        }
        else if(Line == "kwmc tree root create right")
        {
            DEBUG("Child: Create root")
            Parent->RightChild = CreateLeafNode(KWMScreen.Current, Parent, std::vector<window_info*>(), RightVertical);
            CreateDeserializedNodeContainer(Parent->RightChild);
            LineNumber = DeserializeParentNode(Parent->RightChild, Serialized, LineNumber+1);
            return LineNumber;
        }
        else if(Line == "kwmc tree leaf create left")
        {
            DEBUG("Child: Create left leaf")
            Parent->LeftChild = CreateLeafNode(KWMScreen.Current, Parent, std::vector<window_info*>(), LeftVertical);
            CreateDeserializedNodeContainer(Parent->LeftChild);
            return LineNumber;
        }
        else if(Line == "kwmc tree leaf create right")
        {
            DEBUG("Child: Create right leaf")
            Parent->RightChild = CreateLeafNode(KWMScreen.Current, Parent, std::vector<window_info*>(), RightVertical);
            CreateDeserializedNodeContainer(Parent->RightChild);
            return LineNumber;
        }
    }

    return LineNumber;
}

tree_node *DeserializeNodeTree(const std::vector<std::string> &Serialized)
{
    if(Serialized.empty() || Serialized[0] != "kwmc tree root create parent")
        return NULL;

    DEBUG("Deserialize: Create Master")
    tree_node *RootNode = CreateRootNode();
    SetRootNodeContainer(KWMScreen.Current, RootNode);
    DeserializeParentNode(RootNode, Serialized, 1);
    return RootNode;
}

void SaveBSPTreeToFile(screen_info* const Screen, const std::string &Name)
{
    if(IsSpaceInitializedForScreen(Screen))
    {
        space_info *Space = &Screen->Space[Screen->ActiveSpace];
        if(Space->Mode != SpaceModeBSP || IsLeafNode(Space->RootNode))
            return;

        std::string TempPath = KWMPath.EnvHome + "/" + KWMPath.ConfigFolder;
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

void LoadBSPTreeFromFile(screen_info* const Screen, const std::string &Name)
{
    if(IsSpaceInitializedForScreen(Screen))
    {
        space_info *Space = &Screen->Space[Screen->ActiveSpace];
        if(Space->Mode != SpaceModeBSP)
            return;

        std::string TempPath = KWMPath.EnvHome + "/" + KWMPath.ConfigFolder;
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

bool IsWindowInNode(const window_info* const Window, const tree_node* const Node)
{
    std::vector<window_info*>::const_iterator It;
    for (It = Node->Windows.begin(); It != Node->Windows.end(); ++It)
    {
        if ((*It)->WID == Window->WID)
            return true;
    }
    return false;
};
