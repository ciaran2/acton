
struct numpy$$Integral$ndarray;
typedef struct numpy$$Integral$ndarray *numpy$$Integral$ndarray;

struct numpy$$Integral$ndarray$class;
typedef struct numpy$$Integral$ndarray$class *numpy$$Integral$ndarray$class;

struct numpy$$Logical$ndarray;
typedef struct numpy$$Logical$ndarray *numpy$$Logical$ndarray;

struct numpy$$Logical$ndarray$class;
typedef struct numpy$$Logical$ndarray$class *numpy$$Logical$ndarray$class;

struct numpy$$Minus$ndarray;
typedef struct numpy$$Minus$ndarray *numpy$$Minus$ndarray;

struct numpy$$Minus$ndarray$class;
typedef struct numpy$$Minus$ndarray$class *numpy$$Minus$ndarray$class;

struct numpy$$Iterable$ndarray;
typedef struct numpy$$Iterable$ndarray *numpy$$Iterable$ndarray;

struct numpy$$Iterable$ndarray$class;
typedef struct numpy$$Iterable$ndarray$class *numpy$$Iterable$ndarray$class;

// numpy$$Integral$ndarray ////////////////////////////////////////////////////////////

struct numpy$$Integral$ndarray {
    numpy$$Integral$ndarray$class $class;
    $Logical w$Logical;
    $Minus w$Minus;
    numpy$$Primitive w$Primitive$A$numpy;
};

struct numpy$$Integral$ndarray$class {
    char *$GCINFO;
    int $class_id;
    $Super$class $superclass;
    void (*__init__)(numpy$$Integral$ndarray,numpy$$Primitive);
    void (*__serialize__)(numpy$$Integral$ndarray,$Serial$state); 
    numpy$$Integral$ndarray (*__deserialize__)($Serial$state);
    $bool (*__bool__)(numpy$$Integral$ndarray);
    $str (*__str__)(numpy$$Integral$ndarray);
    numpy$$ndarray (*__add__)(numpy$$Integral$ndarray, numpy$$ndarray, numpy$$ndarray);
    numpy$$ndarray (*__fromatom__)(numpy$$Integral$ndarray,$WORD);
    $complex (*__complx__)(numpy$$Integral$ndarray, numpy$$ndarray);
    numpy$$ndarray (*__mul__)(numpy$$Integral$ndarray, numpy$$ndarray, numpy$$ndarray);
    numpy$$ndarray (*__truediv__)(numpy$$Integral$ndarray, numpy$$ndarray, numpy$$ndarray);
    numpy$$ndarray (*__pow__)(numpy$$Integral$ndarray, numpy$$ndarray, numpy$$ndarray);
    numpy$$ndarray (*__neg__)(numpy$$Integral$ndarray, numpy$$ndarray);
    numpy$$ndarray (*__pos__)(numpy$$Integral$ndarray, numpy$$ndarray);
    $WORD (*real)(numpy$$Integral$ndarray, $Real, numpy$$ndarray);
    $WORD (*imag)(numpy$$Integral$ndarray, $Real, numpy$$ndarray);
    $WORD (*__abs__)(numpy$$Integral$ndarray, $Real, numpy$$ndarray);
    numpy$$ndarray (*conjugate)(numpy$$Integral$ndarray, numpy$$ndarray);
    $float (*__float__)(numpy$$Integral$ndarray, numpy$$ndarray);
    $WORD (*__trunc__)(numpy$$Integral$ndarray, $Integral, numpy$$ndarray);
    $WORD (*__floor__)(numpy$$Integral$ndarray, $Integral, numpy$$ndarray);
    $WORD (*__ceil__)(numpy$$Integral$ndarray, $Integral, numpy$$ndarray);
    numpy$$ndarray (*__round__)(numpy$$Integral$ndarray, numpy$$ndarray, numpy$$ndarray);
    $WORD (*numerator)(numpy$$Integral$ndarray, $Integral, numpy$$ndarray);
    $WORD (*denominator)(numpy$$Integral$ndarray, $Integral, numpy$$ndarray);
    numpy$$ndarray (*__int__)(numpy$$Integral$ndarray, numpy$$ndarray);
    numpy$$ndarray (*__index__)(numpy$$Integral$ndarray, numpy$$ndarray);
    $tuple (*__divmod__)(numpy$$Integral$ndarray, numpy$$ndarray, numpy$$ndarray);
    numpy$$ndarray (*__floordiv__)(numpy$$Integral$ndarray, numpy$$ndarray, numpy$$ndarray);
    numpy$$ndarray (*__mod__)(numpy$$Integral$ndarray, numpy$$ndarray, numpy$$ndarray);
    numpy$$ndarray (*__lshift__)(numpy$$Integral$ndarray, numpy$$ndarray, numpy$$ndarray);
    numpy$$ndarray (*__rshift__)(numpy$$Integral$ndarray, numpy$$ndarray, numpy$$ndarray);
    numpy$$ndarray (*__invert__)(numpy$$Integral$ndarray, numpy$$ndarray);
};

