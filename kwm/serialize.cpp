#include "serialize.h"
#include "tree.h"
#include "node.h"
#include "helpers.h"
#include "container.h"
#include "display.h"
#include "space.h"
#include "border.h"

extern kwm_screen KWMScreen;
extern kwm_path KWMPath;

void CreateDeserializedNodeContainer(tree_node *Node)
{
    split_mode SplitMode = Node->Parent->Container.SplitMode;
    container_type ContainerType;
    space_info *Space = GetActiveSpaceOfScreen(KWMScreen.Current);

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

    Node->Container = CreateNodeContainer(Space->Offset, Node->Parent->Container, ContainerType);
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
            CreateLeafNodePair(KWMScreen.Current, Root, Root->WindowID, Windows[Counter]->WID, GetOptimalSplitMode(Root->Container));
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
    tree_node *RootNode = CreateRootNode(KWMScreen.Current);
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
