#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#define order 4 
//4 values
// therefore, we have the capacity to store 6 pointers and 5 values, but we will store only 4 data values and 5 pointers

typedef union pointerType_tag
{
    struct bPlusNode *nodeptr[order + 2];
    struct flightNode_tag *dataptr[order + 2];
} pointerType;

typedef struct Time
{
    int Hour;
    int Min;
} Time;

typedef struct Flight_tag
{
    int flightID;
    Time departureTime;
    Time ETA;
    struct Flight_tag *next;
    struct Flight_tag *prev;
} Flight;

typedef struct bPlusNode
{
    Time key[order+1];
    int isLeaf;
    pointerType children;
    int activeKeys;
    struct bPlusNode *parent;
} BPlusTreeNode;

typedef struct flightNode_tag
{
    Flight *lptr;
    struct flightNode_tag *prev;
    struct flightNode_tag *next;
    int size;
} DataNode;

typedef struct Bucket_tag
{
    struct Bucket_tag *next;
    Time beginningETA;
    Time endETA;
    BPlusTreeNode *root;
    int isLeaf;
} Bucket;

int BucketId = 1000;

int maxTime(Time t1, Time t2) // ret_val=1 indicates t1 < t2 and 0 indicates t1 = t2 and -1 indicates t1 > t2
{
    int ret_val = 1;
    if (t1.Hour > t2.Hour)
        ret_val = -1;
    else if (t1.Hour < t2.Hour)
        ret_val = 1;
    else // same hour
    {
        if (t1.Min > t2.Min)
            ret_val = -1;
        else if (t1.Min < t2.Min)
            ret_val = 1;
        else // same min
        {
            ret_val = 0;
        }
    }
    return ret_val;
}

Flight *insertInPlaneList(Flight *head, Flight *newNode) // inserting in ascending order
{
    if (head == NULL)
        head = newNode;

    else if (maxTime(head->departureTime, newNode->departureTime) == -1) // Insert at start
    {
        newNode->next = head;
        head->prev = newNode;
        head = newNode;
    }

    else // General Case
    {
        Flight *ptr, *prev_ptr;
        ptr = head;

        while (ptr != NULL && maxTime(ptr->departureTime, newNode->departureTime) >= 0)
        {
            prev_ptr = ptr;
            ptr = ptr->next;
        }

        if (ptr == NULL) // Insert at end
        {
            prev_ptr->next = newNode;
            newNode->prev = prev_ptr;
            newNode->next = NULL;
        }

        else
        {
            newNode->next = ptr;
            ptr->prev = newNode;
            prev_ptr->next = newNode;
            newNode->prev = prev_ptr;
        }
    }

    return head;
}

void visit(Flight *ptr)
{
    while (ptr != NULL)
    {
        printf("\n\nFlight ID is %d\n", ptr->flightID);
        printf("Flight Departure = %d:%d\n\n", ptr->departureTime.Hour, ptr->departureTime.Min);
        ptr = ptr->next;
    }
}

Flight *createNode(int ID, Time departureTime, Time ETA)
{
    Flight *nptr;
    nptr = (Flight *)malloc(sizeof(Flight));

    nptr->flightID = ID;
    nptr->next = NULL;
    nptr->prev = NULL;

    nptr->departureTime.Hour = departureTime.Hour;
    nptr->departureTime.Min = departureTime.Min;

    nptr->ETA.Hour = ETA.Hour;
    nptr->ETA.Min = ETA.Min;

    return nptr;
}

DataNode *createDataNode()
{
    DataNode *newNode;
    newNode = (DataNode *)malloc(sizeof(DataNode));
    newNode->lptr = NULL;
    newNode->next = NULL;
    newNode->prev = NULL;
    newNode->size = 0;
    return newNode;
}

BPlusTreeNode *createTreeNode()
{
    BPlusTreeNode *newNode; // new BPlusTreeNode
    newNode = (BPlusTreeNode *)malloc(sizeof(BPlusTreeNode));
    newNode->activeKeys = 0;
    newNode->isLeaf = 0;
    for (int i = 0; i < order; i++)
        newNode->children.dataptr[i] = NULL;
    return newNode;
}

