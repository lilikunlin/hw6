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

    int getPredecessor(MWayNode* node, int index) {
        MWayNode* current = node->children[index];
        while (current->children[0]) {
            current = current->children.back();
        }
        return current->keys.back();
    }

    void fill(MWayNode* node, int index) {
        if (index != 0 && static_cast<int>(node->children[index - 1]->keys.size()) >= (m + 1) / 2) {
            borrowFromPrev(node, index);
        }
        else if (index != static_cast<int>(node->keys.size()) && static_cast<int>(node->children[index + 1]->keys.size()) >= (m + 1) / 2) {
            borrowFromNext(node, index);
        }
        else {
            if (index != static_cast<int>(node->keys.size())) {
                merge(node, index);
            }
            else {
                merge(node, index - 1);
            }
        }
    }

    void borrowFromPrev(MWayNode* node, int index) {
        MWayNode* child = node->children[index];
        MWayNode* sibling = node->children[index - 1];

        child->keys.insert(child->keys.begin(), node->keys[index - 1]);
        node->keys[index - 1] = sibling->keys.back();
        sibling->keys.pop_back();

        if (!sibling->children[0]) {
            child->children.insert(child->children.begin(), sibling->children.back());
            sibling->children.pop_back();
        }
    }

    void borrowFromNext(MWayNode* node, int index) {
        MWayNode* child = node->children[index];
        MWayNode* sibling = node->children[index + 1];

        child->keys.push_back(node->keys[index]);
        node->keys[index] = sibling->keys[0];
        sibling->keys.erase(sibling->keys.begin());

        if (!sibling->children[0]) {
            child->children.push_back(sibling->children[0]);
            sibling->children.erase(sibling->children.begin());
        }
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

    void removeFromNode(MWayNode* node, int key) {
        int index = static_cast<int>(std::lower_bound(node->keys.begin(), node->keys.end(), key) - node->keys.begin());

        if (index < static_cast<int>(node->keys.size()) && node->keys[index] == key) {
            if (!node->children[0]) {
                node->keys.erase(node->keys.begin() + index);
            }
            else {
                int pred = getPredecessor(node, index);
                node->keys[index] = pred;
                removeFromNode(node->children[index], pred);
            }
        }
        else {
            if (!node->children[0]) return;

            bool atLastChild = (index == static_cast<int>(node->keys.size()));

            if (static_cast<int>(node->children[index]->keys.size()) < (m + 1) / 2) {
                fill(node, index);
            }

            if (atLastChild && index > static_cast<int>(node->keys.size())) {
                removeFromNode(node->children[index - 1], key);
            }
            else {
                removeFromNode(node->children[index], key);
            }
        }
    }

    void merge(MWayNode* parent, int index) {
        MWayNode* child = parent->children[index];
        MWayNode* sibling = parent->children[index + 1];

        child->keys.push_back(parent->keys[index]);
        parent->keys.erase(parent->keys.begin() + index);

        for (int key : sibling->keys) {
            child->keys.push_back(key);
        }

        for (MWayNode* subChild : sibling->children) {
            child->children.push_back(subChild);
        }

        parent->children.erase(parent->children.begin() + index + 1);
        delete sibling;
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

    void remove(BTreeNode* node, int k) {
        int idx = std::lower_bound(node->keys.begin(), node->keys.end(), k) - node->keys.begin();

        if (idx < node->keys.size() && node->keys[idx] == k) {
            if (node->leaf) {
                node->keys.erase(node->keys.begin() + idx);
            }
            else {
                if (node->children[idx]->keys.size() >= t) {
                    int pred = getPredecessor(node, idx);
                    node->keys[idx] = pred;
                    remove(node->children[idx], pred);
                }
                else if (node->children[idx + 1]->keys.size() >= t) {
                    int succ = getSuccessor(node, idx);
                    node->keys[idx] = succ;
                    remove(node->children[idx + 1], succ);
                }
                else {
                    merge(node, idx);
                    remove(node->children[idx], k);
                }
            }
        }
        else {
            if (node->leaf) {
                std::cout << "Key " << k << " not found in the tree.\n";
                return;
            }

            bool flag = (idx == node->keys.size());

            if (node->children[idx]->keys.size() < t) {
                fill(node, idx);
            }

            if (flag && idx > node->keys.size()) {
                remove(node->children[idx - 1], k);
            }
            else {
                remove(node->children[idx], k);
            }
        }
    }

    int getPredecessor(BTreeNode* node, int idx) {
        BTreeNode* cur = node->children[idx];
        while (!cur->leaf) {
            cur = cur->children[cur->keys.size()];
        }
        return cur->keys[cur->keys.size() - 1];
    }

    int getSuccessor(BTreeNode* node, int idx) {
        BTreeNode* cur = node->children[idx + 1];
        while (!cur->leaf) {
            cur = cur->children[0];
        }
        return cur->keys[0];
    }

    void fill(BTreeNode* node, int idx) {
        if (idx != 0 && node->children[idx - 1]->keys.size() >= t) {
            borrowFromPrev(node, idx);
        }
        else if (idx != node->keys.size() && node->children[idx + 1]->keys.size() >= t) {
            borrowFromNext(node, idx);
        }
        else {
            if (idx != node->keys.size()) {
                merge(node, idx);
            }
            else {
                merge(node, idx - 1);
            }
        }
    }

    void borrowFromPrev(BTreeNode* node, int idx) {
        BTreeNode* child = node->children[idx];
        BTreeNode* sibling = node->children[idx - 1];

        child->keys.insert(child->keys.begin(), node->keys[idx - 1]);
        if (!child->leaf) {
            child->children.insert(child->children.begin(), sibling->children[sibling->children.size() - 1]);
        }

        node->keys[idx - 1] = sibling->keys[sibling->keys.size() - 1];
        sibling->keys.pop_back();
        if (!sibling->leaf) {
            sibling->children.pop_back();
        }
    }

    void borrowFromNext(BTreeNode* node, int idx) {
        BTreeNode* child = node->children[idx];
        BTreeNode* sibling = node->children[idx + 1];

        child->keys.push_back(node->keys[idx]);
        if (!child->leaf) {
            child->children.push_back(sibling->children[0]);
        }

        node->keys[idx] = sibling->keys[0];
        sibling->keys.erase(sibling->keys.begin());
        if (!sibling->leaf) {
            sibling->children.erase(sibling->children.begin());
        }
    }

    void merge(BTreeNode* node, int idx) {
        BTreeNode* child = node->children[idx];
        BTreeNode* sibling = node->children[idx + 1];

        child->keys.push_back(node->keys[idx]);

        for (int i = 0; i < sibling->keys.size(); ++i) {
            child->keys.push_back(sibling->keys[i]);
        }

        if (!child->leaf) {
            for (int i = 0; i < sibling->children.size(); ++i) {
                child->children.push_back(sibling->children[i]);
            }
        }

        node->keys.erase(node->keys.begin() + idx);
        node->children.erase(node->children.begin() + idx + 1);

        delete sibling;
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

    void remove(int k) {
        if (!root) return;

        remove(root, k);

        if (root->keys.empty()) {
            BTreeNode* tmp = root;
            if (root->leaf) {
                root = nullptr;
            }
            else {
                root = root->children[0];
            }
            delete tmp;
        }
    }

    void printTree(BTreeNode* node, int level) {
        if (!node) return;
        std::cout << std::string(level * 4, ' ');
        for (int key : node->keys) {
        std::cout << key << " ";
        std::cout << "\n"; // 在這裡添加換行符號
        }
        for (BTreeNode* child : node->children) {
            if (child) {
                printTree(child, level + 1);
            }
        }
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
        std::cout << "5. 從 B-tree 刪除\n";
        std::cout << "6. 顯示 B-tree\n";
        std::cout << "7. 離開\n";
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
            std::cout << "請輸入要刪除的值: ";
            std::cin >> value;
            bTree.remove(value);
            break;
        case 6:
            std::cout << "B-tree:\n";
            bTree.printTree();
            break;
        case 7:
            std::cout << "程式結束...\n";
            break;
        default:
            std::cout << "無效的選項，請重新選擇。\n";
        }
    } while (choice != 7);

    return 0;
}
