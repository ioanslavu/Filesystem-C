#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "tree.h"
#define TREE_CMD_INDENT_SIZE 4
#define NO_ARG ""
#define PARENT_DIR ".."

FileTree createFileTree(char *rootFolderName)
{
    FileTree fileTree;

    // allocate memory for the root folder
    fileTree.root = (TreeNode *)malloc(sizeof(TreeNode));
    DIE(!fileTree.root, "malloc");

    // set the root folder props
    fileTree.root->parent = NULL;
    fileTree.root->name = rootFolderName;
    fileTree.root->type = FOLDER_NODE;

    // allocate memory for the root folder's content
    fileTree.root->content = (FolderContent *)malloc(sizeof(FolderContent));
    DIE(!fileTree.root->content, "malloc");

    // set the root folder's content list
    ((FolderContent *)fileTree.root->content)->children = ll_create();
    return fileTree;
}

void freeTree(FileTree fileTree)
{
    // free the root folder
    freeNode(fileTree.root);
}

void freeNode(TreeNode *treeNode)
{
    // if the node is a file, free its content
    if (treeNode->type == FILE_NODE)
    {
        free(((FileContent *)treeNode->content)->text);
        free(treeNode->content);
        free(treeNode->name);
        free(treeNode);
        return;
    }

    // iterate over the folder's content
    FolderContent *folderContent = treeNode->content;
    ListNode *listNode = folderContent->children->head;
    while (listNode != NULL)
    {
        // if the node is a folder, recursively free it
        if (listNode->info->type == FOLDER_NODE)
        {
            freeNode(listNode->info);
        }
        // free the file content
        else
        {
            free(((FileContent *)listNode->info->content)->text);
            free(listNode->info->content);
            free(listNode->info->name);
            free(listNode->info);
        }

        listNode = listNode->next;
    }
    // free the folder's content list
    ll_free(&folderContent->children);
    // free the folder's props
    free(treeNode->content);
    free(treeNode->name);
    free(treeNode);
}

void ls(TreeNode *currentNode, char *arg)
{
    // check if the arg is empty, if so, print the current folder's content
    if (!strcmp(arg, NO_ARG))
    {
        FolderContent *folderContent = currentNode->content;
        ListNode *listNode = folderContent->children->head;
        while (listNode != NULL)
        {
            printf("%s\n", listNode->info->name);
            listNode = listNode->next;
        }
    }
    // if the arg is not empty, print the content of the arg folder/file
    else
    {
        // check if the arg exists
        TreeNode *treeNode = fileExist(currentNode, arg);
        if (treeNode == NULL)
            printf("ls: cannot access '%s': No such file or directory\n", arg);
        else
        {
            // if the arg is a folder, print the content of the folder
            if (treeNode->type == FOLDER_NODE)
            {
                FolderContent *folderContent = treeNode->content;
                ListNode *listNode = folderContent->children->head;
                while (listNode != NULL)
                {
                    printf("%s\n", listNode->info->name);
                    listNode = listNode->next;
                }
            }
            // if the arg is a file, print the content of the file
            else
                printf("%s: %s\n", treeNode->name,
                       ((FileContent *)(treeNode->content))->text);
        }
    }
}

void pwd(TreeNode *treeNode)
{
    // if the current node is the root folder, print the root folder's name
    if (treeNode->parent == NULL)
        printf("%s", treeNode->name);
    // if the current node is not the root folder,
    // print the path of the current node
    else
    {
        pwd(treeNode->parent);
        printf("/%s", treeNode->name);
    }
}