void numpy$$Integral$ndarray$__init__ ($Integral);
void numpy$$Integral$ndarray$__serialize__(numpy$$Integral$ndarray,$Serial$state); 
numpy$$Integral$ndarray numpy$$Integral$ndarray$__deserialize__($Serial$state);
numpy$$ndarray numpy$$Integral$ndarray$__add__(numpy$$Integral$ndarray, numpy$$ndarray, numpy$$ndarray);
numpy$$ndarray numpy$$Integral$ndarray$__fromatom__(numpy$$Integral$ndarray,$WORD);
$complex numpy$$Integral$ndarray$__complx__(numpy$$Integral$ndarray, numpy$$ndarray);
numpy$$ndarray numpy$$Integral$ndarray$__mul__(numpy$$Integral$ndarray, numpy$$ndarray, numpy$$ndarray);
numpy$$ndarray numpy$$Integral$ndarray$__truediv__(numpy$$Integral$ndarray, numpy$$ndarray, numpy$$ndarray);
numpy$$ndarray numpy$$Integral$ndarray$__pow__(numpy$$Integral$ndarray, numpy$$ndarray, numpy$$ndarray);
numpy$$ndarray numpy$$Integral$ndarray$__neg__(numpy$$Integral$ndarray, numpy$$ndarray);
numpy$$ndarray numpy$$Integral$ndarray$__pos__(numpy$$Integral$ndarray, numpy$$ndarray);
$WORD numpy$$Integral$ndarray$real(numpy$$Integral$ndarray, $Real, numpy$$ndarray);
$WORD numpy$$Integral$ndarray$imag(numpy$$Integral$ndarray, $Real, numpy$$ndarray);
$WORD numpy$$Integral$ndarray$__abs__(numpy$$Integral$ndarray, $Real, numpy$$ndarray);
numpy$$ndarray numpy$$Integral$ndarray$conjugate(numpy$$Integral$ndarray, numpy$$ndarray);
$float numpy$$Integral$ndarray$__float__ (numpy$$Integral$ndarray, numpy$$ndarray);
$WORD numpy$$Integral$ndarray$__trunc__ (numpy$$Integral$ndarray, $Integral, numpy$$ndarray);
$WORD numpy$$Integral$ndarray$__floor__ (numpy$$Integral$ndarray, $Integral, numpy$$ndarray);
$WORD numpy$$Integral$ndarray$__ceil__ (numpy$$Integral$ndarray, $Integral, numpy$$ndarray);
numpy$$ndarray numpy$$Integral$ndarray$__round__ (numpy$$Integral$ndarray, numpy$$ndarray, numpy$$ndarray);
$WORD numpy$$Integral$ndarray$numerator (numpy$$Integral$ndarray, $Integral, numpy$$ndarray);
$WORD numpy$$Integral$ndarray$denominator (numpy$$Integral$ndarray, $Integral, numpy$$ndarray);
numpy$$ndarray numpy$$Integral$ndarray$__int__ (numpy$$Integral$ndarray, numpy$$ndarray);
numpy$$ndarray numpy$$Integral$ndarray$__index__ (numpy$$Integral$ndarray, numpy$$ndarray);
$tuple numpy$$Integral$ndarray$__divmod__ (numpy$$Integral$ndarray, numpy$$ndarray, numpy$$ndarray);
numpy$$ndarray numpy$$Integral$ndarray$__floordiv__ (numpy$$Integral$ndarray, numpy$$ndarray, numpy$$ndarray);
numpy$$ndarray numpy$$Integral$ndarray$__mod__ (numpy$$Integral$ndarray, numpy$$ndarray, numpy$$ndarray);
numpy$$ndarray numpy$$Integral$ndarray$__lshift__ (numpy$$Integral$ndarray, numpy$$ndarray, numpy$$ndarray);
numpy$$ndarray numpy$$Integral$ndarray$__rshift__ (numpy$$Integral$ndarray, numpy$$ndarray, numpy$$ndarray);
numpy$$ndarray numpy$$Integral$ndarray$__invert__ (numpy$$Integral$ndarray, numpy$$ndarray);

// numpy$$Logical$ndarray ////////////////////////////////////////////////////////////

struct numpy$$Logical$ndarray {
    numpy$$Logical$ndarray$class $class;
    $Integral w$Integral;
};