void printTree(BPlusTreeNode *root)
{
    int i = 0;
    printf ("Keys\n");
    while (i < root->activeKeys)
    {
        printf ("%d:%d ", root->key[i].Hour, root->key[i].Min);
        i++;
    }
    printf ("\n");
    if (root->isLeaf == 1)
    {
        DataNode *data = root->children.dataptr[0];
        while (data != NULL)
        {
            Flight *pointer = data->lptr;
            while (pointer != NULL)
            {
                printf ("%d:%d ", pointer->departureTime.Hour, pointer->departureTime.Min);
                pointer = pointer->next;
            }
            printf ("\n");
            data = data->next;
        }
    }
    else
    {
        i = 0;
        printf ("%d", root->activeKeys);
        while (i <= root->activeKeys)
        {
            if (root->children.nodeptr[i] != NULL) 
            {
                printTree(root->children.nodeptr[i]);
            }
            i++;
        }
    }
}

BPlusTreeNode *findParent(BPlusTreeNode* start, BPlusTreeNode *root)
{
    if (start != NULL)
    {
        for (int i = 0; i < start->activeKeys; i++)
        {
            if (start->children.nodeptr[i] == root) return root;
            else
            {
                BPlusTreeNode * ans = findParent(start->children.nodeptr[i], root);
                if (ans != NULL) return ans;
            }
        }
    }
    return NULL;
}

BPlusTreeNode *splitBPlusTreeNode(BPlusTreeNode* Actualroot, BPlusTreeNode *root)//return parent
{
    if (root->isLeaf == 1) //we will be distributing data nodes only
    {
        int i;

        BPlusTreeNode *newNode = createTreeNode();
        newNode->activeKeys = order/2;
        newNode->isLeaf = 1;

        for (i = order/2 + 1; i <= order; i++)
        {
            newNode->children.dataptr[i-order/2 -1] = root->children.dataptr[i];
            newNode->key[i-order/2 -1] = root->key[i];
            root->children.dataptr[i] = NULL;
        }

        newNode->children.dataptr[i-order/2-1] = root->children.dataptr[i];

        newNode->activeKeys = root->activeKeys = order/2;
        root->children.dataptr[order/2]->next = newNode->children.dataptr[0];
        newNode->children.dataptr[0]->prev = root->children.dataptr[order/2];

        if (root->parent == NULL)//Root Node
        {
            BPlusTreeNode *newRoot = createTreeNode();
            newNode->parent = root->parent = newRoot;
            newRoot->key[0] = root->key[order/2];

            newRoot->isLeaf = 0;
            newRoot->children.nodeptr[0] = root;
            newRoot->children.nodeptr[1] = newNode;
            newRoot->parent = NULL;
            newRoot->activeKeys = 1;
            return newRoot;
        }

        else
        {
            BPlusTreeNode *Parent = Actualroot;
            i = Parent->activeKeys-1;
            while (i >= 0 && maxTime(Parent->key[i], root->key[order/2]) < 0)
            {
                Parent->key[i+1] = Parent->key[i];
                Parent->children.nodeptr[i+2] = Parent->children.nodeptr[i+1];
                i--;
            }
            i++;
            Parent->key[i] = root->key[order/2];
            Parent->children.nodeptr[i] = root;
            Parent->children.nodeptr[i+1] = newNode;
            Parent->isLeaf = 0;
            Parent->activeKeys++;
            if (Parent->activeKeys == order+1)
            {
                Parent = splitBPlusTreeNode(Actualroot, Parent);
            }
            return Parent;
        }
    }
    else
    {
        int i;

        BPlusTreeNode *newNode = createTreeNode();
        newNode->activeKeys = order/2;
        newNode->isLeaf = 0;

        for (i = order/2 + 1; i <= order; i++)
        {
            newNode->children.nodeptr[i-order/2 -1] = root->children.nodeptr[i];
            newNode->key[i-order/2 -1] = root->key[i];
            root->children.nodeptr[i] = NULL;
        }

        newNode->children.nodeptr[i-order/2-1] = root->children.nodeptr[i];

        newNode->activeKeys = root->activeKeys = order/2;

        if (root->parent == NULL)//Root Node
        {
            BPlusTreeNode *newRoot = createTreeNode();
            newNode->parent = root->parent = newRoot;
            newRoot->key[0] = root->key[order/2];

            newRoot->isLeaf = 0;
            newRoot->children.nodeptr[0] = root;
            newRoot->children.nodeptr[1] = newNode;
            newRoot->parent = NULL;
            newRoot->activeKeys = 1;
            Actualroot = newRoot;
            return newRoot;
        }

        else
        {
            BPlusTreeNode *Parent = findParent(Actualroot, root);
            i = Parent->activeKeys-1;
            while (i >= 0 && maxTime(Parent->key[i], root->key[order/2]) < 0)
            {
                Parent->key[i+1] = Parent->key[i];
                Parent->children.nodeptr[i+2] = Parent->children.nodeptr[i+1];
                i--;
            }
            i++;
            Parent->key[i] = root->key[order/2];
            Parent->children.nodeptr[i] = root;
            Parent->children.nodeptr[i+1] = newNode;
            Parent->isLeaf = 0;
            Parent->activeKeys++;
            if (Parent->activeKeys == order+1)
            {
                Parent = splitBPlusTreeNode(Actualroot, Parent);
            }
            return Parent;
        }
    }
    // return root->parent;
}