TreeNode *cd(TreeNode *currentNode, char *path)
{
    if (!strcmp(path, NO_ARG))
        return currentNode;

    // if we can't go to a node, we save the current node in which we are
    TreeNode *copyCurrentNode = currentNode;

    // we split the path in order to get the path of the node we want to go to
    char *stripPath = strtok(path, "/");
    int found = 0;
    while (stripPath != NULL)
    {
        // we check if the node we want to go to is a parent node
        if (!strcmp(stripPath, PARENT_DIR))
        {
            // if we are in the root node, we don't do that here
            if (currentNode->parent == NULL)
            {
                printf("cd: no such file or directory: %s", path);
                return copyCurrentNode;
            }
            else
                currentNode = currentNode->parent;
        }
        else
        {
            found = 0;
            // we search for the content from the current node
            FolderContent *folderContent = currentNode->content;
            ListNode *listNode = folderContent->children->head;
            while (listNode != NULL)
            {
                // if we find the node we want to go to, we store it
                if (strcmp(listNode->info->name, stripPath) == 0)
                {
                    currentNode = listNode->info;
                    found = 1;
                    break;
                }
                listNode = listNode->next;
            }
            // if we didn't find the node we want to go to
            // we print an error message
            if (!found)
            {
                printf("cd: no such file or directory: %s", path);
                return copyCurrentNode;
            }
        }
        // we get the next path
        stripPath = strtok(NULL, "/");
    }
    // we return the node we want to go to
    return currentNode;
}

void tree(TreeNode *currentNode, char *arg)
{
    // if the arg is empty, print the current folder's content
    // else print the content of the arg folder
    char *stripPath = strtok(arg, "/");
    int found = 0;
    while (stripPath != NULL)
    {
        // we check if the node we want to go to is a parent node
        if (!strcmp(stripPath, PARENT_DIR))
        {
            // if we are in the root node, we don't go to the parent node
            if (currentNode->parent == NULL)
                break;
            else
                currentNode = currentNode->parent;
        }
        else
        {
            found = 0;
            // we search for the content from the current node
            FolderContent *folderContent = currentNode->content;
            ListNode *listNode = folderContent->children->head;
            while (listNode != NULL)
            {
                // if we find the node we want to go to, we store it
                if (strcmp(listNode->info->name, stripPath) == 0)
                {
                    currentNode = listNode->info;
                    found = 1;
                    break;
                }
                listNode = listNode->next;
            }
            // if we didn't find the node we want to go to
            if (!found)
            {
                printf("%s [error opening dir]\n\n0 directories, 0 files\n",
                       arg);
                return;
            }
        }
        // we get the next path
        stripPath = strtok(NULL, "/");
    }

    // we check if the source node is a file
    if (currentNode->type == FILE_NODE)
    {
        printf("%s [error opening dir]\n\n0 directories, 0 files\n", arg);
        return;
    }

    TreeNode *parrent = currentNode->parent;
    FolderContent *folderContent;
    ListNode *listNode;

    int noDirectories = 0;
    int noFiles = 0;
    int i = 0;
    int back = 0;

    char *name;
    // iterate through the current node's content
    while (currentNode != parrent)
    {
        folderContent = currentNode->content;
        listNode = folderContent->children->head;
        // when we return in parent folder
        if (back)
        {
            while (strcmp(listNode->info->name, name) != 0 && listNode != NULL)
                listNode = listNode->next;
            if (listNode != NULL)
                listNode = listNode->next;
            back = 0;
        }
        // we search in a folder all the files and folders
        while (listNode != NULL)
        {
            // if the node is a folder, we print the folder's name
            // we increment the number of directories
            // and we go to the folder
            if (listNode->info->type == FOLDER_NODE)
            {
                printf("%*s%s\n", i * TREE_CMD_INDENT_SIZE, "",
                       listNode->info->name);
                noDirectories++;
                i++;
                currentNode = listNode->info;

                break;
            }
            // if the node is a file, we print the file's name
            // we increment the number of files
            else
            {
                printf("%*s%s\n", i * TREE_CMD_INDENT_SIZE, "",
                       listNode->info->name);
                noFiles++;
            }
            listNode = listNode->next;
        }

        // if we are at the end of a folders content
        // we return to the parent folder
        if (listNode == NULL)
        {
            name = currentNode->name;
            back = 1;
            currentNode = currentNode->parent;
            i--;
        }
    }
    printf("\n%d directories, %d files\n", noDirectories, noFiles);
}

