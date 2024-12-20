#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iomanip>
using namespace std;

// M-way 搜尋樹的節點結構
struct MWayNode {
    vector<int> keys; // 儲存節點的鍵值
    vector<MWayNode*> children; // 儲存子節點指標
    MWayNode(int m) {
        keys.reserve(m - 1);
        children.reserve(m);
    }
};

// M-way 搜尋樹的類別
class MWaySearchTree {
    int m; // M-way 搜尋樹的最大子節點數
    MWayNode* root; // 樹的根節點

    // 顯示樹的遞迴函數
    void display(MWayNode* node, int level) {
        if (!node) return;

        cout << string(level * 4, ' ');
        for (int key : node->keys) cout << key << " ";
        cout << endl;

        for (MWayNode* child : node->children) {
            display(child, level + 1);
        }
    }

    // 分裂節點
    void splitNode(MWayNode* node) {
        MWayNode* newNode = new MWayNode(m);
        int mid = m / 2;
        int midKey = node->keys[mid];

        // 分割鍵值和子節點
        vector<int> leftKeys(node->keys.begin(), node->keys.begin() + mid);
        vector<int> rightKeys(node->keys.begin() + mid + 1, node->keys.end());
        node->keys = leftKeys;

        newNode->keys = rightKeys;
        node->children.push_back(newNode);
        node->keys.push_back(midKey);
    }

    // 插入鍵值的遞迴函數
    void insertKey(MWayNode* node, int key) {
        if (node->children.empty()) {
            node->keys.push_back(key);
            sort(node->keys.begin(), node->keys.end());
            if (node->keys.size() >= m) {
                splitNode(node);
            }
        }
        else {
            for (size_t i = 0; i < node->keys.size(); ++i) {
                if (key < node->keys[i]) {
                    insertKey(node->children[i], key);
                    return;
                }
            }
            insertKey(node->children.back(), key);
        }
    }

public:
    MWaySearchTree(int m) : m(m), root(nullptr) {}

    // 插入資料到樹
    void insert(int key) {
        if (!root) {
            root = new MWayNode(m);
            root->keys.push_back(key);
        }
        else {
            insertKey(root, key);
        }
    }

    // 插入多個資料到樹
    void insertMultiple(const vector<int>& keys) {
        for (int key : keys) {
            insert(key);
        }
    }

    // 刪除資料
    void remove(int key) {
        if (removeKey(root, key)) {
            cout << "成功刪除鍵值 " << key << "\n";
        }
        else {
            cout << "無法找到鍵值 " << key << "\n";
        }
    }

    // 顯示樹的內容
    void display() {
        display(root, 0);
    }
};

// B-Tree 的節點結構
struct BTreeNode {
    vector<int> keys; // 儲存節點的鍵值
    vector<BTreeNode*> children; // 儲存子節點指標
    bool isLeaf; // 是否為葉節點

    BTreeNode(bool isLeaf) : isLeaf(isLeaf) {}
};

// B-Tree 的類別
class BTree {
    int t; // B-Tree 的最小度數
    BTreeNode* root; // 樹的根節點

    // 顯示樹的遞迴函數
    void display(BTreeNode* node, int level) {
        if (!node) return;

        cout << string(level * 4, ' ');
        for (int key : node->keys) cout << key << " ";
        cout << endl;

        for (BTreeNode* child : node->children) {
            display(child, level + 1);
        }
    }

    // 插入鍵值的遞迴函數
    void insertKey(BTreeNode* node, int key) {
        if (node->isLeaf) {
            node->keys.push_back(key);
            sort(node->keys.begin(), node->keys.end());
            if (node->keys.size() >= 2 * t - 1) {
                splitNode(node);
            }
        }
        else {
            for (size_t i = 0; i < node->keys.size(); ++i) {
                if (key < node->keys[i]) {
                    insertKey(node->children[i], key);
                    return;
                }
            }
            insertKey(node->children.back(), key);
        }
    }

