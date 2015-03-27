#include "F_heap.h"



template <class Type>
FibonacciHeap <Type> :: FibonacciHeap() : minNode(nullptr), numItem(0) {
}

template <class Type>
FibonacciHeap <Type> :: ~FibonacciHeap (){

}

template <class Type> void
FibonacciHeap <Type> :: print() {
	Node *p = minNode;
	do {
		std::cerr << p->data << " ";
		p = p->rsibling;
	} while(p != minNode);
	std::cerr << "\n";
}

// Add a new single-node min tree to the collection
// Update min-element pointer if necessary
template <class Type> void
FibonacciHeap <Type> :: insert (Type item){
	Node *temp = new (std::nothrow) Node;
	if (nullptr == temp) {
		std::cerr << "ERROR : Not enough memory. EXIT !!!\n";
		exit(1);
	}
	//add item to data field
	temp->data = item;
	//if the heap is empty, left and right sibling point to itself
	if (nullptr == minNode) {
		minNode = temp;
		minNode->lsibling = minNode;
		minNode->rsibling = minNode;
	}
	//if the heap is not empty, insert to top circular list
	else {
		list_insert(temp, minNode);
		//update min-element pointer if necessary
		if (temp->data < minNode->data) {
			minNode = temp;
		}
	}
	//increment number of items
	++numItem;
} 

// meld heap2 to heap1, heap2 will be empty after meld
// Combine the 2 top-level circular lists.
template <class Type> void 
FibonacciHeap <Type> :: meld (FibonacciHeap<Type> &heap){
	//if heap2 is empty, nothing to meld to heap1
	if (nullptr == heap.minNode) {
		return;
	}
	//if heap1 is empty, copy heap2 to heap1
	else if (nullptr == minNode) {
		minNode = heap.minNode;
		numItem = heap.numItem;
		//consumed heap2
		heap.minNode = nullptr;
		heap.numItem = 0;
		return;
	} else {
		//update the heap pointer
		meld_list(minNode, heap.minNode);
		//update number of items
		numItem += heap.numItem;
		//update min pointer if necessary
		minNode = (minNode->data < heap.minNode->data)? minNode : heap.minNode;
		//consumed heap2
		heap.minNode = nullptr;
		heap.numItem = 0;
	}	
}

// Remove a min tree
// Reinsert subtrees of removed min tree
// Update binomial heap pointer
template <class Type> int
FibonacciHeap <Type> :: remove_min (Type &item){
	if (minNode == nullptr) {
		return 0;
	}

	item = minNode->data;
	//if the min node does not have children
	if (nullptr == minNode->child) {
		// list_remove(minNode);

		minNode->rsibling->lsibling = minNode->lsibling;
		minNode->lsibling->rsibling = minNode->rsibling;

		Node *iter = minNode->rsibling;
		Node *min = iter;
		do {
			if(iter->data < min->data){
				min = iter;
			}
			iter = iter->rsibling;
		} while(iter != minNode->rsibling);

		//delete min node
		delete minNode;	
		minNode = min;
	} 
	else {
		Node *childptr = minNode->child;
		//set all children's parent field to nullptr and childCut field to false
		do {
			childptr->parent = nullptr;
			childptr->childCut = false;
			childptr = childptr->rsibling;
		} while (childptr != minNode->child);
		//take out min node and meld the children list into the top doubly linked list
		childptr->lsibling->rsibling = minNode->rsibling;
		minNode->rsibling->lsibling = childptr->lsibling;
		childptr->lsibling = minNode->lsibling;
		minNode->lsibling->rsibling = childptr;
		//delete min node
		delete minNode;	
		//find the new min node
		find_min(childptr);
	}	

	// pairwise combine trees in heap and update heap pointer
	pairwise_combine();
	--numItem;
	return 1;
}

template <class Type> void 
FibonacciHeap <Type> :: remove (Node *rmMe){
	if (nullptr == rmMe) {
		std::cerr << "ERROR: Attempt to remove non-exist node\n";
		exit(1);
	}
	//if the node is min node
	if (rmMe == minNode) {
		Type dummy;
		remove_min(dummy);
	} else {
		Node *parent = rmMe->parent;
		//cut the subtree rooted in the node
		cut_subtree(rmMe);		
		//update parent and childCut for all children of rmMe
		//and find the minimum node
		Node *iter = rmMe->child;
		Node *min = rmMe->child;
		do {
			iter->parent = nullptr;
			iter->childCut = false;
			if (iter->data < min->data){
				min = iter;
			}
			iter = iter->rsibling;
		} while (iter != rmMe->child);
		//meld the children list with top list
		meld_list(iter, minNode);
		//update min pointer if necessary
		if (min->data < minNode) {
			minNode = min;
		}
		//delete rmMe
		delete rmMe;
		//decrement number of items
		--numItem;
		//cascading cut from the node to root
		cascading_cut(parent);
	}
}