void mkdir(TreeNode *currentNode, char *folderName)
{
    // check if the folder already exists
    TreeNode *treeNode = fileExist(currentNode, folderName);

    // if the folder already exists, we print an error message
    if (treeNode != NULL)
    {
        printf("mkdir: cannot create directory '%s': File exists\n",
               folderName);
        return;
    }

    // if the folder doesn't exist, we create it
    treeNode = (TreeNode *)malloc(sizeof(TreeNode));
    DIE(!treeNode, "malloc");

    // set the folder's props
    treeNode->parent = currentNode;
    treeNode->name = folderName;
    treeNode->type = FOLDER_NODE;
    // allocate memory for the folder's content
    treeNode->content = (FolderContent *)malloc(sizeof(FolderContent));
    DIE(!treeNode->content, "malloc");

    // set the folder's content list
    ((FolderContent *)treeNode->content)->children = ll_create();

    // add the folder to the current node's content list
    ll_add_node(((FolderContent *)currentNode->content)->children, treeNode);
    // let it be free, we just copied it
    free(treeNode);
}

void rmrec(TreeNode *currentNode, char *resourceName)
{
    // check if the resource exists
    TreeNode *treeNode = fileExist(currentNode, resourceName);

    // if the resource doesn't exist, we print an error message
    if (treeNode == NULL)
    {
        printf("rmrec: failed to remove '%s': No such file or directory\n",
               resourceName);
        return;
    }

    // we remove the file from the current node's content list
    FolderContent *folderContent = currentNode->content;
    ListNode *removed = ll_remove_node(folderContent->children, resourceName);
    // if the resource is a folder, we remove it
    freeNode(removed->info);
    free(removed);
}

void rm(TreeNode *currentNode, char *fileName)
{
    // check if the file exists
    TreeNode *treeNode = fileExist(currentNode, fileName);

    // if the file doesn't exist, we print an error message
    if (treeNode == NULL)
    {
        printf("rm: failed to remove '%s': No such file or directory\n",
               fileName);
        return;
    }

    // if the file is a folder, we print an error message
    if (treeNode->type == FOLDER_NODE)
    {
        printf("rm: cannot remove '%s': Is a directory\n", fileName);
        return;
    }

    // if the resource is a file, we remove it
    FolderContent *folderContent = currentNode->content;
    ListNode *removed = ll_remove_node(folderContent->children, fileName);
    free(treeNode->content);
    free(treeNode);
    free(removed);
}

void rmdir(TreeNode *currentNode, char *folderName)
{
    // verify if the folder exists
    TreeNode *treeNode = fileExist(currentNode, folderName);
    // if the folder doesn't exist, we print an error message
    if (treeNode == NULL)
    {
        printf("rmdir: failed to remove '%s': No such file or directory\n",
               folderName);
        return;
    }
    // if the folder is not empty, we print an error message
    if (treeNode->type == FOLDER_NODE)
    {
        FolderContent *folderContent = treeNode->content;
        if (folderContent->children->head != NULL)
        {
            printf("rmdir: failed to remove '%s': Directory not empty\n",
                   folderName);
            return;
        }
    }
    // if the folder is a file, we print an error message
    else
    {
        printf("rmdir: failed to remove '%s': Not a directory\n", folderName);
        return;
    }
    // if the folder is empty, we remove it and free the memory
    FolderContent *folderContent = currentNode->content;
    ListNode *removed = ll_remove_node(folderContent->children, folderName);
    free(treeNode->content);
    free(treeNode);
    free(removed);
}

