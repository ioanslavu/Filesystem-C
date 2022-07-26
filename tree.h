#define TREE_CMD_INDENT_SIZE 4
#define NO_ARG ""
#define PARENT_DIR ".."

typedef struct FileContent FileContent;
typedef struct FolderContent FolderContent;
typedef struct TreeNode TreeNode;
typedef struct FileTree FileTree;
typedef struct ListNode ListNode;
typedef struct List List;

enum TreeNodeType {
    FILE_NODE,
    FOLDER_NODE
};

struct FileContent {
    char* text;
};

struct FolderContent {
    List* children;
};

struct TreeNode {
    TreeNode* parent;
    char* name;
    enum TreeNodeType type;
    void* content;
};

struct FileTree {
    TreeNode* root;
};

struct ListNode {
    TreeNode* info;
    ListNode* next;
};

struct List {
    ListNode* head;
};


void ls(TreeNode* currentNode, char* arg);
void pwd(TreeNode* treeNode);
TreeNode* cd(TreeNode* currentNode, char* path);
void tree(TreeNode* currentNode, char* arg);
void mkdir(TreeNode* currentNode, char* folderName);
void rm(TreeNode* currentNode, char* fileName);
void rmdir(TreeNode* currentNode, char* folderName);
void rmrec(TreeNode* currentNode, char* resourceName);
void touch(TreeNode* currentNode, char* fileName, char* fileContent);
void cp(TreeNode* currentNode, char* source, char* destination);
void mv(TreeNode* currentNode, char* source, char* destination);
FileTree createFileTree();
void freeTree(FileTree fileTree);
void freeNode(TreeNode *treeNode);
TreeNode *fileExist(TreeNode *currentNode, char *fileName);
List* ll_create();
void ll_add_node(List* list, const void* new_data);
ListNode* ll_remove_node(List* list, const void* data);
void ll_free(List **list);

#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(errno);				        \
		}							\
	} while (0)
