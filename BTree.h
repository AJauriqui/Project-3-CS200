#ifndef BTREE_H
#define BTREE_H

#include <string>
using namespace std;

template <typename T>
class BTreeNode {
public:
    int n;//number of keys
    bool leaf;//true if node is a leaf
    string* keys;//DOB keys for patients or doctors
    T** records;//patient or doctor records
    BTreeNode<T>** children;//children pointers
    
    /*Creates a new B-Tree node, input is t, a minimum degree of the b tree, and is leaf, whether node is a leaf
    Output is a new node with allocated arrays
    */
    BTreeNode(int t, bool isleaf);
    
};
template <typename T>
class BTree {
private:
    BTreeNode<T>* root;//pointer to the root node
    int t; // Minimum degree
    /*Splits full child node into two nodes and moves the middle key and record into the parent. 
    Input is x, a pointer to the parent node, and i, index of the child to split, there is no output*/
    void splitChild(BTreeNode<T>* x, int i);
    /*Inserts a record into a non-full node.
    Input is x, a pointer to the node to insert into, dob, the key to insert, and record, the record to insert.
    Output is nothing.
    */
    void insertNonFull(BTreeNode<T>* x, const string& dob, T* record);
    /*Recursively searches the B-Tree for a DOB, input is x, a pointer to the current node, and dob, the key to search for.
    Output is a pointer to the record if found, otherwise nullptr.
    */
    T* searchNode(BTreeNode<T>* x, const string& dob);
    /*Recursively deletes all nodes in the B-Tree.
    Input is x, a pointer to the current node.
    Output is nothing.
    */
    void deleteTree(BTreeNode<T>* x);//deletes all nodes in the tree
public:
    /*Creates an empty B-Tree, input is t, a minimum degree of the b tree.
    Output is a empty B-Tree.
    */
    BTree(int t);
    /*Destructor for the B-Tree.
    No input, output is nothing, frees all allocated B-Tree nodes and arrays.
    */
    ~BTree();

    /*Inserts a record into the B-Tree, duplicates are ignored.
    Input is dob, the key to insert, and record, pointer to the patient or doctor.
    Output is nothing.
    */
    void insert(const string& dob, T* record);
    /*Searches the B-Tree for a record.
    Input is dob, the key to search for.
    Output is a pointer to the record if found, otherwise nullptr.
    */
    T* search(const string& dob);
};
template <typename T>
BTreeNode<T>::BTreeNode(int t, bool isleaf) {
    n=0;//new node starts with no keys
    leaf=isleaf;//sets whether node is a leaf

    keys=new string[2*t-1];//allocates arrays based on degree t
    records=new T*[2*t-1];
    children=new BTreeNode<T>*[2*t];

    for(int i=0;i<2*t-1;i++){//initializes record pointers to nullptr
        records[i]=nullptr;
    }
    for(int i=0;i<2*t;i++){//initializes child pointers to nullptr
        children[i]=nullptr;
    }
}

template <typename T>
BTree<T>::BTree(int t) {
    root = nullptr;//tree starts empty
    this->t = t;//sets min degree of the B-Tree
}
template <typename T>
T* BTree<T>::searchNode(BTreeNode<T>* x, const string& dob) {
    if(x==nullptr){//return nullptr if node doesn't exist
        return nullptr;
    }
    int i = 0;
    while (i < x->n && dob > x->keys[i]) {//find the first key greater than or equal to dob
        i++;
    }
    if (i < x->n && dob == x->keys[i]) {//return the record if dob matches the key
        return x->records[i];
    }
    if (x->leaf) {//if node is a leaf, the dob isnt in the tree
        return nullptr;
    }
    return searchNode(x->children[i], dob);//searches the appropriate child
}

template <typename T>
T* BTree<T>::search(const string& dob) {
    return searchNode(root, dob);//begin search from root of the B-Tree
}

template <typename T>
void BTree<T>::splitChild(BTreeNode<T>* x, int i) {
    BTreeNode<T>* y=x->children[i];//gets the child node that needs to be split
    BTreeNode<T>* z = new BTreeNode<T>(t,y->leaf);//creates a new node with the same leaf status as y
    z->n = t - 1;//new node will contain the last t-1 keys
    for(int j=0;j<t-1;j++){//copies last t-1 keys and records from y to z
        z->keys[j]=y->keys[j+t];
        z->records[j]=y->records[j+t];
    }
    if(!y->leaf){//copies last t children from y to z if y is not a leaf
        for(int j=0;j<t;j++){
            z->children[j]=y->children[j+t];
        }
    }
    y->n=t-1;//og child keeps the first t-1 keys
    for(int j=x->n;j>=i+1;j--){//shifts children of x to make room for z
        x->children[j+1]=x->children[j];
    }
    x->children[i+1]=z;//places the new node right after y
    for(int j=x->n-1;j>=i;j--){//shifts keys and records of x to make room for y's middle key
        x->keys[j+1]=x->keys[j];
        x->records[j+1]=x->records[j];
    }
    x->keys[i]=y->keys[t-1];//move middle key from y to x
    x->records[i]=y->records[t-1];
    x->n++;//parent now has additional key
}

template <typename T>
void BTree<T>::insertNonFull(BTreeNode<T>* x, const string& dob, T* record) {
    int i = x->n - 1;//starts at the last key in the current node
    if (x->leaf) {
        while (i >= 0 && dob < x->keys[i]) {//shifts larger keys and records to the right
            x->keys[i + 1] = x->keys[i];
            x->records[i + 1] = x->records[i];
            i--;
        }
        x->keys[i + 1] = dob;//inserts new key and record
        x->records[i + 1] = record;
        x->n++;//increases number of keys in the node
    } else {//find child where new key belongs
        while (i >= 0 && dob < x->keys[i]) {
            i--;
        }
        i++;
        if (x->children[i]->n == 2 * t - 1) {//split child if its full
            splitChild(x, i);
            if (dob > x->keys[i]) {//determine which child should get the new dob
                i++;
            }
        }
        insertNonFull(x->children[i], dob, record);//insert into appropriate child
    }
}

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
        s->children[0]=root;//make old root the first child of the new root
        splitChild(s,0);//split the full child and move its middle key up
        root=s;//update root pointer
    }
    insertNonFull(root,dob,record);//insert new record into node that isn't full

}

template <typename T>
void BTree<T>::deleteTree(BTreeNode<T>* x) {
    if (x == nullptr) {//stop if node doesn't exist
        return;
    }
    if(!x->leaf){//recursively delete children if not a leaf
        for(int i=0;i<=x->n;i++){
            deleteTree(x->children[i]);
        }
    }
    //frees memory allocated for the node's arrays
    delete[] x->keys;
    delete[] x->records;
    delete[] x->children;
    delete x;//delete node itself
}

template <typename T>
BTree<T>::~BTree() {
    deleteTree(root);//free all allocated memory when B-tree is destroyed
}
#endif // BTREE_H