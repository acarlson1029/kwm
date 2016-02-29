#ifndef TREE_H
#define TREE_H

#include "types.h"

/* Create new node_containers from existing node_containers.
    Map:
        node_container -> node_container
    Inputs:
        Screen - for current Space offsetting
        Container - start with copy of current Container
    Outputs:
        node_container - split node_container based on the input Node
 */
node_container LeftVerticalContainerSplit(screen_info *Screen, const node_container &Container);
node_container RightVerticalContainerSplit(screen_info *Screen, const node_container &Container);
node_container UpperHorizontalContainerSplit(screen_info *Screen, const node_container &Container);
node_container LowerHorizontalContainerSplit(screen_info *Screen, const node_container &Container);

/* Determine whether to split Vertically or Horizontally based on Node->Container.
    Map:
        node_container -> split_mode
    Input:
        Container - get dimensions of container
    Output:
        split_mode - vertical split or horizontal split
*/
split_mode GetOptimalSplitMode(const node_container &Container);

/* Mux ContainerType to select which node_container to create
    Map:
        node_container -> node_container
    Input:
        Screen - pass through to create container functions
        Parent - Node whose container to use as baseline
        ContainerType - Left/Right/Top/Bottom
    Output:
        node_container - new container dimensions for split.
*/
node_container CreateNodeContainer(screen_info *Screen, const node_container &ParentContainer, const container_type &ContainerType);

/* Mux SplitMode to select which ContainerTypes to use for CreateNodeContainer
    Map:
        node_container -> node_container
    Input:
        Screen - pass through to  create container functions
        Parent - Node whose children containers are being created.
        SplitMode - whether to split horizontally or vertically.
    Output:
        tree_node *Parent - mutate Left and Right children containers
*/
void CreateNodeContainerPair(screen_info *Screen, tree_node *Parent, const split_mode &SplitMode);

/* Create Container for RootNode
    Map:
        Screen Pos -> node_container
    Input:
        Screen - for position info
    Output:
        node_container *Container
*/
void SetRootNodeContainer(screen_info *Screen, node_container *Container);

/* Recursively create Containers for Tree starting from Node.
    Map:
        Recursive node_container -> node_container
    Input:
        screen_info *Screen - for Container functions
        tree_node *Node     - tree containing nodes to recursively process
    Output:
        tree_node *Node     - mutated children Containers
 */
void ResizeNodeContainer(screen_info *Screen, tree_node *Node);

/* Recursively create node_container pairs for Tree starting from Node.
    Map:
        Recursive node_container -> node_container
    Input:
        screen_info *Screen - for Container functions
        tree_node *Node     - tree containing nodes to recursively process
    Output:
        tree_node *Node     - created children Containers
*/
void CreateNodeContainers(screen_info *Screen, tree_node *Node, bool OptimalSplit);

/* Recursively resize all windows in nodes in subtree 
    Map:
        Window Resize -> node
    Input:
        tree_node *Node - subtree containing nodes to be resized
        space_tiling_window_option Mode - whether we're in SpaceModeBSP
    Output:
        tree_node *Node - resized windows in children
*/
void ApplyNodeContainer(tree_node *Node, space_tiling_option Mode);

/* Create new tree_node from Parent
    Map:
        <args> -> tree_node
    Input:
        Screen - for creating container
        Parent - for tree_node::Parent
        WindowID - for tree_node::WindowID
        ContainerType - for creating container
    Output:
        tree_node (new)
*/
tree_node *CreateLeafNode(screen_info *Screen, tree_node *Parent, int WindowID, const container_type &ContainerType);

/* Create new children tree_nodes from Parent
    Map:
        <args> -> Parent->LeftChild, Parent->RightChild
    Input:
        Screen - pass to CreateLeafNode()
        Parent - Node for which to create children
        FirstWindowID - WindowID of first child
        SecondWindowID - WindowID of second child
        SplitMode - create vertical or horizontal containers
    Output:
        node_info *Parent - mutate to delete its WindowID, 
                            update its Container, 
                            and create two children leaf nodes
 */
void CreateLeafNodePair(screen_info *Screen, tree_node *Parent, int FirstWindowID, int SecondWindowID, const split_mode &SplitMode);

/* Create an empty RootNode for the tree
    Map:
        <none> -> tree_node
    Input:
        <none>
    Output:
        tree_node* - populate elements with invalid/null/uninitialized data.
*/
tree_node *CreateRootNode();

/* Node properties */
bool IsLeafNode(tree_node *Node);
bool IsLeftChild(tree_node *Node);
bool IsRightChild(tree_node *Node);
bool IsLeftLeaf(tree_node *Node);
bool IsRightLeaf(tree_node *Node);

