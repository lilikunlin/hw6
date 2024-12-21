#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <iomanip>

// m-way 搜尋樹的節點結構
struct MWayNode {
    int m; // 每個節點最多的鍵值數量
    std::vector<int> keys; // 儲存節點內的鍵值
    std::vector<MWayNode*> children; // 儲存子節點的指標

    // 節點的構造函式，初始化鍵值和子節點數量
    MWayNode(int m) : m(m) {
        keys.reserve(m - 1); // 預留鍵值的儲存空間
        children.resize(m, nullptr); // 初始化子節點指標的空間
    }
};

// m-way 搜尋樹類別
class MWayTree {
    MWayNode* root; // 樹的根節點
    int m; // 每個節點的階數

    // 遞迴地打印樹的結構
    void printTree(MWayNode* node, int level) {
        if (!node) return; // 如果節點為空，直接返回
        std::cout << std::string(level * 4, ' '); // 根據層次增加縮排
        for (int key : node->keys) std::cout << key << " "; // 打印當前節點的鍵值
        std::cout << "\n"; // 換行
        for (MWayNode* child : node->children) printTree(child, level + 1); // 打印子節點
    }

    // 獲取節點中某鍵值的前驅鍵值
    int getPredecessor(MWayNode* node, int index) {
        MWayNode* current = node->children[index]; // 進入左子節點
        while (current->children[0]) { // 不斷進入右子節點，找到最大鍵值
            current = current->children.back();
        }
        return current->keys.back(); // 返回前驅鍵值
    }

    // 處理子節點數量不足的情況
    void fill(MWayNode* node, int index) {
        if (index != 0 && static_cast<int>(node->children[index - 1]->keys.size()) >= (m + 1) / 2) {
            borrowFromPrev(node, index); // 從左兄弟借用鍵值
        }
        else if (index != static_cast<int>(node->keys.size()) && static_cast<int>(node->children[index + 1]->keys.size()) >= (m + 1) / 2) {
            borrowFromNext(node, index); // 從右兄弟借用鍵值
        }
        else {
            if (index != static_cast<int>(node->keys.size())) {
                merge(node, index); // 合併當前節點與右兄弟
            }
            else {
                merge(node, index - 1); // 合併當前節點與左兄弟
            }
        }
    }

    // 從左兄弟借用鍵值
    void borrowFromPrev(MWayNode* node, int index) {
        MWayNode* child = node->children[index]; // 當前節點的子節點
        MWayNode* sibling = node->children[index - 1]; // 左兄弟節點

        // 從父節點中借用鍵值並插入到當前子節點
        child->keys.insert(child->keys.begin(), node->keys[index - 1]);
        node->keys[index - 1] = sibling->keys.back(); // 更新父節點的鍵值
        sibling->keys.pop_back(); // 移除左兄弟的最後一個鍵值

        // 更新子節點的子節點指標
        if (sibling->children[0]) {
            child->children.insert(child->children.begin(), sibling->children.back());
            sibling->children.pop_back();
        }
    }

    // 從右兄弟借用鍵值
    void borrowFromNext(MWayNode* node, int index) {
        MWayNode* child = node->children[index]; // 當前節點的子節點
        MWayNode* sibling = node->children[index + 1]; // 右兄弟節點

        // 從父節點中借用鍵值並插入到當前子節點
        child->keys.push_back(node->keys[index]);
        node->keys[index] = sibling->keys[0]; // 更新父節點的鍵值
        sibling->keys.erase(sibling->keys.begin()); // 移除右兄弟的第一個鍵值

        // 更新子節點的子節點指標
        if (sibling->children[0]) {
            child->children.push_back(sibling->children[0]);
            sibling->children.erase(sibling->children.begin());
        }
    }

    // 將滿的子節點進行分裂
    void splitChild(MWayNode* parent, int index) {
        MWayNode* child = parent->children[index]; // 要分裂的子節點
        MWayNode* newChild = new MWayNode(m); // 創建一個新節點

        int mid = (m - 1) / 2; // 計算中間鍵值的索引
        parent->keys.insert(parent->keys.begin() + index, child->keys[mid]); // 將中間鍵值上移到父節點

        // 將右半部分鍵值移動到新節點
        for (int i = mid + 1; i < m - 1; i++) {
            newChild->keys.push_back(child->keys[i]);
        }
        child->keys.resize(mid); // 調整原子節點的鍵值數量

        // 如果子節點是內部節點，還需要處理子節點的子節點指標
        if (!child->children[0]) {
            for (int i = mid + 1; i < m; i++) {
                newChild->children[i - mid - 1] = child->children[i];
                child->children[i] = nullptr;
            }
        }

        parent->children.insert(parent->children.begin() + index + 1, newChild); // 將新節點插入到父節點
    }