BPlusTreeNode *splitDataNode(BPlusTreeNode *Actualroot, BPlusTreeNode *root, int dataNodeIndex)//flight to be inserted
{

    DataNode *temp = root->children.dataptr[0];
    Flight *ptr = NULL;

    Flight *flightPtr = root->children.dataptr[dataNodeIndex]->lptr;

    flightPtr = root->children.dataptr[dataNodeIndex]->lptr;
    for (int j = 0; j < order/2; j++)
    {
        flightPtr = flightPtr->next;
    }
    
    Flight *child = flightPtr;
    
    int i = root->activeKeys-1;

    //insert key in the BPlusTreeNode
    while (i >= 0 && maxTime(root->key[i], child->departureTime) < 0)
    {
        root->key[i+1] = root->key[i];
        root->children.dataptr[i+2] = root->children.dataptr[i+1]; 
        i--;
    }
    i++;
    root->key[i] = child->departureTime;
    root->activeKeys++;

    //now split the dataNode
    DataNode *newDataNode = createDataNode();
    newDataNode->size = 2;
    int j = 0;
    ptr = root->children.dataptr[dataNodeIndex]->lptr;
    Flight *newPtr; //to traverse newDataNode
    while (j < order/2)
    {
        ptr = ptr->next;
        j++;
    }

    Flight *prev = ptr;
    ptr = ptr->next;
    prev->next = NULL;
    root->children.dataptr[dataNodeIndex]->size = 3;

    newPtr = newDataNode->lptr = ptr;
    
    while (ptr != NULL)
    {
       newPtr->next = ptr->next;
       ptr = ptr->next;
       newPtr = newPtr->next;
    }

    newDataNode->next = root->children.dataptr[dataNodeIndex]->next;
    if (newDataNode->next != NULL)
    {
        DataNode *temp = newDataNode->next;
        temp->prev = newDataNode; 
    }
    root->children.dataptr[dataNodeIndex]->next = newDataNode;
    newDataNode->prev = root->children.dataptr[dataNodeIndex];
 
    root->children.dataptr[i+1] = newDataNode;

    if (root->activeKeys == order+1)
    {
        root = splitBPlusTreeNode(Actualroot, root);
        //should return the parent
    }
    return root;
}

BPlusTreeNode *insertInbPlusTree(BPlusTreeNode *Actualroot, BPlusTreeNode *root, Flight *newNode)//if the hight increases, return the topmost root
{
    if (root == NULL)
    {
        root = createTreeNode();
        root->isLeaf = 1;
        root->activeKeys = 1;
        for (int i = 0; i <= order+1; i++)
        {
            root->children.dataptr[i] = NULL;
        }
        root->children.dataptr[0] = createDataNode();
        root->children.dataptr[0]->lptr = insertInPlaneList(root->children.dataptr[0]->lptr, newNode);
        root->children.dataptr[0]->size = 1;
        root->parent = NULL;
        root->key[0] = newNode->departureTime;
        Actualroot = root;
    }

    else if (root->isLeaf == 1)
    {
        int i = 0;
        // Finding appropriate position
        while (i < root->activeKeys && maxTime(newNode->departureTime, root->key[i]) < 0)
            i++;

        if (root->children.dataptr[i] == NULL)
        {
            root->children.dataptr[i] = createDataNode();
            root->children.dataptr[i-1]->next = root->children.dataptr[i];
            root->children.dataptr[i]->prev = root->children.dataptr[i-1];
        }

        root->children.dataptr[i]->lptr = insertInPlaneList(root->children.dataptr[i]->lptr, newNode);
    
        Flight *ptr = root->children.dataptr[i]->lptr;
        DataNode *temp = root->children.dataptr[0];
        root->children.dataptr[i]->size++;

        if (root->children.dataptr[i]->size == order+1 ) // DataNode is FULL
        { 
            root = splitDataNode (Actualroot, root,i);
            //after splitting dataNode, if the bplus tree node also needs to be splitted, return the new root
        }
    }

    else
    {
        int i = 0;
        while (i < root->activeKeys && maxTime(newNode->departureTime, root->key[i]) == (-1))
            i++;
        printf ("recursion");
        root = insertInbPlusTree(Actualroot, root->children.nodeptr[i], newNode);
    }
    return Actualroot;
}

