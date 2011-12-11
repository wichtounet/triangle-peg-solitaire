#include <cstdlib>

/* Utility functions */

template<typename T>
bool CAS32(T old, T value, T* ptr){
    return __sync_bool_compare_and_swap(ptr, old, value);
}

template<typename T>
bool CASPTR(T** ptr, T* old, T* value){
    return __sync_bool_compare_and_swap(ptr, old, value);    
}

template<typename T>
T FetchAndIncrement(T* ptr){
    return __sync_fetch_and_add(ptr, 1);      
}

/* Concurrent Hash Map */

struct NODE {
    bool sentinel;
    unsigned long reversedKey;
    unsigned long item;
    unsigned long value;
    NODE* next;
};

static NODE*** PrimaryBuckets;
static unsigned long ItemCount;
static unsigned long Size = 2;

#define MAXLOAD 2
#define CHAR_BIT 8

unsigned long inline Reverse32(unsigned long v){
    unsigned long reversed = v;

    int s = sizeof(v) * CHAR_BIT - 1;

    for(v >>= 1; v; v >>= 1){
        reversed <<= 1;
        reversed |= v & 1;
        s--;
    }

    reversed <<= s;

    return reversed;
}

unsigned long inline HashCode(unsigned long item){
    return item;
}

NODE* GetSecondaryBucket(unsigned long key);
bool ListInsert(NODE* head, NODE* node);
bool Find(NODE* head, unsigned long reversedKey, bool isSential, NODE** pred, NODE** curr);

bool Initialize(){
    PrimaryBuckets = (NODE***) calloc(48, sizeof(NODE**));

    if(PrimaryBuckets != NULL){
        PrimaryBuckets[0] = (NODE**) calloc(Size, sizeof(NODE*));

        if(PrimaryBuckets[0] != NULL){
            PrimaryBuckets[0][0] = (NODE*) malloc(sizeof(NODE));

            if(PrimaryBuckets[0][0] != NULL){
                PrimaryBuckets[0][0]->sentinel = true;
                PrimaryBuckets[0][0]->next = NULL;
                return true;
            }

            free(PrimaryBuckets[0]);
        }

        free(PrimaryBuckets);
    }

    return false;
}

NODE* InitializeBucket(unsigned long key){
    NODE* sentinel;
    NODE* parentBucket;
    NODE* pred;

    unsigned long parent = Size;

    while ((parent = parent >> 1) > key);

    parentBucket = GetSecondaryBucket(key - parent);

    if ((sentinel = (NODE*) malloc(sizeof(NODE))) != NULL){
        sentinel->sentinel = true;
        sentinel->reversedKey = key = Reverse32(key);
        sentinel->next = NULL;

        if(!ListInsert(parentBucket, sentinel)){
            free(sentinel);
            if(!Find(parentBucket, key, true, &pred, &sentinel)){
                return NULL;
            }
        }
    }

    return sentinel;
}

NODE* GetSecondaryBucket(unsigned long key){
    unsigned long i;
    unsigned long min;
    unsigned long max;

    NODE** secondary;
    NODE* sentinel;

    for(i = min = 0U, max = 2U; key >= max; i += 1, max <<= 1){
        min = max;
    }

    if(PrimaryBuckets[i] == NULL){
        secondary = (NODE**) calloc(Size>>1, sizeof(NODE*));
        if(secondary == NULL){
            return NULL;
        }
        if(!CASPTR(&PrimaryBuckets[i], (NODE**) NULL, (NODE**) secondary)){
            free(secondary);
        }
    }
    
    if(PrimaryBuckets[i][key - min] == NULL){
        if((sentinel = InitializeBucket(key)) != NULL){
            PrimaryBuckets[i][key - min] = sentinel;
        }
    }

    return PrimaryBuckets[i][key - min];
}

bool ListInsert(NODE* head, NODE* node){
    NODE* p;
    NODE* c;

    while(true){
        if(Find(head, node->reversedKey, node->sentinel, &p, &c)){
            return false;
        }

        node->next = c;

        if(CASPTR(&p->next, c, node)){
            return true;
        }
    }
}

union CONVERSION {
    NODE* node;
    unsigned long value;
};

bool Find(NODE* h, unsigned long rKey, bool s, NODE** p, NODE** c){
    CONVERSION next;

    while(true){
        *p = h;
        *c = (*p)->next;

        while(true){
            if(*c == NULL){
                return false;
            }

            next.node = (*c)->next;

            if(next.value & 0x1){
                next.value ^= 0x1;
                if(CASPTR(&(*p)->next, (NODE*) *c, (NODE*) next.node)){
                    *c = next.node; 
                } else {
                    break;
                }
            } else if ((*c)->reversedKey == rKey && s == (*c)->sentinel){
                return true;
            } else if ((*c)->reversedKey == rKey && s){
                return false;
            } else if ((*c)->reversedKey > rKey){
                return false;
            } else {
                *p = *c;
                *c = next.node;
            }
        }
    }
}

bool Set(unsigned long item, unsigned long value){
    NODE* bucket;
    NODE* newNode;
    unsigned long size;
    unsigned long key;

    if((newNode = (NODE*) malloc(sizeof(NODE))) == NULL){
        return false;
    }

    key = HashCode(item);
    
    newNode->sentinel = false;
    newNode->reversedKey = Reverse32(key);
    newNode->item = item;
    newNode->value = value;

    if((bucket = GetSecondaryBucket(key % Size)) == NULL){
        return false;
    }

    if(!ListInsert(bucket, newNode)){
        free(newNode);
        return false;
    }

    size = Size;
    if(FetchAndIncrement(&ItemCount) / size >= MAXLOAD){
        CAS32(size, size*2, &Size);
    }

    return true;
}

//Don't care about Remove

bool Contains(unsigned long item){
    NODE* head;
    NODE* curr;
    NODE* pred;

    unsigned long key = HashCode(item);

    if((head = GetSecondaryBucket(key % Size)) == NULL){
        return false;
    }

    key = Reverse32(key);

    return Find(head, key, false, &pred, &curr);
}

unsigned long Get(unsigned long item){
    NODE* head;
    NODE* curr;
    NODE* pred;

    unsigned long key = HashCode(item);

    if((head = GetSecondaryBucket(key % Size)) == NULL){
        return false;
    }

    key = Reverse32(key);

    Find(head, key, false, &pred, &curr);
    return curr->value;
}