    // 在非滿節點中插入鍵值
    void insertNonFull(MWayNode* node, int key) {
        int i = static_cast<int>(node->keys.size()) - 1;

        if (!node->children[0]) { // 如果節點是葉節點
            node->keys.push_back(0); // 暫時擴展空間
            while (i >= 0 && key < node->keys[i]) { // 從後向前移動鍵值
                node->keys[i + 1] = node->keys[i];
                i--;
            }
            node->keys[i + 1] = key; // 插入鍵值
        }
        else { // 如果節點是內部節點
            while (i >= 0 && key < node->keys[i]) {
                i--;
            }
            i++;

            if (static_cast<int>(node->children[i]->keys.size()) == m - 1) {
                splitChild(node, i); // 分裂滿節點
                if (key > node->keys[i]) {
                    i++;
                }
            }
            insertNonFull(node->children[i], key); // 遞迴插入到子節點
        }
    }

    // 從節點中刪除鍵值
    void removeFromNode(MWayNode* node, int key) {
        int index = static_cast<int>(std::lower_bound(node->keys.begin(), node->keys.end(), key) - node->keys.begin());

        if (index < static_cast<int>(node->keys.size()) && node->keys[index] == key) {
            if (!node->children[0]) { // 如果是葉節點，直接刪除鍵值
                node->keys.erase(node->keys.begin() + index);
            }
            else { // 如果是內部節點
                int pred = getPredecessor(node, index); // 找到前驅鍵值
                node->keys[index] = pred; // 用前驅鍵值替換
                removeFromNode(node->children[index], pred); // 從子節點中刪除前驅鍵值
            }
        }
        else {
            if (!node->children[0]) return; // 如果是葉節點，直接返回

            bool atLastChild = (index == static_cast<int>(node->keys.size()));

            if (static_cast<int>(node->children[index]->keys.size()) < (m + 1) / 2) {
                fill(node, index); // 處理子節點數量不足的情況
            }

            if (atLastChild && index > static_cast<int>(node->keys.size())) {
                removeFromNode(node->children[index - 1], key); // 從左兄弟中刪除
            }
            else {
                removeFromNode(node->children[index], key); // 從右兄弟中刪除
            }
        }
    }

    // 合併節點
    void merge(MWayNode* parent, int index) {
        MWayNode* child = parent->children[index]; // 要合併的節點
        MWayNode* sibling = parent->children[index + 1]; // 右兄弟節點

        child->keys.push_back(parent->keys[index]); // 將父節點的鍵值移入子節點
        parent->keys.erase(parent->keys.begin() + index); // 從父節點刪除鍵值

        for (int key : sibling->keys) {
            child->keys.push_back(key); // 將右兄弟的鍵值移入子節點
        }

        for (MWayNode* subChild : sibling->children) {
            child->children.push_back(subChild); // 將右兄弟的子節點移入子節點
        }

        parent->children.erase(parent->children.begin() + index + 1); // 刪除右兄弟的指標
        delete sibling; // 釋放右兄弟的記憶體
    }

public:
    // 初始化 m-way 搜尋樹
    MWayTree(int m) : root(nullptr), m(m) {}

    // 插入鍵值
    void insert(int key) {
        if (!root) {
            root = new MWayNode(m); // 如果根節點為空，創建新節點
            root->keys.push_back(key);
            return;
        }

        if (static_cast<int>(root->keys.size()) == m - 1) { // 如果根節點滿，進行分裂
            MWayNode* newRoot = new MWayNode(m);
            newRoot->children[0] = root;
            splitChild(newRoot, 0);
            root = newRoot;
        }

        insertNonFull(root, key); // 插入鍵值
    }

