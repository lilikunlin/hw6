#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <iomanip>

// m-way 搜尋樹的節點結構
struct MWayNode {
    int m; // 每個節點最多的鍵值數量
    std::vector<int> keys;
    std::vector<MWayNode*> children;

    MWayNode(int m) : m(m) {
        keys.reserve(m - 1);
        children.resize(m, nullptr);
    }
};

// m-way 搜尋樹類別
class MWayTree {
    MWayNode* root;
    int m;

    void printTree(MWayNode* node, int level) {
        if (!node) return;
        std::cout << std::string(level * 4, ' ');
        for (int key : node->keys) std::cout << key << " ";
        std::cout << "\n";
        for (MWayNode* child : node->children) printTree(child, level + 1);
    }

    void splitChild(MWayNode* parent, int index) {
        MWayNode* child = parent->children[index];
        MWayNode* newChild = new MWayNode(m);

        int mid = (m - 1) / 2;
        parent->keys.insert(parent->keys.begin() + index, child->keys[mid]);

        for (int i = mid + 1; i < m - 1; i++) {
            newChild->keys.push_back(child->keys[i]);
        }
        child->keys.resize(mid);

        if (!child->children[0]) {
            for (int i = mid + 1; i < m; i++) {
                newChild->children[i - mid - 1] = child->children[i];
                child->children[i] = nullptr;
            }
        }

        parent->children.insert(parent->children.begin() + index + 1, newChild);
    }

    void insertNonFull(MWayNode* node, int key) {
        int i = static_cast<int>(node->keys.size()) - 1;

        if (!node->children[0]) {
            node->keys.push_back(0);
            while (i >= 0 && key < node->keys[i]) {
                node->keys[i + 1] = node->keys[i];
                i--;
            }
            node->keys[i + 1] = key;
        }
        else {
            while (i >= 0 && key < node->keys[i]) {
                i--;
            }
            i++;

            if (static_cast<int>(node->children[i]->keys.size()) == m - 1) {
                splitChild(node, i);
                if (key > node->keys[i]) {
                    i++;
                }
            }
            insertNonFull(node->children[i], key);
        }
    }

    void merge(MWayNode* parent, int index) {
        MWayNode* left = parent->children[index];
        MWayNode* right = parent->children[index + 1];
        left->keys.push_back(parent->keys[index]);

        for (int key : right->keys) {
            left->keys.push_back(key);
        }

        for (MWayNode* child : right->children) {
            if (child) {
                left->children[static_cast<int>(left->keys.size())] = child;
            }
        }

        parent->keys.erase(parent->keys.begin() + index);
        parent->children.erase(parent->children.begin() + index + 1);

        delete right;
    }

    void removeFromNode(MWayNode* node, int key) {
        int index = static_cast<int>(std::lower_bound(node->keys.begin(), node->keys.end(), key) - node->keys.begin());

        if (index < static_cast<int>(node->keys.size()) && node->keys[index] == key) {
            if (!node->children[0]) {
                node->keys.erase(node->keys.begin() + index);
            }
            else {
                MWayNode* predNode = node->children[index];
                while (!predNode->children[0]) {
                    predNode = predNode->children.back();
                }
                int predKey = predNode->keys.back();
                removeFromNode(predNode, predKey);
                node->keys[index] = predKey;
            }
        }
        else {
            MWayNode* child = node->children[index];

            if (static_cast<int>(child->keys.size()) == (m - 1) / 2) {
                if (index > 0 && static_cast<int>(node->children[index - 1]->keys.size()) > (m - 1) / 2) {
                    MWayNode* leftSibling = node->children[index - 1];
                    child->keys.insert(child->keys.begin(), node->keys[index - 1]);
                    node->keys[index - 1] = leftSibling->keys.back();
                    leftSibling->keys.pop_back();

                    if (!leftSibling->children[0]) {
                        child->children.insert(child->children.begin(), leftSibling->children.back());
                        leftSibling->children.pop_back();
                    }
                }
                else if (index < static_cast<int>(node->keys.size()) && static_cast<int>(node->children[index + 1]->keys.size()) >(m - 1) / 2) {
                    MWayNode* rightSibling = node->children[index + 1];
                    child->keys.push_back(node->keys[index]);
                    node->keys[index] = rightSibling->keys[0];
                    rightSibling->keys.erase(rightSibling->keys.begin());

                    if (!rightSibling->children[0]) {
                        child->children.push_back(rightSibling->children[0]);
                        rightSibling->children.erase(rightSibling->children.begin());
                    }
                }
                else {
                    if (index < static_cast<int>(node->keys.size())) {
                        merge(node, index);
                    }
                    else {
                        merge(node, index - 1);
                    }
                }
            }

            removeFromNode(child, key);
        }
    }

public:
    MWayTree(int m) : root(nullptr), m(m) {}

