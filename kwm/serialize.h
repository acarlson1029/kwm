/* Functions that save and restore Trees from file */
#ifndef SERIALIZE_H
#define SERIALIZE_H

#include "types.h"

void SaveBSPTreeToFile(screen_info *Screen, std::string Name);
void LoadBSPTreeFromFile(screen_info *Screen, std::string Name);
void SerializeParentNode(tree_node *Parent, std::string Role, std::vector<std::string> &Serialized);
unsigned int DeserializeParentNode(tree_node *Parent, std::vector<std::string> &Serialized, unsigned int Index);
unsigned int DeserializeChildNode(tree_node *Parent, std::vector<std::string> &Serialized, unsigned int Index);
tree_node *DeserializeNodeTree(std::vector<std::string> &Serialized);
void CreateDeserializedNodeContainer(tree_node *Node);
void FillDeserializedTree(tree_node *RootNode);

#endif