Bucket *createBucket(Flight *flight)
{
    Bucket *new = (Bucket *)malloc(sizeof(Bucket));
    new->beginningETA.Hour = flight->ETA.Hour;
    new->endETA.Hour = flight->ETA.Hour;
    new->beginningETA.Min = 0;
    new->endETA.Min = 59;
    new->next = NULL;
    new->root = NULL;
    new->isLeaf = 1;
    return new;
}

Bucket *insert(Bucket *firstBucket, Flight *flight)
{
    Bucket *tempBucketPtr = firstBucket;
    Bucket *prevBucketPtr = NULL;

    if (tempBucketPtr == NULL)
    {
        firstBucket = createBucket(flight);
        tempBucketPtr = firstBucket;
    }
    
    else
    {
        while (tempBucketPtr != NULL && maxTime(tempBucketPtr->endETA, flight->ETA) > 0)
        {
            prevBucketPtr = tempBucketPtr;
            tempBucketPtr = tempBucketPtr->next;
        }

        if (tempBucketPtr != NULL && maxTime(flight->ETA, tempBucketPtr->endETA) >= 0 && maxTime(tempBucketPtr->beginningETA, flight->ETA) >= 0)
        {}
        else
        {
            Bucket *newBucket = createBucket(flight);
            newBucket->next = tempBucketPtr;
            if (tempBucketPtr == firstBucket)
            {
                if (maxTime(tempBucketPtr->beginningETA, newBucket->beginningETA) < 0)
                {
                    firstBucket = newBucket;
                }
            }
            else
            {
                prevBucketPtr->next = newBucket;
            }
            tempBucketPtr = newBucket;
        }
        printf ("inserting flight %d:%d", flight->departureTime.Hour, flight->departureTime.Min);
        tempBucketPtr->root = insertInbPlusTree(tempBucketPtr->root, tempBucketPtr->root, flight);
        BPlusTreeNode *tempRoot = tempBucketPtr->root;
        while (tempRoot->parent != NULL) tempRoot = tempRoot->parent;
        tempBucketPtr->root = tempRoot;
        printf ("******************Printing the tree*********************");
        printTree (tempBucketPtr->root);
    }
    
    return firstBucket;
}

Flight *searchForFlight (BPlusTreeNode *root, int ID)
{
    if (root->isLeaf != 1)
    {
        return searchForFlight(root->children.nodeptr[0], ID);
    }
    DataNode *Node = root->children.dataptr[0];
    while (Node != NULL)
    {
        Flight *flightPtr = Node->lptr;
        while (flightPtr != NULL)
        {
            printf ("%d:%d ",flightPtr->departureTime.Hour, flightPtr->departureTime.Min);
            if (flightPtr->flightID == ID)
            {
                printf ("The flight departed at %d:%d and is expected to arrive at %d:%d ",flightPtr->departureTime.Hour, flightPtr->departureTime.Min, flightPtr->ETA.Hour, flightPtr->ETA.Min);
                return flightPtr;
            }
            flightPtr = flightPtr->next;
        }
        Node = Node->next;
    }
    return NULL;
}