    // 刪除鍵值
    void remove(int key) {
        if (!root) return;

        removeFromNode(root, key);

        if (root->keys.empty() && root->children[0]) { // 如果根節點為空且有子節點
            MWayNode* oldRoot = root;
            root = root->children[0];
            delete oldRoot; // 釋放舊根節點的記憶體
        }
    }

    // 打印樹的結構
    void printTree() {
        printTree(root, 0);
    }
};

// B-tree 的節點結構
struct BTreeNode {
    int t; // 最小度數
    std::vector<int> keys; // 儲存鍵值的陣列
    std::vector<BTreeNode*> children; // 儲存子節點指標的陣列
    bool leaf; // 是否為葉節點

    // 節點的構造函式，初始化鍵值和子節點
    BTreeNode(int t, bool leaf) : t(t), leaf(leaf) {
        keys.reserve(2 * t - 1); // 預留最多 2t-1 個鍵值的空間
        children.resize(2 * t, nullptr); // 初始化最多 2t 個子節點的空間
    }
};

// B-tree 類別
class BTree {
    BTreeNode* root; // 樹的根節點
    int t; // B-tree 的最小度數

    // 分裂子節點
    void splitChild(BTreeNode* node, int i) {
        BTreeNode* z = new BTreeNode(t, node->children[i]->leaf); // 創建新節點 z
        BTreeNode* y = node->children[i]; // y 是要分裂的節點
        z->keys.assign(y->keys.begin() + t, y->keys.end()); // 將 y 的右半部分鍵值移到 z
        y->keys.resize(t - 1); // 調整 y 的鍵值數量

        if (!y->leaf) { // 如果 y 不是葉節點
            z->children.assign(y->children.begin() + t, y->children.end()); // 將 y 的右半部分子節點移到 z
            y->children.resize(t); // 調整 y 的子節點數量
        }

        // 更新父節點
        node->children.insert(node->children.begin() + i + 1, z); // 插入新節點 z
        node->keys.insert(node->keys.begin() + i, y->keys[t - 1]); // 將 y 的中間鍵值提升到父節點
    }

    // 插入鍵值到非滿節點
    void insertNonFull(BTreeNode* node, int k) {
        int i = static_cast<int>(node->keys.size()) - 1; // 初始化索引為最後一個鍵值

        if (node->leaf) { // 如果是葉節點
            node->keys.push_back(0); // 暫時擴展空間
            while (i >= 0 && k < node->keys[i]) { // 從後向前移動鍵值
                node->keys[i + 1] = node->keys[i];
                i--;
            }
            node->keys[i + 1] = k; // 插入鍵值
        }
        else { // 如果是內部節點
            while (i >= 0 && k < node->keys[i]) {
                i--;
            }
            i++;

            if (static_cast<int>(node->children[i]->keys.size()) == 2 * t - 1) { // 如果子節點已滿
                splitChild(node, i); // 分裂子節點
                if (k > node->keys[i]) {
                    i++;
                }
            }
            insertNonFull(node->children[i], k); // 遞迴插入到子節點
        }
    }

    // 從節點中刪除鍵值
    void remove(BTreeNode* node, int k) {
        int idx = std::lower_bound(node->keys.begin(), node->keys.end(), k) - node->keys.begin(); // 找到鍵值的位置

        if (idx < node->keys.size() && node->keys[idx] == k) { // 如果鍵值在節點中
            if (node->leaf) { // 如果是葉節點
                node->keys.erase(node->keys.begin() + idx); // 直接刪除鍵值
            }
            else { // 如果是內部節點
                if (node->children[idx]->keys.size() >= t) { // 如果左子節點有足夠鍵值
                    int pred = getPredecessor(node, idx); // 獲取前驅鍵值
                    node->keys[idx] = pred; // 替換為前驅鍵值
                    remove(node->children[idx], pred); // 遞迴刪除前驅鍵值
                }
                else if (node->children[idx + 1]->keys.size() >= t) { // 如果右子節點有足夠鍵值
                    int succ = getSuccessor(node, idx); // 獲取後繼鍵值
                    node->keys[idx] = succ; // 替換為後繼鍵值
                    remove(node->children[idx + 1], succ); // 遞迴刪除後繼鍵值
                }
                else { // 左右子節點都不夠，合併節點
                    merge(node, idx);
                    remove(node->children[idx], k); // 遞迴刪除鍵值
                }
            }
        }
        else { // 如果鍵值不在節點中
            if (node->leaf) { // 如果是葉節點
                std::cout << "Key " << k << " not found in the tree.\n";
                return;
            }

            bool flag = (idx == node->keys.size()); // 是否在最後一個子節點中

            if (node->children[idx]->keys.size() < t) { // 如果子節點鍵值數不足
                fill(node, idx); // 處理子節點數量不足的情況
            }

            if (flag && idx > node->keys.size()) {
                remove(node->children[idx - 1], k); // 從左子節點中刪除
            }
            else {
                remove(node->children[idx], k); // 從右子節點中刪除
            }
        }
    }

