The program is compiled with Makefile.
The program uses stdout to show stuff to the user.
The program uses stdin to read from the user input.


Project conventions:

- private module functions are first in the source file


libkv impl:


struct item {
	char* key;
	void* value;	
	func() dtor;
	item* next;
}

struct store {	
	item* first_item;
}

store initial_store;

The initial_store is the store for stores.
When a store is allocated, a pointer to it is saved in the initial_store
When kv_flush is called, the initial_store is cleared and so all the stores are cleared.