Flight *checkStatus (Bucket *firstBucket)
{
    Bucket *bucketPtr = firstBucket;
    printf ("Enter flightID: ");
    int ID;
    scanf ("%d", &ID);
    int found = 0;
    Flight *flight;
    while (bucketPtr != NULL && found == 0)
    {
        //search inside the bucket
        //this needs to be taken care
        if (bucketPtr->root != NULL)
        {
            flight = searchForFlight(bucketPtr->root, ID);
            if (flight != NULL)
            {
                found = 1;
            }
        }
        bucketPtr = bucketPtr->next;
    }
    if (found == 0)
    {
        printf ("Flight not found");
    }
    return flight;
}

void rangeSearch(Bucket *firstBucket)
{
    Time start, end;
    printf ("Enter the start time: ");
    scanf ("%d:%d", &start.Hour, &start.Min);
    printf ("Enter the end time: ");
    scanf ("%d:%d", &end.Hour, &end.Min);

    if (maxTime (start, end) < 1) printf ("Invalid");

    Bucket *bucketPtr = firstBucket;
    while (bucketPtr != NULL )
    {
        printf ("%d", bucketPtr == NULL);
        BPlusTreeNode *root = bucketPtr->root;
        int flag = 0;

        while (root != NULL && root->isLeaf != 1 && flag == 0)
        {
            int i = 0;
            while ( i < root->activeKeys && maxTime(root->key[i], start) > 1) i++;
            if ( root->children.nodeptr[i] != NULL)
            {
                root = root->children.nodeptr[i];
            }
            else flag = 1;
        }
        
        if (root != NULL && flag == 0)
        {
            DataNode *Node = root->children.dataptr[0];

            while (Node != NULL && flag == 0)
            {
                Flight *flightPtr = Node->lptr;
                while (flightPtr != NULL && maxTime(flightPtr->departureTime, start) > 0) flightPtr = flightPtr->next;
                while ( flightPtr != NULL && maxTime(flightPtr->departureTime, end) >= 0) 
                {
                    printf ("Flight Id: %d, Departure Time %d:%d\n", flightPtr->flightID, flightPtr->departureTime.Hour, flightPtr->departureTime.Min);
                    flightPtr = flightPtr->next;
                }
                Node = Node->next;
            }
        }
        bucketPtr = bucketPtr->next;
    }
}

void showMenu(Bucket *firstBucket)
{
    int option;
    do
    {
        printf("\n\n1 - Insert a Flight \n2 - Check Status \n3 - To search a range of flights\nPress any other key to exit\n\n");
        printf("Select option : ");
        scanf("%d", &option);
        Time temp, deptTime, ETA;
        Flight *flight;

        switch (option)
        {
        case 1:
            printf("Enter details for the flight to be added :\n");
            int ID;
            printf("Flight ID : ");
            scanf("%d", &ID);
            printf("Departure time : \nhr = ");
            scanf("%d", &deptTime.Hour);
            printf("min = ");
            scanf("%d", &deptTime.Min);

            printf("Expected Arrival time : \nhr = ");
            scanf("%d", &ETA.Hour);
            printf("min = ");
            scanf("%d", &ETA.Min);

            if (deptTime.Hour > 23 || deptTime.Min > 59 ||
                ETA.Hour >23 || ETA.Min >59 ) printf ("Invalid Time");
            else 
            {
                Flight *newNode;
                newNode = createNode(ID, deptTime, ETA);
                firstBucket = insert(firstBucket, newNode);
                printf("Flight Added Successfully.\n");
            }
            break;

        case 2:
            flight = checkStatus(firstBucket);
            break;
        case 3:
            rangeSearch(firstBucket);
        }
    } while (option > 0 && option < 4);
}

int main()
{
    int tempFlightID;
    Time tempDepartureTime, tempETA;
    FILE *fptr = fopen("Data2.csv", "r");

    Flight *planeNode; // temp pointer to link nodes in list

    Bucket *firstBucket;
    firstBucket = NULL;
    char line[100];
    while (fgets(line, 100, fptr) != NULL)
    {
        sscanf(line, "%d,%d,%d,%d,%d", &tempFlightID, &(tempETA.Hour), &(tempETA.Min), &(tempDepartureTime.Hour), &(tempDepartureTime.Min));
        planeNode = createNode(tempFlightID, tempDepartureTime, tempETA);
        firstBucket = insert(firstBucket, planeNode);
        printf ("\n");
    }
    showMenu(firstBucket);
    return 0;
}