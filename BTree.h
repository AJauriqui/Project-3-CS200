#ifndef BTREE_H
#define BTREE_H

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

/*A B-tree that stores elements of type T ordered by the comparator Compare.
Compare(x, y) must return true if x should come BEFORE y (e.g. PatientPrio, DoctorPrio).
The tree owns its nodes but not the objects T points to (T is normally Patient* or Doctor*).
Elements that compare equal are all kept (multiset behaviour), so no patient or doctor is ever silently dropped.
The "first" element is the one that comes first according to Compare, i.e. the highest priority.*/
template <typename T, typename Compare = std::less<T> >
class BTree {
private:
    struct Node {
        bool leaf;                  // true if node has no children
        std::vector<T> keys;        // sorted elements (at most 2t-1)
        std::vector<Node*> children;// keys.size()+1 children when not a leaf
        explicit Node(bool isLeaf) : leaf(isLeaf) {}
    };

    Node* root;          // root node, nullptr when the tree is empty
    int t;               // minimum degree: every non-root node has between t-1 and 2t-1 keys
    std::size_t count;   // total number of elements stored
    Compare comp;        // ordering of the elements

    BTree(const BTree&);            // copying is disabled because the tree owns raw pointers
    BTree& operator=(const BTree&);

    /*Splits the full child x->children[i] into two nodes and moves its middle key up into x.
    Input is x, a non-full parent node, and i, the index of the full child. No output.*/
    void splitChild(Node* x, int i) {
        Node* y = x->children[i];
        Node* z = new Node(y->leaf);
        T middle = y->keys[t - 1];
        z->keys.assign(y->keys.begin() + t, y->keys.end());     // z takes the last t-1 keys
        if (!y->leaf) {
            z->children.assign(y->children.begin() + t, y->children.end()); // and the last t children
            y->children.erase(y->children.begin() + t, y->children.end());
        }
        y->keys.erase(y->keys.begin() + (t - 1), y->keys.end()); // y keeps the first t-1 keys
        x->keys.insert(x->keys.begin() + i, middle);
        x->children.insert(x->children.begin() + (i + 1), z);
    }

    /*Inserts value into the subtree rooted at x, which must not be full.
    Input is x, the node to insert into, and value, the element to insert. No output.*/
    void insertNonFull(Node* x, const T& value) {
        // upper_bound puts equal elements after existing ones
        std::size_t i = std::upper_bound(x->keys.begin(), x->keys.end(), value, comp) - x->keys.begin();
        if (x->leaf) {
            x->keys.insert(x->keys.begin() + i, value);
            return;
        }
        if ((int)x->children[i]->keys.size() == 2 * t - 1) {
            splitChild(x, (int)i);
            if (!comp(value, x->keys[i])) ++i; // value belongs to the right half
        }
        insertNonFull(x->children[i], value);
    }

    /*Makes sure x->children[0] has at least t keys so a key can be removed from it safely.
    Borrows a key through x from the right sibling, or merges with the sibling if it cannot spare one.
    Input is x, a non-leaf node. No output.*/
    void fillFirstChild(Node* x) {
        Node* c = x->children[0];
        Node* sib = x->children[1];
        if ((int)sib->keys.size() >= t) {      // borrow from right sibling
            c->keys.push_back(x->keys[0]);
            x->keys[0] = sib->keys.front();
            sib->keys.erase(sib->keys.begin());
            if (!c->leaf) {
                c->children.push_back(sib->children.front());
                sib->children.erase(sib->children.begin());
            }
        }
        else {                               // merge c + separator + sib
            c->keys.push_back(x->keys[0]);
            c->keys.insert(c->keys.end(), sib->keys.begin(), sib->keys.end());
            if (!c->leaf) c->children.insert(c->children.end(), sib->children.begin(), sib->children.end());
            x->keys.erase(x->keys.begin());
            x->children.erase(x->children.begin() + 1);
            delete sib;
        }
    }

    /*Removes and returns the first element of the subtree rooted at x.
    x is either the root or has at least t keys. Input is x, output is the removed element.*/
    T removeFirst(Node* x) {
        if (x->leaf) {
            T v = x->keys.front();
            x->keys.erase(x->keys.begin());
            return v;
        }
        if ((int)x->children[0]->keys.size() < t) fillFirstChild(x);
        return removeFirst(x->children[0]);
    }

    /*Adds every element of the subtree rooted at x to out in sorted order. Inputs are x and out, no return.*/
    void collect(const Node* x, std::vector<T>& out) const {
        if (!x) return;
        for (std::size_t i = 0; i < x->keys.size(); ++i) {
            if (!x->leaf) collect(x->children[i], out);
            out.push_back(x->keys[i]);
        }
        if (!x->leaf) collect(x->children[x->keys.size()], out);
    }