    // 分裂節點
    void splitNode(BTreeNode* node) {
        BTreeNode* newNode = new BTreeNode(node->isLeaf);
        int mid = t - 1;
        int midKey = node->keys[mid];

        // 分割鍵值和子節點
        vector<int> leftKeys(node->keys.begin(), node->keys.begin() + mid);
        vector<int> rightKeys(node->keys.begin() + mid + 1, node->keys.end());
        node->keys = leftKeys;

        newNode->keys = rightKeys;
        node->children.push_back(newNode);
        node->keys.push_back(midKey);
    }

public:
    BTree(int t) : t(t), root(nullptr) {}

    // 插入資料到樹
    void insert(int key) {
        if (!root) {
            root = new BTreeNode(true);
            root->keys.push_back(key);
        }
        else {
            insertKey(root, key);
        }
    }

    // 插入多個資料到樹
    void insertMultiple(const vector<int>& keys) {
        for (int key : keys) {
            insert(key);
        }
    }

    // 刪除資料
    void remove(int key) {
        if (removeKey(root, key)) {
            cout << "成功刪除鍵值 " << key << "\n";
        }
        else {
            cout << "無法找到鍵值 " << key << "\n";
        }
    }

    // 顯示樹的內容
    void display() {
        display(root, 0);
    }
};

int main() {
    int m, t;
    cout << "請輸入 M-way 搜尋樹的最大子節點數 (m): ";
    cin >> m;

    cout << "請輸入 B-Tree 的最小度數 (t): ";
    cin >> t;

    MWaySearchTree mwayTree(m);
    BTree btree(t);

    int choice;
    while (true) {
        cout << "\n功能選單:\n";
        cout << "1. 插入資料到 M-way 搜尋樹\n";
        cout << "2. 插入資料到 B-Tree\n";
        cout << "3. 插入多筆資料到 M-way 搜尋樹\n";
        cout << "4. 插入多筆資料到 B-Tree\n";
        cout << "5. 刪除 M-way 搜尋樹中的資料\n";
        cout << "6. 刪除 B-Tree 中的資料\n";
        cout << "7. 顯示 M-way 搜尋樹\n";
        cout << "8. 顯示 B-Tree\n";
        cout << "9. 結束程式\n";
        cout << "請選擇功能: ";
        cin >> choice;

        if (choice == 1) {
            int key;
            cout << "請輸入要插入到 M-way 搜尋樹的資料: ";
            cin >> key;
            mwayTree.insert(key);
        }
        else if (choice == 2) {
            int key;
            cout << "請輸入要插入到 B-Tree 的資料: ";
            cin >> key;
            btree.insert(key);
        }
        else if (choice == 3) {
            string input;
            cout << "請輸入要插入到 M-way 搜尋樹的多筆資料（以空格分隔）: ";
            cin.ignore();
            getline(cin, input);
            stringstream ss(input);
            vector<int> keys;
            int key;
            while (ss >> key) {
                keys.push_back(key);
            }
            mwayTree.insertMultiple(keys);
        }
        else if (choice == 4) {
            string input;
            cout << "請輸入要插入到 B-Tree 的多筆資料（以空格分隔）: ";
            cin.ignore();
            getline(cin, input);
            stringstream ss(input);
            vector<int> keys;
            int key;
            while (ss >> key) {
                keys.push_back(key);
            }
            btree.insertMultiple(keys);
        }
        else if (choice == 5) {
            int key;
            cout << "請輸入要刪除的 M-way 搜尋樹資料: ";
            cin >> key;
            mwayTree.remove(key);
        }
        else if (choice == 6) {
            int key;
            cout << "請輸入要刪除的 B-Tree 資料: ";
            cin >> key;
            btree.remove(key);
        }
        else if (choice == 7) {
            cout << "M-way 搜尋樹:\n";
            mwayTree.display();
        }
        else if (choice == 8) {
            cout << "B-Tree:\n";
            btree.display();
        }
        else if (choice == 9) {
            break;
        }
        else {
            cout << "無效的選項，請重新選擇。\n";
        }
    }

    return 0;
}