    // 獲取鍵值的前驅
    int getPredecessor(BTreeNode* node, int idx) {
        BTreeNode* cur = node->children[idx]; // 進入左子節點
        while (!cur->leaf) {
            cur = cur->children[cur->keys.size()]; // 找到最右邊的鍵值
        }
        return cur->keys[cur->keys.size() - 1];
    }

    // 獲取鍵值的後繼
    int getSuccessor(BTreeNode* node, int idx) {
        BTreeNode* cur = node->children[idx + 1]; // 進入右子節點
        while (!cur->leaf) {
            cur = cur->children[0]; // 找到最左邊的鍵值
        }
        return cur->keys[0];
    }

    // 處理子節點數量不足的情況
    void fill(BTreeNode* node, int idx) {
        if (idx != 0 && node->children[idx - 1]->keys.size() >= t) {
            borrowFromPrev(node, idx); // 從左兄弟借用鍵值
        }
        else if (idx != node->keys.size() && node->children[idx + 1]->keys.size() >= t) {
            borrowFromNext(node, idx); // 從右兄弟借用鍵值
        }
        else {
            if (idx != node->keys.size()) {
                merge(node, idx); // 合併當前節點與右兄弟
            }
            else {
                merge(node, idx - 1); // 合併當前節點與左兄弟
            }
        }
    }

    // 從左兄弟借用鍵值
    void borrowFromPrev(BTreeNode* node, int idx) {
        BTreeNode* child = node->children[idx];
        BTreeNode* sibling = node->children[idx - 1];

        child->keys.insert(child->keys.begin(), node->keys[idx - 1]); // 將父節點鍵值插入子節點
        if (!child->leaf) {
            child->children.insert(child->children.begin(), sibling->children.back()); // 更新子節點
        }

        node->keys[idx - 1] = sibling->keys.back(); // 更新父節點鍵值
        sibling->keys.pop_back(); // 移除左兄弟的鍵值
        if (!sibling->leaf) {
            sibling->children.pop_back(); // 刪除左兄弟的子節點
        }
    }

    // 從右兄弟借用鍵值
    void borrowFromNext(BTreeNode* node, int idx) {
        BTreeNode* child = node->children[idx];
        BTreeNode* sibling = node->children[idx + 1];

        child->keys.push_back(node->keys[idx]); // 從父節點借用鍵值
        if (!child->leaf) {
            child->children.push_back(sibling->children[0]); // 更新子節點
        }

        node->keys[idx] = sibling->keys[0]; // 更新父節點鍵值
        sibling->keys.erase(sibling->keys.begin()); // 移除右兄弟的鍵值
        if (!sibling->leaf) {
            sibling->children.erase(sibling->children.begin()); // 刪除右兄弟的子節點
        }
    }

    // 合併節點
    void merge(BTreeNode* node, int idx) {
        BTreeNode* child = node->children[idx];
        BTreeNode* sibling = node->children[idx + 1];

        child->keys.push_back(node->keys[idx]); // 將父節點鍵值移入子節點

        for (int i = 0; i < sibling->keys.size(); ++i) {
            child->keys.push_back(sibling->keys[i]); // 合併右兄弟的鍵值
        }

        if (!child->leaf) {
            for (int i = 0; i < sibling->children.size(); ++i) {
                child->children.push_back(sibling->children[i]); // 合併右兄弟的子節點
            }
        }

        node->keys.erase(node->keys.begin() + idx); // 刪除父節點中的鍵值
        node->children.erase(node->children.begin() + idx + 1); // 刪除右兄弟的指標

        delete sibling; // 釋放右兄弟的記憶體
    }

public:
    // 初始化 B-tree
    BTree(int t) : root(nullptr), t(t) {}

