
typedef struct {
    $WORD key;
    long hash;    
} B_setentry;

struct B_set {
    struct B_setG_class *$class;
    long numelements;    // nr of elements in B_set
    long fill;           // numelements + #dummy entries
    long mask;
    long finger;                       // Search finger for pop() 
    B_setentry *table;                  // the hashtable
};
extern GC_word B_setD_gcbm[GC_BITMAP_SIZE(struct B_set)];


// Iterators over sets ///////////////////////////////////////////////////////

typedef struct B_IteratorD_set *B_IteratorD_set; ;

struct B_IteratorD_setG_class {
    GC_descr $GCdescr;
    char *$name;
    int $class_id;
    $SuperG_class $superclass;
    void (*__init__)(B_IteratorD_set, B_set);
    void (*__serialize__)(B_IteratorD_set, $Serial$state);
    B_IteratorD_set (*__deserialize__)(B_IteratorD_set, $Serial$state);
    B_bool (*__bool__)(B_IteratorD_set);
    B_str (*__str__)(B_IteratorD_set);
    B_str (*__repr__)(B_IteratorD_set);
    $WORD(*__next__)(B_IteratorD_set);
};

struct B_IteratorD_set {
    struct B_IteratorD_setG_class *$class;
    B_set src;
    int nxt;
};
extern GC_word B_IteratorD_setD_gcbm[GC_BITMAP_SIZE(struct B_IteratorD_set)];

extern struct  B_IteratorD_setG_class  B_IteratorD_setG_methods;
B_IteratorD_set B_IteratorD_setG_new(B_set);
