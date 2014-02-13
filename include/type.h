#ifndef HEADER_TYPE
#define HEADER_TYPE

#define TYPE_LAMBDA 1
#define TYPE_CLASS 2
#define TYPE_MATCHALL 3
#define TYPE_NOTHING 4

struct TypeArray;

typedef struct Type {
	int type;							// 1 lambda 2 class
	union {
		struct {
			struct TypeArray* arguments;// the args nbd
			struct Type* returntype;	// returns an X
			struct Node* body;			// parse tree of body
		} lambda;
		struct {
			char* classname;			// Bongo
			int shadow;					// number of $s
		} _class;
	} typedata;
	int arrayed;						// number of []
	char* alias;						// this should be moved....
	char* specialty;					// {Alive}
	int optional;
} Type;

typedef struct TypeArray {
	Type** types;
	int typecount;
} TypeArray;

Type* MakeType(int type);
TypeArray* MakeTypeArray();
void AddTypeToTypeArray(Type* nexttype, TypeArray* container);
void freeType(Type* t);
void freeTypeArray(TypeArray* ta);
Type* copyType(Type* t);
TypeArray* copyTypeArray(TypeArray* ta);

#endif