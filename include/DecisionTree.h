#pragma once
#include <vector>
#include <string>
#include <memory>

/**
 * TreeNode - Represents a node in the narrative decision tree
 * Each node can have multiple children (choices)
 */
template <typename T>
class TreeNode {
public:
    T data;
    TreeNode* parent;
    std::vector<TreeNode*> children;
    
    // Constructor
    TreeNode(const T& nodeData) : data(nodeData), parent(nullptr) {}
    
    // Add a child node
    TreeNode* addChild(const T& childData) {
        TreeNode* child = new TreeNode(childData);
        child->parent = this;
        children.push_back(child);
        return child;
    }
    
    // Get number of children
    int getChildCount() const {
        return children.size();
    }
    
    // Check if node is a leaf (no children)
    bool isLeaf() const {
        return children.empty();
    }
    
    // Get parent node
    TreeNode* getParent() const {
        return parent;
    }
    
    // Get child at index
    TreeNode* getChild(int index) const {
        if (index >= 0 && index < children.size()) {
            return children[index];
        }
        return nullptr;
    }
    
    // Destructor - clean up all children recursively
    ~TreeNode() {
        for (auto child : children) {
            delete child;
        }
    }
};

/**
 * DecisionTree - Manages the narrative branching structure
 * Root represents the start of the game/memory
 * Each path from root to leaf represents a possible story outcome
 */
template <typename T>
class DecisionTree {
private:
    TreeNode<T>* root;
    int totalNodes;
    
public:
    // Constructor
    DecisionTree(const T& rootData) : totalNodes(1) {
        root = new TreeNode<T>(rootData);
    }
    
    // Get root node
    TreeNode<T>* getRoot() const {
        return root;
    }
    
    // Get total number of nodes in tree
    int getNodeCount() const {
        return totalNodes;
    }
    
    // Add a node as child of parent
    TreeNode<T>* addNode(TreeNode<T>* parent, const T& data) {
        if (parent == nullptr) return nullptr;
        
        TreeNode<T>* newNode = parent->addChild(data);
        totalNodes++;
        return newNode;
    }
    
    // Get height of tree (longest path from root to leaf)
    int getHeight() {
        return getHeightHelper(root);
    }
    
    // Get height from a specific node
    int getHeightHelper(TreeNode<T>* node) {
        if (node == nullptr) return 0;
        
        int maxHeight = 0;
        for (auto child : node->children) {
            maxHeight = std::max(maxHeight, getHeightHelper(child));
        }
        return 1 + maxHeight;
    }
    
    // Count leaf nodes (endings)
    int countLeaves() {
        return countLeavesHelper(root);
    }
    
    int countLeavesHelper(TreeNode<T>* node) {
        if (node == nullptr) return 0;
        if (node->isLeaf()) return 1;
        
        int leaves = 0;
        for (auto child : node->children) {
            leaves += countLeavesHelper(child);
        }
        return leaves;
    }
    
    // Destructor
    ~DecisionTree() {
        delete root;
    }
};

/**
 * StoryNode - A specialized node for story data
 * Contains narrative information and choice data
 */
struct StoryNode {
    std::string id;
    std::string title;
    std::string narration;
    std::string choiceText;
    std::string outcome;
    
    StoryNode() = default;
    StoryNode(const std::string& nodeId, const std::string& nodeTitle, 
              const std::string& nodeNarration)
        : id(nodeId), title(nodeTitle), narration(nodeNarration) {}
};

/**
 * NarrativeTree - Specialized tree for managing story branching
 * Tracks player choices and possible story paths
 */
class NarrativeTree {
private:
    DecisionTree<StoryNode>* storyTree;
    TreeNode<StoryNode>* currentNode;
    
public:
    // Constructor
    NarrativeTree(const StoryNode& root) {
        storyTree = new DecisionTree<StoryNode>(root);
        currentNode = storyTree->getRoot();
    }
    
    // Add a new story choice
    TreeNode<StoryNode>* addChoice(const StoryNode& choice) {
        TreeNode<StoryNode>* newNode = storyTree->addNode(currentNode, choice);
        return newNode;
    }
    
    // Move to a child node (make a choice)
    bool moveToChoice(int childIndex) {
        TreeNode<StoryNode>* child = currentNode->getChild(childIndex);
        if (child != nullptr) {
            currentNode = child;
            return true;
        }
        return false;
    }
    
    // Move back to parent node
    bool moveToParent() {
        if (currentNode->getParent() != nullptr) {
            currentNode = currentNode->getParent();
            return true;
        }
        return false;
    }
    
    // Get current node data
    StoryNode& getCurrentData() {
        return currentNode->data;
    }
    
    // Get current node
    TreeNode<StoryNode>* getCurrentNode() const {
        return currentNode;
    }
    
    // Get number of available choices
    int getChoiceCount() const {
        return currentNode->getChildCount();
    }
    
    // Get choice at index
    StoryNode* getChoice(int index) const {
        TreeNode<StoryNode>* child = currentNode->getChild(index);
        if (child != nullptr) {
            return &child->data;
        }
        return nullptr;
    }
    
    // Check if current node is an ending (leaf)
    bool isEnding() const {
        return currentNode->isLeaf();
    }
    
    // Get tree stats
    int getTotalNodes() const {
        return storyTree->getNodeCount();
    }
    
    int getTreeHeight() const {
        return storyTree->getHeight();
    }
    
    int getNumberOfEndings() const {
        return storyTree->countLeaves();
    }
    
    // Destructor
    ~NarrativeTree() {
        delete storyTree;
    }
};