void touch(TreeNode *currentNode, char *fileName, char *fileContent)
{
    // verify if file exists
    TreeNode *treeNode = fileExist(currentNode, fileName);
    if (treeNode != NULL)
        return;

    // create file
    treeNode = (TreeNode *)malloc(sizeof(TreeNode));
    DIE(!treeNode, "malloc");

    // set file's props
    treeNode->parent = currentNode;
    treeNode->name = fileName;
    treeNode->type = FILE_NODE;
    treeNode->content = (FileContent *)malloc(sizeof(FileContent));
    DIE(!treeNode->content, "malloc");

    FileContent *nodeContent = treeNode->content;
    // set file's content, if content exists
    if (strcmp(fileContent, NO_ARG) == 0)
        nodeContent->text = NULL;
    else
    {
        // allocate memory for the file's content
        nodeContent->text = fileContent;
    }
    // add the file to the current node's content list
    ll_add_node(((FolderContent *)currentNode->content)->children, treeNode);
    // let it be free, we just copied it
    free(treeNode);
}

void cp(TreeNode *currentNode, char *source, char *destination)
{
    TreeNode *destinationNode = currentNode;
    TreeNode *sourceNode = currentNode;
    // check if the source exists and get the source node
    char *stripSourcePath = strtok(source, "/");
    while (stripSourcePath != NULL)
    {
        // we check if the node we want to go to is a parent node
        if (!strcmp(stripSourcePath, PARENT_DIR))
        {
            // if we are in the root node, we don't go to the parent node
            if (sourceNode->parent == NULL)
                break;
            else
                sourceNode = sourceNode->parent;
        }
        else
        {
            // we search for the content from the current node
            FolderContent *folderContent = sourceNode->content;
            ListNode *listNode = folderContent->children->head;
            while (listNode != NULL)
            {
                // if we find the node we want to go to, we store it
                if (strcmp(listNode->info->name, stripSourcePath) == 0)
                {
                    sourceNode = listNode->info;
                    break;
                }
                listNode = listNode->next;
            }
        }
        // we get the next path
        stripSourcePath = strtok(NULL, "/");
    }

    // we check if the source node is a folder
    if (sourceNode->type == FOLDER_NODE)
    {
        printf("cp: -r not specified; omitting directory '%s'\n", source);
        return;
    }

    // copy destination path
    char *copyDestination = malloc(strlen(destination) + 1);
    DIE(!copyDestination, "malloc");
    memcpy(copyDestination, destination, strlen(destination) + 1);
    // verify if we can acces the destination folder
    char *stripDestinationPath = strtok(destination, "/");
    char *fileDestinationName;
    int found = 0;
    while (stripDestinationPath != NULL)
    {
        // we check if the node we want to go to is a parent node
        if (!strcmp(stripDestinationPath, PARENT_DIR))
        {
            // if we are in the root node, we don't go to the parent node
            if (destinationNode->parent == NULL)
            {
                printf("cp: failed to access '%s': Not a directory\n",
                       copyDestination);
                free(copyDestination);
                return;
            }
            else
                destinationNode = destinationNode->parent;
            stripDestinationPath = strtok(NULL, "/");
        }
        else
        {
            found = 0;
            // we search for the content from the current node
            FolderContent *folderContent = destinationNode->content;
            ListNode *listNode = folderContent->children->head;
            while (listNode != NULL)
            {
                // if we find the node we want to go to, we store it
                if (strcmp(listNode->info->name, stripDestinationPath) == 0)
                {
                    destinationNode = listNode->info;
                    found = 1;
                    break;
                }
                listNode = listNode->next;
            }
            fileDestinationName = stripDestinationPath;
            // we get the next path
            stripDestinationPath = strtok(NULL, "/");
            // if we didn't find the node we want to go to
            // we print an error message
            if (!found && stripDestinationPath != NULL)
            {
                printf("cp: failed to access '%s': Not a directory\n",
                       copyDestination);
                free(copyDestination);
                return;
            }
        }
    }
    free(copyDestination);
    // if the file exists, we update it with the source content
    // else we create a new file

    // verify if destination node is a folder
    if (destinationNode->type == FOLDER_NODE)
    {
        TreeNode *file = fileExist(destinationNode, sourceNode->name);
        if (file)
        {
            // update the file
            FileContent *destinationContent = destinationNode->content;
            FileContent *sourceContent = sourceNode->content;
            free(destinationContent->text);
            destinationContent->text = malloc(strlen(sourceContent->text) + 1);
            DIE(!destinationContent->text, "malloc");
            memcpy(destinationContent->text, sourceContent->text,
                   strlen(sourceContent->text) + 1);
            return;
        }
        else
        {
            // create a new file
            FileContent *fileContent = sourceNode->content;
            // if the source file has content, we copy it
            if (fileContent->text == NULL)
                touch(destinationNode, strdup(sourceNode->name),
                      strdup(NO_ARG));
            else
                touch(destinationNode, strdup(sourceNode->name),
                      strdup(fileContent->text));
            return;
        }
    }
    if (!found)
    {
        // destination node is a not existing file
        // we copy the file
        FileContent *fileContent = sourceNode->content;
        if (fileContent->text == NULL)
            touch(destinationNode, strdup(fileDestinationName), strdup(NO_ARG));
        else
            touch(destinationNode, strdup(fileDestinationName),
                  strdup(fileContent->text));
        return;
    }
    else
    {
        // update the file
        FileContent *destinationContent = destinationNode->content;
        FileContent *sourceContent = sourceNode->content;
        // clear destination content
        free(destinationContent->text);
        destinationContent->text = malloc(strlen(sourceContent->text) + 1);
        DIE(!destinationContent->text, "malloc");
        // copy the source content
        memcpy(destinationContent->text, sourceContent->text,
               strlen(sourceContent->text) + 1);
        return;
    }
}

