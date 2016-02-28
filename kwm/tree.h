#ifndef TREE_H
#define TREE_H

#include "types.h"

node_container LeftVerticalContainerSplit(screen_info* const Screen, const tree_node* const Node);
node_container RightVerticalContainerSplit(screen_info* const Screen, const tree_node* const Node);
node_container UpperHorizontalContainerSplit(screen_info* const Screen, const tree_node* const Node);
node_container LowerHorizontalContainerSplit(screen_info* const Screen, const tree_node* const Node);

void CreateNodeContainer(screen_info* const Screen, tree_node* const Node, const container_type &ContainerType);
void CreateNodeContainerPair(screen_info* const Screen, tree_node* const LeftNode, tree_node* const RightNode, const split_mode &SplitMode);
void SetRootNodeContainer(screen_info* const Screen, tree_node* const Node);
void ResizeNodeContainer(screen_info* const Screen, tree_node* const Node);
void CreateNodeContainers(screen_info* const Screen, tree_node* const Node, const bool &OptimalSplit);
void ApplyNodeContainer(const tree_node* const Node, const space_tiling_option Mode);

tree_node *CreateLeafNode(screen_info* const Screen, tree_node* const Parent, const std::vector<window_info*> Windows, const container_type &ContainerType);
void CreateLeafNodePair(screen_info* const Screen, tree_node* const Parent, const std::vector<window_info*> &FirstNodeWindows, const std::vector<window_info*> &SecondNodeWindows, const split_mode &SplitMode);
tree_node *CreateRootNode();

bool IsLeafNode(const tree_node* const Node);
bool IsLeftChild(const tree_node* const Node);
bool IsRightChild(const tree_node* const Node);

const tree_node *GetFirstLeafNode(const tree_node* const Node);
const tree_node *GetLastLeafNode(const tree_node* const Node);
const tree_node *GetFirstPseudoLeafNode(const tree_node* const Node);
const tree_node *GetNearestLeafNeighbour(const tree_node* const Node, const space_tiling_option &Mode);
const tree_node *GetNearestNodeToTheLeft(const tree_node* const Node, const space_tiling_option &Mode);
const tree_node *GetNearestNodeToTheRight(const tree_node* const Node, const space_tiling_option &Mode);
const tree_node *GetNodeFromWindow(const tree_node* const Node, const window_info* const Window, const space_tiling_option &Mode);
void SwapNodeWindows(tree_node* const A, tree_node* const B);
split_mode GetOptimalSplitMode(const tree_node* const Node);
void ToggleNodeSplitMode(screen_info* const Screen, const tree_node *Node);
void ChangeSplitRatio(const double Value);

tree_node *CreateTreeFromWindowIDList(screen_info* const Screen, const std::vector<window_info*>* const WindowsPtr);
bool CreateBSPTree(tree_node* const RootNode, screen_info* const Screen, const std::vector<window_info*>* const WindowsPtr);
bool CreateMonocleTree(tree_node* const RootNode, screen_info* const Screen, const std::vector<window_info*>* const WindowsPtr);
void DestroyNodeTree(tree_node *Node, const space_tiling_option &Mode);
void RotateTree(tree_node *Node, const int &Deg);

void SaveBSPTreeToFile(screen_info* const Screen, const std::string &Name);
void LoadBSPTreeFromFile(screen_info* const Screen, const std::string &Name);
void SerializeParentNode(const tree_node* const Parent, const std::string &Role, std::vector<std::string> &Serialized);
unsigned int DeserializeParentNode(tree_node *Parent, const std::vector<std::string> &Serialized, const unsigned int &Index);
unsigned int DeserializeChildNode(const tree_node *Parent, const std::vector<std::string> &Serialized, const unsigned int &Index);
tree_node *DeserializeNodeTree(const std::vector<std::string> &Serialized);
void CreateDeserializedNodeContainer(tree_node* const Node);
void FillDeserializedTree(const tree_node* const RootNode);

bool IsWindowInNode(const window_info* const Window, const tree_node* const Node)
#endif
