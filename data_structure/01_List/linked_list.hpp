#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <iostream>

template<typename T>
struct Node {
    T data;
    Node* next;
    Node* tail;
    Node(T val) : data(val), next(nullptr) {}
};

template<typename T>
class LinkedList {
public:
    LinkedList(){
        head = nullptr;
    };
    ~LinkedList(){
        Node<T>* current = head;
        while (current) {
            Node<T>* tmp = current;
            current = current->next;
            delete tmp;
        }
    };

    //尾插法
    void append(const T& val){
        Node<T>* newNode = new Node<T>(val);
        if(!head){
            head = newNode;
            return;
        }
        Node<T>* current = head;
    };     
    void prepend(const T& val);      // 在头部添加
    void remove(const T& val);       // 删除节点
    void print() const;         // 打印链表

private:
    Node<T>* head;
};

#endif
