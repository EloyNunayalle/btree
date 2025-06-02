#ifndef NODE_H
#define NODE_H

using namespace std;

template <typename TK>
struct Node {
  // array de keys
  TK* keys;
  // array de punteros a hijos
  Node** children;
  // cantidad de keys
  int count;
  // indicador de nodo hoja
  bool leaf;

  Node() : keys(nullptr), children(nullptr), count(0) {}
  Node(int M) {
    keys = new TK[M - 1];
    children = new Node<TK>*[M];
    count = 0;
    leaf = true;
  }

  void killSelf() {
      if (!leaf) {
          for (int i = 0; i <= count; ++i)
              if (children[i]) children[i]->killSelf();
      }
      delete[] keys;
      delete[] children;
      delete this;
  }
};

#endif