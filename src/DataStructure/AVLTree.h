#ifndef CPPTOOLKITS_AVLTREE_H
#define CPPTOOLKITS_AVLTREE_H

#include "noncopyable.h"
#include "disableCopyAndAssign.h"

template <typename K, typename V>
struct TreeNode
{
    K key;
    V value;
    int height{};
    TreeNode<K, V> *left;
    TreeNode<K, V> *right;

    TreeNode(K key, V value) : key(key), value(value), height(0), left(nullptr), right(nullptr)
    {}

    TreeNode() : key(), value(), height(0), left(nullptr), right(nullptr)
    {}

    TreeNode<K, V> &operator=(const TreeNode<K, V> &other)
    {
        if(this == &other)
            return *this;
        this->key = other.key;
        this->value = other.value;
        this->height = other.height;
        this->left = other.left;
        this->right = other.right;

        return *this;
    }

    TreeNode(const TreeNode<K, V> &other)
    {
        if(this == &other)
            return;
        this->key = other.key;
        this->value = other.value;
        this->height = other.height;
        this->left = other.left;
        this->right = other.right;
    }
};

template <typename K, typename V>
class AVLTree : noncopyable
{
    DISABLE_COPY_AND_ASSIGN(AVLTree);
public:
    AVLTree() = default;

    TreeNode<K, V> *get_root()
    {
        return root;
    }

    void insert(K key, V value)
    {
        root = insert(root, key, value);
    }

    static int get_hight(TreeNode<K, V> *node)
    {
        if(node == nullptr)
            return 0;
        return node->height;
    }

    bool empty()
    {
        return size == 0;
    }

private:
    TreeNode<K, V> *insert(TreeNode<K, V> *node, K key, V value)
    {
        if(node == nullptr)
        {
            size++;
            return new TreeNode<K, V>(key ,value);
        }

        if(key < node->key)
            node->left = insert(node->left, key, value);
        else if(key > node->key)
            node->right = insert(node->right, key, value);
        else
            node->value = value;

        update_hight(node);
        auto balance_factor = get_balance_factor(node);

        // LL
        if(balance_factor > 1 && get_balance_factor(node->left) >= 0)
            return r_rotate(node);

        // RR
        if (balance_factor < -1 && get_balance_factor(node->right) <= 0)
            return l_rotate(node);

        // LR
        if(balance_factor > 1 && get_balance_factor(node->right) < 0)
            return l_r_rotate(node);

        // RL
        if(balance_factor < -1 && get_balance_factor(node->left) > 0)
            return r_l_rotate(node);

        return node;
    }


    // 左-左型, 右旋
    //            node                       node_l
    //           /                           /     \
    //          node_l        --->    node_l_l      node
    //         /     \                             /
    //    node_l_l    node_l_r(null)          node_l_r
    TreeNode<K, V> *r_rotate(TreeNode<K, V> *node)
    {
        TreeNode<K, V> *node_l = node->left;
        TreeNode<K, V> *node_l_r = node_l->right;

        node_l->right = node;
        node->left = node_l_r;

        update_hight(node);
        update_hight(node_l);

        return node_l;
    }

    // 右-右型, 左旋
    //            node                        node_r
    //               \                        /     \
    //                node_r       --->    node     node_r_r
    //               /     \               /
    //  node_r_l(null)      node_r_r    node_r_l
    TreeNode<K, V> *l_rotate(TreeNode<K, V> *node)
    {
        TreeNode<K, V> *node_r = node->right;
        node->right = node_r->left;
        node->left = node;

        update_hight(node);
        update_hight(node_r);
    }

    // 右-左型，右左旋
    TreeNode<K, V> *r_l_rotate(TreeNode<K, V> *node)
    {
        node->left = r_rotate(node->left);
        return l_rotate(node);
    }

    // 左-右型，左右旋
    TreeNode<K, V> *l_r_rotate(TreeNode<K, V> *node)
    {
        node->right = l_rotate(node->right);
        return r_rotate(node);
    }

    TreeNode<K, V> *get_node(TreeNode<K, V> *node, K key)
    {
        if(root == nullptr)
            return nullptr;
        if(node->key == key)
            return node;
        else if(node->key < key)
            return get_node(node->left, key);
        else
            return get_node(node->right, key);
    }

    int get_balance_factor(TreeNode<K, V> *node)
    {
        if (root == nullptr)
            return 0;
        return get_hight(node->left) - get_hight(node->right);
    }

    inline void update_hight(TreeNode<K, V> *node)
    {
        node->height = MAX(get_hight(node->left), get_hight(node->right) + 1);
    }

private:
    unsigned long size = 0;
    TreeNode<K, V> *root = nullptr;
};


#endif //CPPTOOLKITS_AVLTREE_H
