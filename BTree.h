#ifndef BTREE_H
#define BTREE_H

#include <string>
using namespace std;

template <typename T>
class BTreeNode {
public:
    int n;//number of keys
    bool leaf;//true if node is a leaf
    string* keys;//DOB's of doctors
    T** records;//patient or doctor records
    BTreeNode<T>** children;//children pointers
    BTreeNode(int t, bool isleaf);
    
};
template <typename T>
class BTree {
private:
    BTreeNode<T>* root;
    int t; // Minimum degree
    void splitChild(BTreeNode<T>* x, int i);//splits child into 2 nodes and moves middle key into parent
    void insertNonFull(BTreeNode<T>* x, const string& dob, T* record);//inserts record into a non-full node
    T* searchNode(BTreeNode<T>* x, const string& dob);//searches for a record in a node
    void deleteTree(BTreeNode<T>* x);//deletes all nodes in the tree
public:
    BTree(int t);
    ~BTree();

    void insert(const string& dob, T* record);
    T* search(const string& dob);
};
//constructor for a B-Tree node
template <typename T>
BTreeNode<T>::BTreeNode(int t, bool isleaf) {
    n=0;
    leaf=isleaf;
    keys=new string[2*t-1];
    records=new T*[2*t-1];
    children=new BTreeNode<T>*[2*t];

    for(int i=0;i<2*t-1;i++){
        records[i]=nullptr;
    }
    for(int i=0;i<2*t;i++){
        children[i]=nullptr;
    }
}
//constructor for a B-tree
template <typename T>
BTree<T>::BTree(int t) {
    root = nullptr;
    this->t = t;
}
template <typename T>
T* BTree<T>::searchNode(BTreeNode<T>* x, const string& dob) {
    if(x==nullptr){
        return nullptr;
    }
    int i = 0;
    while (i < x->n && dob > x->keys[i]) {
        i++;
    }
    if (i < x->n && dob == x->keys[i]) {
        return x->records[i];
    }
    if (x->leaf) {
        return nullptr;
    }
    return searchNode(x->children[i], dob);
}
//searches B-Tree starting at the root
template <typename T>
T* BTree<T>::search(const string& dob) {
    return searchNode(root, dob);
}
//splits full node using CLRS B-TREE-SPLIT-CHILD
template <typename T>
void BTree<T>::splitChild(BTreeNode<T>* x, int i) {
    BTreeNode<T>* y=x->children[i];
    BTreeNode<T>* z = new BTreeNode<T>(t,y->leaf);
    z->n = t - 1;
    for(int j=0;j<t-1;j++){//copies last t-1 keys and records from y to z
        z->keys[j]=y->keys[j+t];
        z->records[j]=y->records[j+t];
    }
    if(!y->leaf){//copies last t children from y to z if y is not a leaf
        for(int j=0;j<t;j++){
            z->children[j]=y->children[j+t];
        }
    }
    y->n=t-1;//reduces number of keys in y
    for(int j=x->n;j>=i+1;j--){//shifts children of x to make room for z
        x->children[j+1]=x->children[j];
    }
    x->children[i+1]=z;
    for(int j=x->n-1;j>=i;j--){//shifts keys and records of x to make room for y's middle key
        x->keys[j+1]=x->keys[j];
        x->records[j+1]=x->records[j];
    }
    x->keys[i]=y->keys[t-1];//move middle key from y to x
    x->records[i]=y->records[t-1];
    x->n++;
}
//inserts a record into a non full node using CLRS B-TREE-INSERT-NONFULL
template <typename T>
void BTree<T>::insertNonFull(BTreeNode<T>* x, const string& dob, T* record) {
    int i = x->n - 1;
    if (x->leaf) {
        while (i >= 0 && dob < x->keys[i]) {//shifts larger keys and records to the right
            x->keys[i + 1] = x->keys[i];
            x->records[i + 1] = x->records[i];
            i--;
        }
        x->keys[i + 1] = dob;//inserts new key and record
        x->records[i + 1] = record;
        x->n++;
    } else {//find child where new key belonds
        while (i >= 0 && dob < x->keys[i]) {
            i--;
        }
        i++;
        if (x->children[i]->n == 2 * t - 1) {//split child if its full
            splitChild(x, i);
            if (dob > x->keys[i]) {
                i++;
            }
        }
        insertNonFull(x->children[i], dob, record);//insert into appropriate child
    }
}
//inserts a record into B-Tree using CLRS B-Tree-INSERT
template <typename T>
void BTree<T>::insert(const string& dob, T* record) {
    if(search(dob)!=nullptr){//dont insert duplicate dobs
        return;
    }
    if(root==nullptr){//if tree is empty create new root
        root=new BTreeNode<T>(t,true);
    }
    if(root->n==2*t-1){//if root is full create new root and split
        BTreeNode<T>* s=new BTreeNode<T>(t,false);
        s->children[0]=root;
        splitChild(s,0);
        root=s;
    }
    insertNonFull(root,dob,record);

}
//recursive function to delete all nodes in the tree
template <typename T>
void BTree<T>::deleteTree(BTreeNode<T>* x) {
    if (x == nullptr) {
        return;
    }
    if(!x->leaf){//recursively delete children if not a leaf
        for(int i=0;i<=x->n;i++){
            deleteTree(x->children[i]);
        }
    }
    delete[] x->keys;
    delete[] x->records;
    delete[] x->children;
    delete x;
}
//destructor for BTree
template <typename T>
BTree<T>::~BTree() {
    deleteTree(root);
}
#endif // BTREE_H