    /*Recursively frees every node under x. Input is x, no output.*/
    void destroy(Node* x) {
        if (!x) return;
        for (std::size_t i = 0; i < x->children.size(); ++i) destroy(x->children[i]);
        delete x;
    }

    template <typename Label>
    void printNode(std::ostream& os, const Node* x, int depth, Label label) const {
        os << std::string(depth * 4, ' ') << "[";
        for (std::size_t i = 0; i < x->keys.size(); ++i) {
            if (i) os << " | ";
            os << label(x->keys[i]);
        }
        os << "]\n";
        if (!x->leaf) {
            for (std::size_t i = 0; i < x->children.size(); ++i) printNode(os, x->children[i], depth + 1, label);
        }
    }

    /*Checks the B-tree rules for the subtree at x. lo/hi are the bounding keys from ancestors (or nullptr).
    leafDepth records the depth of the first leaf seen so all leaves can be compared to it.*/
    bool check(const Node* x, int depth, int& leafDepth, bool isRoot, const T* lo, const T* hi, std::size_t& seen) const {
        const int n = (int)x->keys.size();
        if (n > 2 * t - 1) return false;
        if (isRoot ? n < 1 : n < t - 1) return false;
        for (int i = 0; i < n; ++i) {
            if (i > 0 && comp(x->keys[i], x->keys[i - 1])) return false; // keys must be sorted
            if (lo && comp(x->keys[i], *lo)) return false;               // key smaller than ancestor bound
            if (hi && comp(*hi, x->keys[i])) return false;               // key larger than ancestor bound
        }
        seen += n;
        if (x->leaf) {
            if (!x->children.empty()) return false;
            if (leafDepth < 0) leafDepth = depth;
            return leafDepth == depth;                                    // all leaves at same depth
        }
        if ((int)x->children.size() != n + 1) return false;
        for (int i = 0; i <= n; ++i) {
            const T* childLo = (i == 0) ? lo : &x->keys[i - 1];
            const T* childHi = (i == n) ? hi : &x->keys[i];
            if (!check(x->children[i], depth + 1, leafDepth, false, childLo, childHi, seen)) return false;
        }
        return true;
    }

public:
    /*Creates an empty B-tree. Input is degree, the minimum degree t (at least 2), and optionally a comparator.
    Output is an empty tree.*/
    explicit BTree(int degree, Compare c = Compare()) : root(nullptr), t(degree < 2 ? 2 : degree), count(0), comp(c) {}

    /*Frees all nodes. No input or output.*/
    ~BTree() { destroy(root); }

    /*Inserts value into the tree. Input is value, no output.*/
    void insert(const T& value) {
        if (!root) root = new Node(true);
        if ((int)root->keys.size() == 2 * t - 1) {  // root is full, so the tree grows in height
            Node* s = new Node(false);
            s->children.push_back(root);
            splitChild(s, 0);
            root = s;
        }
        insertNonFull(root, value);
        ++count;
    }

    /*Removes the first (highest priority) element. Input is out, which receives the element.
    Output is true if an element was removed, false if the tree was empty.*/
    bool popFirst(T& out) {
        if (!root) return false;
        out = removeFirst(root);
        --count;
        if (root->keys.empty()) {  // root ran out of keys: shrink the tree
            Node* old = root;
            root = old->leaf ? nullptr : old->children[0];
            delete old;
        }
        return true;
    }

    /*Returns all elements in priority order, no input, output is a vector.*/
    std::vector<T> toVector() const {
        std::vector<T> out;
        out.reserve(count);
        collect(root, out);
        return out;
    }

    /*Removes every element (the pointed-to objects are not deleted), no input or output.*/
    void clear() {
        destroy(root);
        root = nullptr;
        count = 0;
    }

    /*Returns the number of elements in the tree.*/
    std::size_t size() const { return count; }

    /*Returns true if the tree has no elements.*/
    bool empty() const { return count == 0; }

    /*Returns the number of levels in the tree (0 when empty, 1 when only the root exists).*/
    int height() const {
        int h = 0;
        for (const Node* x = root; x; x = x->leaf ? nullptr : x->children[0]) ++h;
        return h;
    }

    /*Draws the tree sideways, one node per line, children indented under their parent.
    Inputs are os, the stream to print to, and label, a function turning one element into a string.*/
    template <typename Label>
    void print(std::ostream& os, Label label) const {
        if (!root) { os << "(empty)\n"; return; }
        printNode(os, root, 0, label);
    }

    /*Verifies the B-tree rules: key counts, sorted order, equal leaf depth and element count.
    Output is true if the tree is valid.*/
    bool checkInvariants() const {
        if (!root) return count == 0;
        int leafDepth = -1;
        std::size_t seen = 0;
        return check(root, 0, leafDepth, true, nullptr, nullptr, seen) && seen == count;
    }
};

#endif // BTREE_H