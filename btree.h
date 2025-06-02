#ifndef BTree_H
#define BTree_H
#include <iostream>
#include "node.h"
#include <vector>

using namespace std;

template <typename TK>
class BTree {
private:
    Node<TK>* root;
    int M;  // grado u orden del arbol
    int n; // total de elementos en el arbol;

    void splitChild(Node<TK>* p, int idx) {
        Node<TK>* y = p->children[idx];
        Node<TK>* z = new Node<TK>(M);
        z->leaf = y->leaf;
        int mid = (M - 1) / 2;
        z->count = (M - 1) - mid - 1;
        for (int j = 0; j < z->count; ++j)
            z->keys[j] = y->keys[mid + 1 + j];
        if (!y->leaf) {
            for (int j = 0; j <= z->count; ++j)
                z->children[j] = y->children[mid + 1 + j];
        }
        y->count = mid;
        for (int j = p->count; j >= idx + 1; --j)
            p->children[j + 1] = p->children[j];
        p->children[idx + 1] = z;
        for (int j = p->count - 1; j >= idx; --j)
            p->keys[j + 1] = p->keys[j];
        p->keys[idx] = y->keys[mid];
        p->count += 1;
    }

    void insertNonFull(Node<TK>* x, TK k) {
        int i = x->count - 1;
        if (x->leaf) {
            while (i >= 0 && k < x->keys[i]) {
                x->keys[i + 1] = x->keys[i];
                --i;
            }
            x->keys[i + 1] = k;
            x->count += 1;
        } else {
            while (i >= 0 && k < x->keys[i]) --i;
            ++i;
            if (x->children[i]->count == M - 1) {
                splitChild(x, i);
                if (k > x->keys[i]) ++i;
            }
            insertNonFull(x->children[i], k);
        }
    }

    void mergeChildren(Node<TK>* x, int idx) {
        Node<TK>* y = x->children[idx];
        Node<TK>* z = x->children[idx + 1];
        y->keys[y->count] = x->keys[idx];
        for (int i = 0; i < z->count; ++i)
            y->keys[y->count + 1 + i] = z->keys[i];
        if (!y->leaf) {
            for (int i = 0; i <= z->count; ++i)
                y->children[y->count + 1 + i] = z->children[i];
        }
        y->count += z->count + 1;
        for (int i = idx + 1; i < x->count; ++i)
            x->keys[i - 1] = x->keys[i];
        for (int i = idx + 2; i <= x->count; ++i)
            x->children[i - 1] = x->children[i];
        x->count--;
        delete[] z->keys;
        delete[] z->children;
        delete z;
    }

    bool removeKey(Node<TK>* x, TK k) {
        int idx = 0;
        while (idx < x->count && k > x->keys[idx]) ++idx;
        if (idx < x->count && k == x->keys[idx]) {
            if (x->leaf) {
                for (int i = idx + 1; i < x->count; ++i)
                    x->keys[i - 1] = x->keys[i];
                x->count--;
                return true;
            }
            Node<TK>* predChild = x->children[idx];
            if (predChild->count >= (M + 1) / 2) {
                Node<TK>* cur = predChild;
                while (!cur->leaf) cur = cur->children[cur->count];
                TK pred = cur->keys[cur->count - 1];
                x->keys[idx] = pred;
                return removeKey(predChild, pred);
            }
            Node<TK>* succChild = x->children[idx + 1];
            if (succChild->count >= (M + 1) / 2) {
                Node<TK>* cur = succChild;
                while (!cur->leaf) cur = cur->children[0];
                TK succ = cur->keys[0];
                x->keys[idx] = succ;
                return removeKey(succChild, succ);
            }
            mergeChildren(x, idx);
            return removeKey(predChild, k);
        } else {
            if (x->leaf) return false;
            Node<TK>* child = x->children[idx];
            if (child->count == (M + 1) / 2 - 1) {
                Node<TK>* left = idx > 0 ? x->children[idx - 1] : nullptr;
                Node<TK>* right = idx < x->count ? x->children[idx + 1] : nullptr;
                if (left && left->count >= (M + 1) / 2) {
                    for (int i = child->count - 1; i >= 0; --i)
                        child->keys[i + 1] = child->keys[i];
                    if (!child->leaf) {
                        for (int i = child->count; i >= 0; --i)
                            child->children[i + 1] = child->children[i];
                    }
                    child->keys[0] = x->keys[idx - 1];
                    if (!child->leaf)
                        child->children[0] = left->children[left->count];
                    x->keys[idx - 1] = left->keys[left->count - 1];
                    child->count++;
                    left->count--;
                } else if (right && right->count >= (M + 1) / 2) {
                    child->keys[child->count] = x->keys[idx];
                    if (!child->leaf)
                        child->children[child->count + 1] = right->children[0];
                    x->keys[idx] = right->keys[0];
                    for (int i = 1; i < right->count; ++i)
                        right->keys[i - 1] = right->keys[i];
                    if (!right->leaf) {
                        for (int i = 1; i <= right->count; ++i)
                            right->children[i - 1] = right->children[i];
                    }
                    child->count++;
                    right->count--;
                } else {
                    if (left) {
                        mergeChildren(x, idx - 1);
                        child = left;
                    } else {
                        mergeChildren(x, idx);
                    }
                }
            }
            return removeKey(child, k);
        }
    }

    void inorderToString(Node<TK>* node, string& res, const string& sep) {
        if (!node) return;
        int i;
        for (i = 0; i < node->count; ++i) {
            if (!node->leaf) inorderToString(node->children[i], res, sep);
            res += to_string(node->keys[i]) + sep;
        }
        if (!node->leaf) inorderToString(node->children[i], res, sep);
    }