void mv(TreeNode *currentNode, char *source, char *destination)
{
    TreeNode *destinationNode = currentNode;
    TreeNode *sourceNode = currentNode;

    // check if the source exists and get the source node
    char *stripSourcePath = strtok(source, "/");
    while (stripSourcePath != NULL)
    {
        // we check if the node we want to go to is a parent node
        if (!strcmp(stripSourcePath, PARENT_DIR))
        {
            // if we are in the root node, we don't go to the parent node
            if (sourceNode->parent == NULL)
                break;
            else
                sourceNode = sourceNode->parent;
        }
        else
        {
            // we search for the content from the current node
            FolderContent *folderContent = sourceNode->content;
            ListNode *listNode = folderContent->children->head;
            while (listNode != NULL)
            {
                // if we find the node we want to go to, we store it
                if (strcmp(listNode->info->name, stripSourcePath) == 0)
                {
                    sourceNode = listNode->info;
                    break;
                }
                listNode = listNode->next;
            }
        }
        // we get the next path
        stripSourcePath = strtok(NULL, "/");
    }

    // copy destination path
    char *copyDestination = malloc(strlen(destination) + 1);
    DIE(!copyDestination, "malloc");
    memcpy(copyDestination, destination, strlen(destination) + 1);
    // verify if we can acces the destination folder
    char *stripDestinationPath = strtok(destination, "/");
    int found = 0;
    while (stripDestinationPath != NULL)
    {
        // we check if the node we want to go to is a parent node
        if (!strcmp(stripDestinationPath, PARENT_DIR))
        {
            // if we are in the root node, we don't go to the parent node
            if (destinationNode->parent == NULL)
            {
                printf("mv: failed to access '%s': Not a directory\n",
                       copyDestination);
                free(copyDestination);
                return;
            }
            else
                destinationNode = destinationNode->parent;
            stripDestinationPath = strtok(NULL, "/");
        }
        else
        {
            found = 0;
            // we search for the content from the current node
            FolderContent *folderContent = destinationNode->content;
            ListNode *listNode = folderContent->children->head;
            while (listNode != NULL)
            {
                // if we find the node we want to go to, we store it
                if (strcmp(listNode->info->name, stripDestinationPath) == 0)
                {
                    destinationNode = listNode->info;
                    found = 1;
                    break;
                }
                listNode = listNode->next;
            }
            stripDestinationPath = strtok(NULL, "/");
            // if we didn't find the node we want to go to
            // we print an error message
            if (!found && stripDestinationPath != NULL)
            {
                printf("mv: failed to access '%s': Not a directory\n",
                       copyDestination);
                free(copyDestination);
                return;
            }
        }
        // we get the next path

        // verify if destination node is null
    }
    free(copyDestination);
    // verify if destination node is a folder
    if (destinationNode->type == FOLDER_NODE)
    {
        TreeNode *file = fileExist(destinationNode, sourceNode->name);
        // if the file exists, we delete it
        if (file)
        {
            freeNode(file);
            FolderContent *destContent = destinationNode->parent->content;
            ListNode *removed = ll_remove_node(destContent->children,
                                               sourceNode->name);
            free(removed);
        }
        // we link the source node to the destination node
        FolderContent *folderContent = sourceNode->parent->content;
        ListNode *new = ll_remove_node(folderContent->children,
                                       sourceNode->name);

        FolderContent *destinationContent = destinationNode->content;

        ListNode *temp = destinationContent->children->head;
        destinationContent->children->head = new;
        new->next = temp;
        sourceNode->parent = destinationNode;
        return;
    }
    if (!found)
    {
        // remove from the source folder
        FolderContent *folderContent = sourceNode->parent->content;
        ListNode *new = ll_remove_node(folderContent->children,
                                       sourceNode->name);

        FolderContent *destinationContent = destinationNode->parent->content;
        // link it to the destination folder
        ListNode *temp = destinationContent->children->head;
        destinationContent->children->head = new;
        new->next = temp;
        sourceNode->parent = destinationNode;
        return;
    }
    else
    {
        // update the file
        FileContent *destinationContent = destinationNode->content;
        FileContent *sourceContent = sourceNode->content;
        // clear destination content
        free(destinationContent->text);
        destinationContent->text = malloc(strlen(sourceContent->text) + 1);
        DIE(!destinationContent->text, "malloc");
        // copy the source content
        memcpy(destinationContent->text, sourceContent->text,
               strlen(sourceContent->text) + 1);
        // remove from the source folder
        FolderContent *folderContent = sourceNode->parent->content;
        ListNode *new = ll_remove_node(folderContent->children,
                                       sourceNode->name);
        freeNode(sourceNode);
        free(new);
        return;
    }
}