    void insert(int key) {
        if (!root) {
            root = new MWayNode(m);
            root->keys.push_back(key);
            return;
        }

        if (static_cast<int>(root->keys.size()) == m - 1) {
            MWayNode* newRoot = new MWayNode(m);
            newRoot->children[0] = root;
            splitChild(newRoot, 0);
            root = newRoot;
        }

        insertNonFull(root, key);
    }

    void remove(int key) {
        if (!root) return;

        removeFromNode(root, key);

        if (root->keys.empty() && root->children[0]) {
            MWayNode* oldRoot = root;
            root = root->children[0];
            delete oldRoot;
        }
    }

    void printTree() {
        printTree(root, 0);
    }
};

// B-tree 的節點結構
struct BTreeNode {
    int t;
    std::vector<int> keys;
    std::vector<BTreeNode*> children;
    bool leaf;

    BTreeNode(int t, bool leaf) : t(t), leaf(leaf) {
        keys.reserve(2 * t - 1);
        children.resize(2 * t, nullptr);
    }
};

// B-tree 類別
class BTree {
    BTreeNode* root;
    int t;

    void splitChild(BTreeNode* node, int i) {
        BTreeNode* z = new BTreeNode(t, node->children[i]->leaf);
        BTreeNode* y = node->children[i];
        z->keys.assign(y->keys.begin() + t, y->keys.end());
        y->keys.resize(t - 1);

        if (!y->leaf) {
            z->children.assign(y->children.begin() + t, y->children.end());
            y->children.resize(t);
        }

        node->children.insert(node->children.begin() + i + 1, z);
        node->keys.insert(node->keys.begin() + i, y->keys[t - 1]);
    }

    void insertNonFull(BTreeNode* node, int k) {
        int i = static_cast<int>(node->keys.size()) - 1;

        if (node->leaf) {
            node->keys.push_back(0);
            while (i >= 0 && k < node->keys[i]) {
                node->keys[i + 1] = node->keys[i];
                i--;
            }
            node->keys[i + 1] = k;
        }
        else {
            while (i >= 0 && k < node->keys[i]) {
                i--;
            }
            i++;

            if (static_cast<int>(node->children[i]->keys.size()) == 2 * t - 1) {
                splitChild(node, i);
                if (k > node->keys[i]) {
                    i++;
                }
            }
            insertNonFull(node->children[i], k);
        }
    }

public:
    BTree(int t) : root(nullptr), t(t) {}

    void insert(int k) {
        if (!root) {
            root = new BTreeNode(t, true);
            root->keys.push_back(k);
            return;
        }

        if (static_cast<int>(root->keys.size()) == 2 * t - 1) {
            BTreeNode* newRoot = new BTreeNode(t, false);
            newRoot->children[0] = root;
            splitChild(newRoot, 0);
            root = newRoot;
        }

        insertNonFull(root, k);
    }

    void printTree(BTreeNode* node, int level) {
        if (!node) return;
        std::cout << std::string(level * 4, ' ');
        for (int key : node->keys) std::cout << key << " ";
        std::cout << "\n";
        for (BTreeNode* child : node->children) printTree(child, level + 1);
    }

    void printTree() {
        printTree(root, 0);
    }
};

int main() {
    int m, t;
    std::cout << "請輸入 m-way 搜尋樹的階數: ";
    std::cin >> m;
    MWayTree mwayTree(m);

    std::cout << "請輸入 B-tree 的最小度數: ";
    std::cin >> t;
    BTree bTree(t);

    int choice, value;
    do {
        std::cout << "\n功能選單:\n";
        std::cout << "1. 插入到 m-way 搜尋樹\n";
        std::cout << "2. 從 m-way 搜尋樹刪除\n";
        std::cout << "3. 顯示 m-way 搜尋樹\n";
        std::cout << "4. 插入到 B-tree\n";
        std::cout << "5. 顯示 B-tree\n";
        std::cout << "6. 離開\n";
        std::cout << "請選擇功能: ";
        std::cin >> choice;

        switch (choice) {
        case 1:
            std::cout << "請輸入要插入的值: ";
            std::cin >> value;
            mwayTree.insert(value);
            break;
        case 2:
            std::cout << "請輸入要刪除的值: ";
            std::cin >> value;
            mwayTree.remove(value);
            break;
        case 3:
            std::cout << "m-way 搜尋樹:\n";
            mwayTree.printTree();
            break;
        case 4:
            std::cout << "請輸入要插入到 B-tree 的值: ";
            std::cin >> value;
            bTree.insert(value);
            break;
        case 5:
            std::cout << "B-tree:\n";
            bTree.printTree();
            break;
        case 6:
            std::cout << "程式結束...\n";
            break;
        default:
            std::cout << "無效的選項，請重新選擇。\n";
        }
    } while (choice != 6);

    return 0;
}