    void rangeSearchHelper(Node<TK>* node, TK begin, TK end, vector<TK>& ans) {
        if (!node) return;
        int i;
        for (i = 0; i < node->count; ++i) {
            if (!node->leaf) rangeSearchHelper(node->children[i], begin, end, ans);
            if (node->keys[i] >= begin && node->keys[i] <= end)
                ans.push_back(node->keys[i]);
        }
        if (!node->leaf) rangeSearchHelper(node->children[i], begin, end, ans);
    }



 public:
  BTree(int _M) : root(nullptr), M(_M) {}

  bool search(TK key){
        Node<TK>* x = root;
        while (x) {
            int i = 0;
            while (i < x->count && key > x->keys[i]) ++i;
            if (i < x->count && key == x->keys[i]) return true;
            if (x->leaf) return false;
            x = x->children[i];
        }
        return false;
    };//indica si se encuentra o no un elemento

  void insert(TK key){
      if (!root) {
          root = new Node<TK>(M);
          root->keys[0] = key;
          root->count = 1;
          n = 1;
          return;
      }
      if (root->count == M - 1) {
          Node<TK>* s = new Node<TK>(M);
          s->leaf = false;
          s->children[0] = root;
          splitChild(s, 0);
          root = s;
      }
      insertNonFull(root, key);
      n++;
  };//inserta un elemento

  void remove(TK key) {
      if (!root) return;
      if (removeKey(root, key)) n--;
      if (root->count == 0) {
          Node<TK> *old = root;
          root = root->leaf ? nullptr : root->children[0];
          delete old;
      }
  };//elimina un elemento

  int height(){
      int h = 0;
      Node<TK>* x = root;
      while (x && !x->leaf) {
          x = x->children[0];
          ++h;
      }
      return h;
  };//altura del arbol. Considerar altura 0 para arbol vacio
  string toString(const string& sep){
      string res;
      inorderToString(root, res, sep);
      if (!res.empty()) res.erase(res.size() - sep.size());
      return res;
  };  // recorrido inorder

  vector<TK> rangeSearch(TK begin, TK end){
      vector<TK> ans;
      rangeSearchHelper(root, begin, end, ans);
      return ans;
  };

  TK minKey(){
      Node<TK>* x = root;
      while (x && !x->leaf) x = x->children[0];
      return x->keys[0];
  };  // minimo valor de la llave en el arbol
  TK maxKey(){
      Node<TK>* x = root;
      while (x && !x->leaf) x = x->children[x->count];
      return x->keys[x->count - 1];
  };  // maximo valor de la llave en el arbol
  void clear(){
      if (root) root->killSelf();
      root = nullptr;
      n = 0;
  }; // eliminar todos los elementos del arbol

  int size(){
      return n;
  }; // retorna el total de elementos insertados

  // Construya un árbol B a partir de un vector de elementos ordenados
  static BTree* build_from_ordered_vector(vector<TK> elements,int k){
      BTree* t = new BTree(k);
      for (auto& e : elements) t->insert(e);
      return t;
  };
  // Verifique las propiedades de un árbol B
  bool check_properties(){
      //TODO
//1- cada nodo debe tener al menos M/2
//2- garantizar que las hojas esten al mismo nivel
// cada nodo debe tener count+1 hijos
//3- los elementos en el nodo deben estar ordenados
//4- dado un elemento en un nodo interno:
// - los elemenos del subarbol izquierdo son menores
// - los elemenos del subarbol derecho son mayores
//Si el árbol es vacío,debe de cumplir por defecto las propiedades
      if(!root) return true;
//Estoy usando dos vecotes como si fueran pila para el recorrido manual,nodos a visitar, y la profundidad
      vector<Node<TK>*> nodos;
      vector<int> niveles;
//Almacena la profundidad esperada
      int nivel= -1;
      nodos.push_back(root);
      niveles.push_back(0);
// Bucle principal: recorrido tipo DFS del árbol usando vectores
      while(!nodos.empty()) {
// Extraemos el nodo y su profundidad actual
          Node<TK>* nodo = nodos.back(); nodos.pop_back();
          int profundidad = niveles.back(); niveles.pop_back();
//Deben cumplir con la regla 1 excepto la raíz
          if(nodo!= root && nodo->count < (M/2-1)) return false;
//Llaves ordenadas y de forma creciente
          for(int i= 1; i<nodo->count; i++) {
              if(nodo->keys[i-1]> nodo->keys[i])
                  return false;
          }
//Todas las hojas deben tener el mismo nivel
          if(nodo->leaf) {
              if(nivel == -1) nivel= profundidad; //Guardamos la primera profundidad
              else if( nivel!= profundidad ) return false; //Si es diferente nivel no cumple
          }
//Si es hoja verificamos sus hijos
          if(!nodo->leaf) {
//un nodo interno debe tener count + 1 hijos no nulos
              for(int i=nodo->count; i>=0; i--) {
                  if(nodo->children[i] == nullptr) return false;
// Agregamos cada hijo al vector para seguir recorriendo
                  nodos.push_back(nodo->children[i]);
                  niveles.push_back(profundidad + 1);
              }
//separacion correcta de las claves entre subarboles izquierdo
              for(int i=0; i< nodo->count; i++) {
                  Node<TK>* left = nodo->children[i];
                  Node<TK>* right = nodo->children[i+1];
// El máximo del hijo izquierdo debe ser menor que la clave actual
                  if(left->keys[left->count-1]>= nodo->keys[i]) return false;
// El mínimo del hijo derecho debe ser mayor que la clave actual
                  if(right->keys[0] <= nodo->keys[i]) return false;
              }
          }
      }
      return true;
  };

  ~BTree(){
      clear();
  };     // liberar memoria
};

#endif