template <class Type> void 
FibonacciHeap <Type> :: decrease_key (Node *decMe, Type amount){
	if (nullptr == decMe) {
		std::cerr << "ERROR: Attempt to decrease non-exist node\n";
		exit(1);
	}
	//decrease the key of decMe by amount
	decMe->data -= amount;
	//if decrease the root, nothing to do
	if (decMe->parent == nullptr) {
		//update min pointer if necessary
		if (decMe->data < minNode) {
			minNode = decMe;
		}
		return;
	}
	//if the key of the node less than parent, cut the subtree root in the node
	if (decMe->data < decMe->parent->data) {
		Node *parent = decMe->parent;
		//cut the subtree rooted in the node
		cut_subtree(decMe);
		//meld the cut tree into top list
		meld_list(decMe, minNode);
		//update min pointer if necessary
		if (decMe->data < minNode->data) {
			minNode = decMe;
		}
		//cascading cut from the node to root
		cascading_cut(parent);
	}
}

template <class Type> int
FibonacciHeap <Type> :: size () {
	return numItem;
}



//insert into doubly linked list
template <class Type>
void FibonacciHeap <Type> :: list_insert(Node *insertMe, Node *besideMe) {
	insertMe->rsibling = besideMe;
	insertMe->lsibling = besideMe->lsibling;
	besideMe->lsibling->rsibling = insertMe;
	besideMe->lsibling = insertMe;
}

//insert into doubly linked list
template <class Type>
void FibonacciHeap <Type> :: list_remove(Node *eraseMe) {
	eraseMe->lsibling->rsibling = eraseMe->rsibling;
	eraseMe->rsibling->lsibling = eraseMe->lsibling;
}


//meld two doubly linked list
template <class Type>
void FibonacciHeap <Type> :: meld_list (Node *firstListNode, Node *secondListNode) {
	firstListNode->rsibling->lsibling = secondListNode->lsibling;
	secondListNode->lsibling->rsibling = firstListNode->rsibling;
	firstListNode->rsibling = secondListNode;
	secondListNode->lsibling = firstListNode;
}

//cut the subtree rooted in rootIn
template <class Type>
void FibonacciHeap <Type> :: cut_subtree(Node *rootIn){
	rootIn->lsibling->rsibling = rootIn->rsibling;
	rootIn->rsibling->lsibling = rootIn->lsibling;
	rootIn->lsibling = rootIn;
	rootIn->rsibling = rootIn;
	rootIn->parent->degree -= 1;
	rootIn->parent = nullptr;
	rootIn->childCut = false;
}

//cascading cut when performing remove and decrease key
template <class Type>
void FibonacciHeap <Type> :: cascading_cut(Node *curNode){
	//from curNode to root, if childCut=true, cut the node
	while(curNode->childCut == true && curNode->parent != nullptr){
		Node *parent = curNode->parent;
		cut_subtree(curNode);
		meld_list(curNode, minNode);
		if (curNode->data < minNode->data) {
			minNode = curNode;
		}
		curNode = parent;
	}
	//if current node is not the root
	if (curNode->parent != nullptr) {
		curNode->childCut = true;
	}	
}

//find the second minimal node in the heap
template <class Type>
void FibonacciHeap <Type> :: find_min(Node* aNode){
	Node *iter = aNode;
	minNode = aNode;
	do {
		if (iter->data < minNode->data) {
			minNode = iter;
		}
		iter = iter->rsibling;
	} while (iter != aNode);
}

//join two min tree when performing remove min
template <class Type>
void FibonacciHeap <Type> :: join_min_trees(Node* &root1, Node* &root2){
	if (nullptr == root1 || nullptr == root2) {
		std::cerr << "ERROR: Join non-exist min tree in Pairwise Combine\n";
		exit(1);
	}
	Node *prevTree = root2->lsibling;
	Node *nextTree = root1->rsibling;
	//root1 will be the new root
	if (root1->data <= root2->data) {
		//root1 become root2's parent
		root2->parent = root1;
		//insert root2 into root1's children list
		if (root1->degree != 0) {
			list_insert(root2, root1->child);
		} else {
			root2->lsibling = root2;
			root2->rsibling = root2;
		}	
		//root2 become root1's child
		root1->child = root2;
		//increment degree of root1
		root1->degree += 1;
		//update the top linked list
		root1->lsibling = prevTree;
		prevTree->rsibling = root1;
	} 
	//root2 will be the new root
	else {
		//root2 become root1's parent
		root1->parent = root2;
		//insert root1 into root2's children list
		if (root2->degree != 0) {
			list_insert(root1, root2->child);
		} else {
			root1->lsibling = root1;
			root1->rsibling = root1;
		}
		//root1 become root2's child
		root2->child = root1;
		//increment degree of root2
		root2->degree += 1;
		//update the top linked list
		root2->rsibling = nextTree;
		nextTree->lsibling = root2;
		root1 = root2;
	}
}

template <class Type>
void FibonacciHeap <Type> :: pairwise_combine(){
	//the max degree of trees in the heap is log2(n)
	unsigned int max_degree = log2(numItem) + 1;
	//create the degree table
	std::vector<Node*> tree(max_degree, nullptr);
	//traverse all trees in the heap
	Node *p = minNode;
	int degree;
	do {
		//if there are two trees with the same degree in the heap
		for (degree = p->degree; tree[degree] != nullptr; degree++) {
			//join two trees
			join_min_trees(p, tree[degree]);
			//update tree table
			tree[degree] = nullptr;
		}
		tree[degree] = p;
		p = p->rsibling;	
	} while (p != minNode);
}