/* Tree traversal/selection */
/* GET functions -- no mutation of inputs */
tree_node *GetFirstLeafNode(tree_node *Node);   // left-most leaf
tree_node *GetLastLeafNode(tree_node *Node);    // right-most leaf
tree_node *GetNearestNodeToTheLeft(tree_node *Node, space_tiling_option Mode);
tree_node *GetNearestNodeToTheRight(tree_node *Node, space_tiling_option Mode);
tree_node *GetNearestLeafNeighbour(tree_node *Node, space_tiling_option Mode);
tree_node *GetFirstPseudoLeafNode(tree_node *Node);
tree_node *GetNodeFromWindowID(tree_node *Node, int WindowID, space_tiling_option Mode);

/* Swap the WindowIDs in two nodes and resize them to their new node_container.
    Input:
        A - first node to swap.
        B - second node to swap.
    Output:
        A - mutate WindowID and A->Container
        B - mutate WindowID and A->Container
*/
void SwapNodeWindowIDs(tree_node *A, tree_node *B);

/* Change split_mode for node, 
   recursively create new containers for subtree, and
   recursively resize all windows to fit in subtree node containers.
    Input:
        Screen - used for creating containers
        Node - root of subtree to process
    Output:
        tree_node *Node - Mutate SplitMode, Container, 
                          containers all nodes in subtree, and 
                          window size in all nodes of subtree.
*/
void ToggleNodeSplitMode(screen_info *Screen, tree_node *Node);

/* SET -- KWMScreen.SplitRatio
    TODO: Move to screen.*?
*/
void ChangeSplitRatio(double Value);

/*  Tree Operations */

/* Creation */
/* Create either a BSP Tree or MonocoleTree from the given list of Windows
    Map:
        std::vector<window_info*> -> Tree
    Input:
        Screen - used to determine the mode (BSP or Monocle)
        WindowsPtr - list of windows to put in the tree.
    Output:
        tree_node - Root of full tree.
*/
tree_node *CreateTreeFromWindowIDList(screen_info *Screen, std::vector<window_info*> *WindowsPtr);

/* Create a BSP Tree starting at the root node from a list of windows. 
    Map:
        std::vector<window_info*> -> Tree
    Input:
        RootNode - the root of the BSP tree to be created.
        Screen - pass through to create Containers.
        WindowsPtr - list of windows to put in the tree.
    Output:
        tree_node* RootNode - mutate the RootNode to populate with children created in this function.
        bool - Success status of the tree creation process.
*/
bool CreateBSPTree(tree_node *RootNode, screen_info *Screen, std::vector<window_info*> *WindowsPtr);

/* Create a Monocle Tree starting at the root node from a list of windows.
    Map:
        std::vector<window_info*> -> Tree
    Intput:
        RootNode - the root of the Monocle tree to be created.
        Screen - pass through to create Containers.
        WindowsPtr - list of windows to put in the tree.
    Output:
        tree_node* RootNode - mutate the RootNode to populate with children created in this function.
        bool - Success status of the tree creation process.
*/
bool CreateMonocleTree(tree_node *RootNode, screen_info *Screen, std::vector<window_info*> *WindowsPtr);

/* Recursively destroy the subtree and deallocate memory starting at Node
    Input:
        Node - root of the subtree to destroy
        Mode - whether the tree is BSP or Monocle
    Output:
        Node - destroy Node and all Nodes in subtree.
*/
void DestroyNodeTree(tree_node *Node, space_tiling_option Mode);

/* Recursively swap left and right children according to Deg
    Input:
        Node - root of the subtree to start swapping
        Deg - number of degrees to rotate (valid 90, 180, 270)
            90:  Rotate right, switch from SplitModeVertical to SplitModeHorizontal
            180: Keep split mode, just swap windows.
            270: Rotate left, switch from SplitModeHorizontal to SplitModeVertical
    Output:
        tree_node *Node - mutate LeftChild, RightChild, and Container for every node in subtree.
*/
void RotateTree(tree_node *Node, int Deg);

/* Save and Restore Trees from File */
void SaveBSPTreeToFile(screen_info *Screen, std::string Name);
void LoadBSPTreeFromFile(screen_info *Screen, std::string Name);
void SerializeParentNode(tree_node *Parent, std::string Role, std::vector<std::string> &Serialized);
unsigned int DeserializeParentNode(tree_node *Parent, std::vector<std::string> &Serialized, unsigned int Index);
unsigned int DeserializeChildNode(tree_node *Parent, std::vector<std::string> &Serialized, unsigned int Index);
tree_node *DeserializeNodeTree(std::vector<std::string> &Serialized);
void CreateDeserializedNodeContainer(tree_node *Node);
void FillDeserializedTree(tree_node *RootNode);

#endif