    // 插入鍵值
    void insert(int k) {
        if (!root) {
            root = new BTreeNode(t, true); // 如果根節點為空，創建根節點
            root->keys.push_back(k); // 插入鍵值
            return;
        }

        if (static_cast<int>(root->keys.size()) == 2 * t - 1) { // 如果根節點滿
            BTreeNode* newRoot = new BTreeNode(t, false); // 創建新根節點
            newRoot->children[0] = root; // 將舊根節點設為新根的子節點
            splitChild(newRoot, 0); // 分裂根節點
            root = newRoot; // 更新根節點
        }

        insertNonFull(root, k); // 插入鍵值
    }

    // 刪除鍵值
    void remove(int k) {
        if (!root) return;

        remove(root, k); // 遞迴刪除鍵值

        if (root->keys.empty()) { // 如果根節點鍵值數為空
            BTreeNode* tmp = root;
            if (root->leaf) { // 如果根節點是葉節點
                root = nullptr; // 樹變為空
            }
            else {
                root = root->children[0]; // 根節點下移
            }
            delete tmp; // 釋放舊根節點
        }
    }

    // 遞迴地打印 B-tree
    void printTree(BTreeNode* node, int level) {
        if (!node) return;
        std::cout << std::string(level * 4, ' '); // 根據層次增加縮排
        for (int key : node->keys) {
            std::cout << key << " ";
        }
        std::cout << "\n"; // 換行
        for (BTreeNode* child : node->children) {
            if (child) {
                printTree(child, level + 1); // 遞迴打印子節點
            }
        }
    }

    // 打印整棵 B-tree
    void printTree() {
        printTree(root, 0);
    }
};


int main() {
    int m, t; // 宣告變數 m (m-way 搜尋樹的階數) 和 t (B-tree 的最小度數)

    // 輸入 m-way 搜尋樹的階數，並初始化 MWayTree
    std::cout << "請輸入 m-way 搜尋樹的階數: ";
    std::cin >> m;
    MWayTree mwayTree(m); // 使用輸入的階數初始化 m-way 搜尋樹物件

    // 輸入 B-tree 的最小度數，並初始化 BTree
    std::cout << "請輸入 B-tree 的最小度數: ";
    std::cin >> t;
    BTree bTree(t); // 使用輸入的最小度數初始化 B-tree 物件

    int choice, value; // 宣告選擇功能的變數 (choice) 和鍵值變數 (value)

    // 進入功能選單的迴圈
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
        std::cin >> choice; // 讀取使用者的選擇

        // 根據使用者選擇執行相應功能
        switch (choice) {
        case 1: // 插入到 m-way 搜尋樹
            std::cout << "請輸入要插入的值: ";
            std::cin >> value; // 讀取要插入的值
            mwayTree.insert(value); // 執行插入操作
            break;

        case 2: // 從 m-way 搜尋樹刪除
            std::cout << "請輸入要刪除的值: ";
            std::cin >> value; // 讀取要刪除的值
            mwayTree.remove(value); // 執行刪除操作
            break;

        case 3: // 顯示 m-way 搜尋樹的結構
            std::cout << "m-way 搜尋樹:\n";
            mwayTree.printTree(); // 呼叫 printTree 打印樹的結構
            break;

        case 4: // 插入到 B-tree
            std::cout << "請輸入要插入到 B-tree 的值: ";
            std::cin >> value; // 讀取要插入的值
            bTree.insert(value); // 執行插入操作
            break;

        case 5: // 從 B-tree 刪除
            std::cout << "請輸入要刪除的值: ";
            std::cin >> value; // 讀取要刪除的值
            bTree.remove(value); // 執行刪除操作
            break;

        case 6: // 顯示 B-tree 的結構
            std::cout << "B-tree:\n";
            bTree.printTree(); // 呼叫 printTree 打印樹的結構
            break;

        case 7: // 離開程式
            std::cout << "程式結束...\n";
            break;

        default: // 處理無效的選項
            std::cout << "無效的選項，請重新選擇。\n";
        }
    } while (choice != 7); // 當選擇不為 7 時，重複顯示選單

    return 0;
}