struct numpy$$Logical$ndarray$class {
    char *$GCINFO;
    int $class_id;
    $Super$class $superclass;
    void (*__init__)(numpy$$Logical$ndarray, $Integral);
    void (*__serialize__)(numpy$$Logical$ndarray,$Serial$state); 
    numpy$$Logical$ndarray (*__deserialize__)($Serial$state);
    $bool (*__bool__)(numpy$$Logical$ndarray);
    $str (*__str__)(numpy$$Logical$ndarray);
    numpy$$ndarray (*__and__)(numpy$$Logical$ndarray, numpy$$ndarray, numpy$$ndarray);
    numpy$$ndarray (*__or__)(numpy$$Logical$ndarray, numpy$$ndarray, numpy$$ndarray);
    numpy$$ndarray (*__xor__)(numpy$$Logical$ndarray, numpy$$ndarray, numpy$$ndarray);
};

void numpy$$Logical$ndarray$__init__ (numpy$$Logical$ndarray, $Integral);
void numpy$$Logical$ndarray$__serialize__(numpy$$Logical$ndarray,$Serial$state); 
numpy$$Logical$ndarray numpy$$Logical$ndarray$__deserialize__($Serial$state);
numpy$$ndarray numpy$$Logical$ndarray$__and__ (numpy$$Logical$ndarray, numpy$$ndarray, numpy$$ndarray);
numpy$$ndarray numpy$$Logical$ndarray$__or__ (numpy$$Logical$ndarray, numpy$$ndarray, numpy$$ndarray);
numpy$$ndarray numpy$$Logical$ndarray$__xor__ (numpy$$Logical$ndarray, numpy$$ndarray, numpy$$ndarray);

// numpy$$Minus$ndarray ////////////////////////////////////////////////////////////

struct numpy$$Minus$ndarray {
    numpy$$Minus$ndarray$class $class;
    $Integral w$Integral;
};

struct numpy$$Minus$ndarray$class {
    char *$GCINFO;
    int $class_id;
    $Super$class $superclass;
    void (*__init__)(numpy$$Minus$ndarray, $Integral);
    void (*__serialize__)(numpy$$Minus$ndarray,$Serial$state); 
    numpy$$Minus$ndarray (*__deserialize__)($Serial$state);
    $bool (*__bool__)(numpy$$Minus$ndarray);
    $str (*__str__)(numpy$$Minus$ndarray);
    numpy$$ndarray (*__sub__)(numpy$$Minus$ndarray, numpy$$ndarray, numpy$$ndarray);
};

void numpy$$Minus$ndarray$__init__ (numpy$$Minus$ndarray, $Integral);
void numpy$$Minus$ndarray$__serialize__(numpy$$Minus$ndarray,$Serial$state); 
numpy$$Minus$ndarray numpy$$Minus$ndarray$__deserialize__($Serial$state);
numpy$$ndarray numpy$$Minus$ndarray$__sub__ (numpy$$Minus$ndarray, numpy$$ndarray, numpy$$ndarray);

// numpy$$Iterable$ndarray ////////////////////////////////////////////////////////////

/*  Later (needs implementation of Iterator$ndarray)
struct numpy$$Iterable$ndarray {
  numpy$$Iterable$ndarray$class $class;
  numpy$$Primitive pwit;
};

struct numpy$$Iterable$ndarray$class {
    char *$GCINFO;
    int $class_id;
    $Super$class $superclass;
    void (*__init__)(numpy$$Iterable$ndarray, numpy$$Primitive);
    void (*__serialize__)(numpy$$Iterable$ndarray,$Serial$state); 
    numpy$$Iterable$ndarray (*__deserialize__)($Serial$state);
    $bool (*__bool__)(numpy$$Iterable$ndarray);
    $str (*__str__)(numpy$$Iterable$ndarray);
    $Iterator (*__iter__)(numpy$$Iterable$ndarray, numpy$$ndarray);
};

void numpy$$Iterable$ndarray$__init__ (numpy$$Iterable$ndarray);
void numpy$$Iterable$ndarray$__serialize__(numpy$$Iterable$ndarray,$Serial$state); 
numpy$$Iterable$ndarray numpy$$Iterable$ndarray$__deserialize__($Serial$state);
$Iterator numpy$$Iterable$ndarray$__iter__ (numpy$$Iterable$ndarray, numpy$$ndarray);
*/


// method tables /////////////////////////////////////////////////////////////////

extern struct numpy$$Integral$ndarray$class numpy$$Integral$ndarray$methods;
extern struct numpy$$Logical$ndarray$class numpy$$Logical$ndarray$methods;
extern struct numpy$$Minus$ndarray$class numpy$$Minus$ndarray$methods;
// extern struct numpy$$Iterable$ndarray$class numpy$$Iterable$ndarray$methods;


