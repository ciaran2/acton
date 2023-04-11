#pragma once

#include "builtin/builtin.h"
#include "builtin/env.h"
#include "rts/rts.h"

struct lambda$1;
struct lambda$2;
struct Act;
struct lambda$3;
struct lambda$4;
struct Root;

typedef struct lambda$1 *lambda$1;
typedef struct lambda$2 *lambda$2;
typedef struct Act *Act;
typedef struct lambda$3 *lambda$3;
typedef struct lambda$4 *lambda$4;
typedef struct Root *Root;

struct lambda$1G_class {
    GC_descr $GCdescr;
    char *$name;
    int $class_id;
    $SuperG_class $superclass;
    void (*__init__)(lambda$1, $Cont);
    void (*__serialize__)(lambda$1, B_NoneType);
    lambda$1 (*__deserialize__)(lambda$1, B_NoneType);
    B_bool (*__bool__)(lambda$1);
    B_str (*__str__)(lambda$1);
    $R (*__call__)(lambda$1, B_Msg);    
};
struct lambda$1 {
    struct lambda$1G_class *$class;
    $Cont cont$0;    
};
extern GC_word lambda$1D_gcbm[GC_BITMAP_SIZE(struct lambda$1)];

struct lambda$2G_class {
    GC_descr $GCdescr;
    char *$name;
    int $class_id;
    $SuperG_class $superclass;
    void (*__init__)(lambda$2, Act, B_int, B_list);
    void (*__serialize__)(lambda$2, B_NoneType);
    lambda$2 (*__deserialize__)(lambda$2, B_NoneType);
    B_bool (*__bool__)(lambda$2);
    B_str (*__str__)(lambda$2);
    $R (*__call__)(lambda$2, $Cont);    
};
struct lambda$2 {
    struct lambda$2G_class *$class;
    Act self;
    B_int from;
    B_list table;
};
extern GC_word lambda$2D_gcbm[GC_BITMAP_SIZE(struct lambda$2)];

struct ActG_class {
    GC_descr $GCdescr;
    char *$name;
    int $class_id;
    $SuperG_class $superclass;
    $R (*__init__)(Act, B_int, $Cont);
    void (*__serialize__)(Act, B_NoneType);
    Act (*__deserialize__)(Act, B_NoneType);
    B_bool (*__bool__)(Act);
    B_str (*__str__)(Act);
    $R (*actG_local)(Act, B_int, B_list, $Cont);
    $R (*act)(Act, B_int, B_list, $Cont);
};
struct Act {
    union {
        struct ActG_class *$class;
        struct $Actor super;
    };
    B_int i;
    B_int count;
    B_dict rcv_dict;
    B_dict snd_dict;
};
extern GC_word ActD_gcbm[GC_BITMAP_SIZE(struct Act)];

struct lambda$3G_class {
    GC_descr $GCdescr;
    char *$name;
    int $class_id;
    $SuperG_class $superclass;
    void (*__init__)(lambda$3, Root, B_Iterator, $Cont);
    void (*__serialize__)(lambda$3, B_NoneType);
    lambda$3 (*__deserialize__)(lambda$3, B_NoneType);
    B_bool (*__bool__)(lambda$3);
    B_str (*__str__)(lambda$3);
    $R (*__call__)(lambda$3, Act);
};
struct lambda$3 {
    struct lambda$3G_class *$class;
    Root self;
    B_Iterator iter$1;
    $Cont cont$0;
};
extern GC_word lambda$3D_gcbm[GC_BITMAP_SIZE(struct lambda$3)];

struct lambda$4G_class {
    GC_descr $GCdescr;
    char *$name;
    int $class_id;
    $SuperG_class $superclass;
    void (*__init__)(lambda$4, $Cont);
    void (*__serialize__)(lambda$4, B_NoneType);
    lambda$4 (*__deserialize__)(lambda$4, B_NoneType);
    B_bool (*__bool__)(lambda$4);
    B_str (*__str__)(lambda$4);
    $R (*__call__)(lambda$4, $WORD);
};
struct lambda$4 {
    struct lambda$4G_class *$class;
    $Cont cont$0;
};
extern GC_word lambda$4D_gcbm[GC_BITMAP_SIZE(struct lambda$4)];

struct RootG_class {
    GC_descr $GCdescr;
    char *$name;
    int $class_id;
    $SuperG_class $superclass;
    $R (*__init__)(Root, B_Env, $Cont);
    void (*__serialize__)(Root, B_NoneType);
    Root (*__deserialize__)(Root, B_NoneType);
    B_bool (*__bool__)(Root);
    B_str (*__str__)(Root);
};
struct Root {
    union {
        struct RootG_class *$class;
        struct $Actor super;
    };
    B_list table;
};
extern GC_word RootD_gcbm[GC_BITMAP_SIZE(struct Root)];

$R loop$1(Root, B_Iterator, $Cont, $WORD);
$R cont$1(Root, B_Iterator, $Cont, Act);
$R join$1(Root, $Cont, $WORD);

extern struct lambda$1G_class lambda$1G_methods;
extern struct lambda$2G_class lambda$2G_methods;
extern struct ActG_class ActG_methods;
extern struct lambda$3G_class lambda$3G_methods;
extern struct lambda$4G_class lambda$4G_methods;
extern struct RootG_class RootG_methods;

extern B_Mapping B_MappingD_dictG_witness;
extern B_Indexed B_IndexedD_MappingD_dictG_witness;

extern B_int no_actors;
extern B_int total_msgs;