TreeNode *fileExist(TreeNode *currentNode, char *fileName)
{
    // we search for the file from the current node
    FolderContent *folderContent = (FolderContent *)currentNode->content;
    ListNode *listNode = folderContent->children->head;
    while (listNode != NULL)
    {
        if (strcmp(listNode->info->name, fileName) == 0)
            return listNode->info;
        listNode = listNode->next;
    }
    return NULL;
}

List *
ll_create()
{
    List *list;

    list = (List *)malloc(sizeof(List));
    DIE(!list, "malloc");

    list->head = NULL;

    return list;
}

void ll_add_node(List *list, const void *new_data)
{
    ListNode *new_node, *curr;

    if (list == NULL)
        return;

    curr = list->head;

    new_node = malloc(sizeof(ListNode));
    DIE(!new_node, "malloc");
    new_node->info = (TreeNode *)malloc(sizeof(TreeNode));
    DIE(!new_node->info, "malloc");
    memcpy(new_node->info, new_data, sizeof(TreeNode));
    list->head = new_node;
    new_node->next = curr;
}

ListNode *
ll_remove_node(List *list, const void *data)
{
    ListNode *curr, *prev;

    if (list == NULL)
        return NULL;

    if (list->head == NULL)
        return NULL;

    curr = list->head;
    prev = NULL;
    while (curr != NULL)
    {
        if (strcmp(curr->info->name, data) == 0)
        {
            if (prev == NULL)
                list->head = curr->next;
            else
                prev->next = curr->next;
            return curr;
        }
        prev = curr;
        curr = curr->next;
    }

    return NULL;
}

void ll_free(List **list)
{
    ListNode *curr, *next;

    if (list == NULL || *list == NULL)
        return;

    curr = (*list)->head;
    while (curr != NULL)
    {
        next = curr->next;
        free(curr);
        curr = next;
    }
    free(*list);
    *list = NULL;
}
