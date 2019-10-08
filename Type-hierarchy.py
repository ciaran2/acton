######################################### Type and class hierarchy ###############################################

protocol Eq:
    @staticmethod
    __eq__          : (Self,Self) -> bool
    @staticmethod
    __ne__          : (Self,Self) -> bool
    
    def __ne__(a,b):
        return not (a == b)
====>
"typeclass" Eq[S]:
    __eq__          : (S,S) -> bool
    __ne__          : (S,S) -> bool

    def __ne__(a,b):
        return not (a == b)
====>
struct Eq[S]:
    @classmethod
    __eq__          : (S,S) -> bool
    @classmethod
    __ne__          : (S,S) -> bool

    def __ne__(cls,a,b):
        return not cls.__eq__(a,b)
    
-----------------------------------------------------------------

class Fruit (Eq):
    @staticmethod
    def __eq__(a, b):
        return a.weight == b.weight

class Apple (Fruit):
    @staticmethod
    def __eq__(a, b):
        return Fruit.__eq__(a,b) and a.appleness == b.appleness

class Orange (Fruit):
    @staticmethod
    def __eq__(a, b):
        return Fruit.__eq__(a,b) and a.orangeness == b.orangeness

a, b : Apple
a == b
x, y : Orange
x == y
f1 : Fruit = a
f2 : Fruit = x
f1 == f2
a == x
====>
class Fruit:
    ...
"instance" Eq[Fruit]:
    def __eq__(a, b):
        return a.weight == b.weight
class Apple (Fruit):
    ...
"instance" Eq[Apple]:
    def __eq__(a, b):
        return Fruit.__eq__(a,b) and a.appleness == b.appleness
class Orange (Fruit):
    ...
"instance" Eq[Orange]:
    def __eq__(a, b):
        return Fruit.__eq__(a,b) and a.orangeness == b.orangeness
====>
class Fruit:
    _Eq         : @Eq[Fruit]        = Eq_Fruit
    ...
struct Eq_Fruit (Eq[Fruit]):
    @classmethod
    def __eq__(cls, a:Fruit, b:Fruit):
        return Eq_int.__eq__(a.weight, b.weight)

class Apple (Fruit):
    _Eq                             = Eq_Apple
    ...
struct Eq_Apple (Eq[Apple]):
    @classmethod
    def __eq__(cls, a:Apple, b:Apple):
        return Fruit._Eq.__eq__(a,b) and Eq_str(a.appleness, b.appleness)

class Orange (Fruit):
    _Eq                             = Eq_Orange
    ...
struct Eq_Orange (Eq[Orange]):
    @classmethod
    def __eq__(cls, a:Orange, b:Orange):
        return Fruit._Eq.__eq__(a,b) and Eq_str(a.orangeness, b.orangeness)

a, b : Apple
Eq_Apple.__eq__(a,b)
x, y : Orange
Eq_Orange.__eq__(x,y)
f1 : Fruit = a
f2 : Fruit = x
Eq_Fruit.__eq__(f1,f2)
Eq_Fruit.__eq__(a,x)

-----------------------------------------------------------------

class MyClass (Hashable):
    def __eq__(a, b):
        return hash(a) == hash(b)
    def __hash__(self):
        return self.attr1
    def my_method(self):
        return hash(self)
        
class MySubClass (MyClass):
    def __hash__(self):
        return fancy_hash(self.attr1)

x, y : MySubClass
z : MyClass = y
x == y  -->  MySubClass.__eq__(x,y)
x == z  -->  MyClass.__eq__(x,z)

y.my_method()  -->  hash(self)  -->  self.__hash__()  -->  fancy_hash(y.attr1)
z.my_method()  -->  hash(self)  -->  self.__hash__()  -->  fancy_hash(z.attr1)

====>
class MyClass:
    def my_method(self):
        return hash(self)
"instance" Hashable[MyClass]:
    def __hash__(self):
        return self.attr1
"instance" Eq[MyClass]:
    def __eq__(a, b):
        return hash(a) == hash(b)
class MySubClass (MyClass):
    pass
"instance" Hashable[MySubClass]:
    def __hash__(self):
        return fancy_hash(self.attr1)
====>
class MyClass:
    _Hashable = Hashable_MyClass
    _Eq = Hashable_MyClass
    def my_method(self):
        return self._Hashable.__hash__(self)
struct Hashable_MyClass (Hashable[MyClass]):
    _MyClass = MyClass
    def __hash__(cls, self):
        return self.attr1
struct Eq_MyClass (Eq[MyClass]):
    _MyClass = MyClass
    def __eq__(cls, a, b):
        return Eq_int.__eq__(cls.__hash__(a), cls.__hash__(b))
class MySubClass (MyClass):
    _Hashable = Hashable_MySubClass
    _Eq = Hashable_MySubClass
struct Hashable_MySubClass (Hashable_MyClass):
    _MyClass = MySubClass
    @classmethod
    def __hash__(cls, self):
        return fancy_hash(self.attr1)
Eq_MySubClass = Hashable_MyClass

x, y : MySubClass
z : MyClass = y
MySubClass._Eq.__eq__(x,y)  -->  Hashable_MySubClass.__eq__(x,y)  -->  ... Hashable_MySubClass.__hash__(x) ...
MyClass._Eq.__eq__(x,z)  -->  Hashable_MyClass.__eq__(x,z)  -->  ... Hashable_MyClass.__hash__(x) ...

y.my_method()  -->  MySubClass.my_method(y)  -->  y._Hashable.__hash__(y)  -->  Hashable_MySubClass.__hash__(y)
z.my_method()  -->  MySubClass.my_method(z)  -->  z._Hashable.__hash__(z)  -->  Hashable_MySubClass.__hash__(y)

-----------------------------------------------------------------
    
protocol Ord (Eq):
    @staticmethod
    __lt__          : (Self,Self) -> bool
    @staticmethod
    __le__          : (Self,Self) -> bool
    @staticmethod
    __gt__          : (Self,Self) -> bool
    @staticmethod
    __ge__          : (Self,Self) -> bool

    def __le__(a,b):
        return a < b or a == b
    def __gt__(a,b):
        return b < a
    def __ge__(a,b):
        return b <= a
====>
"typeclass" (Eq[S]) => Ord[S]:
    __lt__          : (S,S) -> bool
    __le__          : (S,S) -> bool
    __gt__          : (S,S) -> bool
    __ge__          : (S,S) -> bool

    def __le__(a,b):
        return a < b or a == b
    def __gt__(a,b):
        return b < a
    def __ge__(a,b):
        return b <= a
====>
struct Ord[S] (Eq[S]):
    @classmethod
    __lt__          : (S,S) -> bool
    @classmethod
    __le__          : (S,S) -> bool
    @classmethod
    __gt__          : (S,S) -> bool
    @classmethod
    __ge__          : (S,S) -> bool

    def __le__(cls,a,b):
        return cls.__lt__(a,b) or cls.__eq__(a,b)
    def __gt__(cls,a,b):
        return cls.__lt__(b,a)
    def __ge__(a,b):
        return cls.__le__(b,a)

-----------------------------------------------------------------

extension int (Eq):
    def __eq__(a:int, b:int) -> bool:
        return primeqint(a,b)
extension int (Ord):
    def __lt__(a:int, b:int) -> bool:
        return primltint(a,b)
#    def __eq__(a:int, b:int) -> bool:             # WOULD TRIGGER AMBIGUITY ERROR: EXISTING EXTENSION int (Eq)
#        return primeqint(a,b)
====>
"instance" Eq[int]:
    def __eq__(a:int, b:int) -> bool:
        return primeqint(a,b)
"instance" Ord[int]:
    def __lt__(a:int, b:int) -> bool:
        return primeqint(a,b)
====>
struct Eq_int (Eq[int]):
    def __eq__(cls, a:int, b:int) -> bool:
        return primeqint(a,b)
struct Ord_int (Ord[int]):
    def __lt__(cls, a:int, b:int) -> bool:
        return primeqint(a,b)

-----------------------------------------------------------------

extension int (Ord):                                   # ALTERNATIVELY: A JOINT EXTENSION int (Ord) AND int (Eq)
    def __eq__(a:int, b:int) -> bool:
        return primeqint(a,b)
    def __lt__(a:int, b:int) -> bool:
        return primltint(a,b)
====>
"instance" Ord[int]:
    def __eq__(a:int, b:int) -> bool:
        return primeqint(a,b)
    def __lt__(a:int, b:int) -> bool:
        return primeqint(a,b)
====>
struct Eq_int (Eq[int]):
    def __eq__(cls, a:int, b:int) -> bool:
        return primeqint(a,b)
struct Ord_int (Eq_int):
    def __lt__(cls, a:int, b:int) -> bool:
        return primeqint(a,b)

-----------------------------------------------------------------------

protocol Hashable (Eq):
    __hash__        : () -> int
====>
"typeclass" (Eq[S]) => Hashable[S]:
    __hash__        : (S) -> int
====>
struct Hashable[S] (Eq[S]):
    @classmethod
    __hash__        : (S) -> int

-----------------------------------------------------------------

protocol SupportsInt:
    __int__         : () -> int
====>
"typeclass" SupportsInt[S]:
    __int__         : (S) -> int
====>
struct SupportsInt[S]:
    @classmethod
    __int__         : (S) -> int

-----------------------------------------------------------------

extension int (SupportsInt):
    def __int__(self:int) -> int
        return self
====>
"instance" SupportsInt[int]:
    def __int__(self:int) -> int
        return self
====>
struct SupportsInt_int (SupportsInt[int]):
    def __int__(self:int) -> int
        return self

-----------------------------------------------------------------

protocol SupportsFloat:
    __float__       : () -> float
====>
"typeclass" SupportsFloat[S]:
    __float__       : (S) -> float
====>
struct SupportsFloat[S]:
    @classmethod
    __float__       : (S) -> float

-----------------------------------------------------------------
    
protocol SupportsComplex:
    __complex__     : () -> complex
====>
"typeclass" SupportsComplex[S]:
    __complex__     : (S) -> complex
====>
struct SupportsComplex[S]:
    @classmethod
    __complex__     : (S) -> complex

-----------------------------------------------------------------

protocol SupportsBytes:
    __bytes__       : () -> bytes
====>
"typeclass" SupportsBytes[S]:
    __bytes__       : (S) -> bytes
====>
struct SupportsBytes[S]:
    @classmethod
    __bytes__       : (S) -> bytes

-----------------------------------------------------------------

protocol Logical:
    @staticmethod
    __and__         : (Self,Self) -> Self
    @staticmethod
    __or__          : (Self,Self) -> Self
    @staticmethod
    __xor__         : (Self,Self) -> Self
====>
"typeclass" Logical[S]:
    __and__         : (S,S) -> S
    __or__          : (S,S) -> S
    __xor__         : (S,S) -> S
====>
struct Logical[S]:
    @classmethod
    __and__         : (S,S) -> S
    @classmethod
    __or__          : (S,S) -> S
    @classmethod
    __xor__         : (S,S) -> S

-----------------------------------------------------------------

protocol Plus:
    @staticmethod
    __add__         : (Self,Self) -> Self
====>
"typeclass" Plus[S]:
    __add__         : (S,S) -> S
====>
struct Plus[S]:
    @classmethod
    __add__         : (S,S) -> S

-----------------------------------------------------------------

protocol Minus:
    @staticmethod
    __sub__         : (Self,Self) -> Self
====>
"typeclass" Minus[S]:
    __sub__         : (S,S) -> S
====>
struct Minus[S]:
    @classmethod
    __sub__         : (S,S) -> S

-----------------------------------------------------------------

protocol Number (Ord, Plus, Minus):
    __pos__         : () -> Self
    __neg__         : () -> Self
    @staticmethod
    __mul__         : (Self,Self) -> Self
    @staticmethod
    __pow__         : (Self,Self) -> Self
    def __sub__(a,b):
        return a + (-b)
====>
"typeclass" (Ord[S],Plus[S],Minus[S]) => Number[S]:
    __pos__         : (S) -> S
    __neg__         : (S) -> S
    __mul__         : (S,S) -> S
    __pow__         : (S,S) -> S
    def __sub__(a:S, b:S):
        return __add__(a, __neg__(b))
====>
struct Plus_Number[S] (Plus[S]):
    _Number         : @Number[S]
struct Minus_Number[S] (Minus[S]):
    _Number         : @Number[S]
    def __sub__(cls, a:S, b:S):
        return cls._Number._Plus.__add__(a, cls._Number.__neg__(b))

struct Number[S] (Ord[S]):
    _Plus           : @Plus_Number[S]
    _Minus          : @Minus_Number[S]

    @classmethod
    __pos__         : (S) -> S
    @classmethod
    __neg__         : (S) -> S
    @classmethod
    __mul__         : (S,S) -> S
    @classmethod
    __pow__         : (S,S) -> S

-----------------------------------------------------------------

protocol Number (Ord, Plus, Minus):                                  # ALTERNATIVELY
    __pos__         : () -> Self
    __neg__         : () -> Self
    @staticmethod
    __mul__         : (Self,Self) -> Self
    @staticmethod
    __pow__         : (Self,Self) -> Self
    def __sub__(a,b):
        return a + (-b)
====>
"typeclass" (Ord_Number[S], Plus_Number[S], Minus_Number[S]) => Number[S]:
    __pos__         : (S) -> S
    __neg__         : (S) -> S
    __mul__         : (S,S) -> S
    __pow__         : (S,S) -> S
"typeclass" (Ord[S], Number[S]) => Ord_Number[S]:
    pass
"typeclass" (Plus[S], Number[S]) => Plus_Number[S]:
    pass
"typeclass" (Minus[S], Number[S]) => Minus_Number[S]:
    def __sub__(a,b):
        return a + (-b)
====>
struct Number[S]:
    _Ord            : @Ord_Number[S]
    _Plus           : @Plus_Number[S]
    _Minus          : @Minus_Number[S]
    @classmethod
    __pos__         : (S) -> S
    @classmethod
    __neg__         : (S) -> S
    @classmethod
    __mul__         : (S,S) -> S
    @classmethod
    __pow__         : (S,S) -> S
struct Ord_Number[S]:
    _Ord            : @Ord[S]
    _Number         : @Number[S]
struct Plus_Number[S]:
    _Plus           : @Plus[S]
    _Number         : @Number[S]
struct Minus_Number[S]:
    _Minus          : @Minus[S]
    _Number         : @Number[S]
    def __sub__(cls, a:S, b:S):
        return cls._Number._Plus.__add__(a, cls._Number.__neg__(b))

-----------------------------------------------------------------

protocol Number (Ord, Plus, Minus):                                  # OR......................
    __pos__         : () -> Self
    __neg__         : () -> Self
    @staticmethod
    __mul__         : (Self,Self) -> Self
    @staticmethod
    __pow__         : (Self,Self) -> Self
    def __sub__(a,b):
        return a + (-b)
====>
"typeclass" (Ord[S], Plus[S], Minus[S]) => Number[S]:               # TYPE-CHECKED AS IN PLAIN HASKELL
    __pos__         : (S) -> S
    __neg__         : (S) -> S
    __mul__         : (S,S) -> S
    __pow__         : (S,S) -> S
    def __sub__(a,b):                                               # ALLOWED, BUT TRANSLATED INTO...
        return a + (-b)
====>
struct Number[S]:
    _Ord            : @Ord_Number[S]                                # OR USE SUB-CLASSING...
    _Plus           : @Plus_Number[S]
    _Minus          : @Minus_Number[S]
    @classmethod
    __pos__         : (S) -> S
    @classmethod
    __neg__         : (S) -> S
    @classmethod
    __mul__         : (S,S) -> S
    @classmethod
    __pow__         : (S,S) -> S
struct Ord_Number[S] (Ord[S]):
    _Number         : @Number[S]
struct Plus_Number[S] (Plus[S]):
    _Number         : @Number[S]
struct Minus_Number[S] (Minus[S]):
    _Number         : @Number[S]
    def __sub__(cls, a:S, b:S):
        return cls._Number._Plus.__add__(a, cls._Number.__neg__(b))

-----------------------------------------------------------------

protocol Plus:
    @staticmethod
    add         : (Self,Self) -> Self
protocol Minus:
    @staticmethod
    sub         : (Self,Self) -> Self
protocol Number (Plus, Minus):
    neg         : () -> Self
====>
"typeclass" Plus[S]:
    add         : (S,S) -> S
"typeclass" Minus[S]:
    sub         : (S,S) -> S
"typeclass" (Plus[S], Minus[S]) => Number[S]:
    neg         : (S) -> S

-----------------------------------------------------------------

extension int (Number):
    def __pos__(self:int) -> int:
        return self
    def __neg__(self:int) -> int:
        return primnegint(self)
    def __add__(a:int, b:int) -> int:
        return primaddint(a,b)
#    def __sub__(a:int, b:int) -> int:
#        return primaddint(a,b)
    def __mul__(a:int, b:int) -> int:
        return primmulint(a,b)
#    def __eq__(a:int, b:int) -> bool:             # WOULD TRIGGER AMBIGUITY ERROR: EXISTING EXTENSION int (Eq)
#        return primeqint(a,b)
#    def __lt__(a:int, b:int) -> bool:             # WOULD TRIGGER AMBIGUITY ERROR: EXISTING EXTENSION int (Ord)
#        return primltint(a,b)
====>
"instance" Number[int]:                             # requires: Ord[int], Plus[int], Minus[int]
    def __pos__(self:int) -> int:
        return self
    def __neg__(self:int) -> int:
        return primnegint(self)
    def __add__(a:int, b:int) -> int:
        return primaddint(a,b)
#    def __sub__(a:int, b:int) -> int:
#        return primaddint(a,b)
    def __mul__(a:int, b:int) -> int:
        return primmulint(a,b)
====>
struct Plus_int (Plus_Number[int]):
    _Number = Number_int
    def __add__(a:int, b:int) -> int:
        return primaddint(a,b)
struct Minus_int (Minus_Number[int]):
    _Number = Number_int
#    def __sub__(a:int, b:int) -> int:
#        return primsubint(a,b)
struct Number_int (Number[int]):
    _Plus = Plus_int
    _Minus = Minus_int
    def __pos__(cls, self:int) -> int:
        return self
    def __neg__(cls, self:int) -> int:              # neg(7) ====> 7.__neg__() ====> Number_int.__neg__(7) ====> primnegint(7)
        return primnegint(self)
    def __mul__(cls, a:int, b:int) -> int:
        return primmulint(a,b)

-----------------------------------------------------------------

protocol SupportsAbs:
    __abs__         : () -> Self
====>
"typeclass" SupportsAbs[S]:
    __abs__         : (S) -> S
====>
struct SupportsAbs[S]:
    @classmethod
    __abs__         : (S) -> S

-----------------------------------------------------------------

protocol Complex (Number, SupportsAbs, SupportsComplex):
    @staticmethod
    __fromcomplex__ : (complex) -> Self
    @staticmethod
    __div__         : (Self,Self) -> Self
    real            : () -> Real
    imag            : () -> Real
    conjugate       : () -> Self
====>
"typeclass" (Number[S],SupportsAbs[S],SupportsComplex[S]) => Complex[S]:
    __fromcomplex__ : (complex) -> S
    __div__         : (S,S) -> S
    real            : (Real[T]) => (S) -> T
    imag            : (Real[T]) => (S) -> T
    conjugate       : (S) -> S
====>
struct Plus_Complex[S] (Plus_Number[S]):
    _Number         : @Complex[S]
struct Minus_Complex[S] (Minus_Number[S]):
    _Number         : @Complex[S]
struct SupportsAbs_Complex[S] (SupportsAbs[S]):
    _Complex        : @Complex[S]
struct SupportsComplex_Complex[S] (SupportsComplex[S]):
    _Complex        : @Complex[S]
struct Complex[S] (Number[S]):
    _Plus           : @Plus_Complex[S]
    _Minus          : @Minus_Complex[S]
    _SuppAbs        : @SupportsAbs_Complex[S]
    _SuppComplex    : @SupportsComplex_Complex[S]

    @classmethod
    __fromcomplex__ : (complex) -> S
    @classmethod
    __div__         : (S,S) -> S
    @classmethod
    real            : (@Real[T], S) -> T
    @classmethod
    imag            : (@Real[T], S) -> T
    @classmethod
    conjugate       : (S) -> S

-----------------------------------------------------------------

protocol SupportsRound:
    __round__       : () -> int
====>
"typeclass" SupportsRound[S]:
    __round__       : (S) -> int
====>
struct SupportsRound[S]:
    @classmethod
    __round__       : (S) -> int

-----------------------------------------------------------------

protocol Real (Complex, SupportsRound, SupportsFloat):
    @staticmethod
    __fromfloat__   : (float) -> Self
    __trunc__       : () -> Integral
    __floor__       : () -> Integral
    __ceil__        : () -> Integral
    @staticmethod
    __divmod__      : (Self,Self) -> (Self,Self) 
    @staticmethod
    __floordiv__    : (Self,Self) -> Integral    
    @staticmethod
    __mod__         : (Self,Self) -> Self        

    def __complex__(a):
        return complex(float(a))
    def real(a):
        return fromfloat(float(a))
    def imag(a):
        return fromfloat(0.0)
    def conjugate(a):
        return a
    def divmod(a,b):
        return (a//b, a%b)
====>
"typeclass" (Complex[S],SupportsRound[S],SupportsFloat[S]) => Real[S]:
    __fromfloat__   : (float) -> S
    __trunc__       : (Integral[T]) => (S) -> T
    __floor__       : (Integral[T]) => (S) -> T
    __ceil__        : (Integral[T]) => (S) -> T
    __divmod__      : (S,S) -> (S,S)
    __floordiv__    : (Integral[T]) => (S,S) -> T
    __mod__         : (S,S) -> S

    def __complex__(a):
        return complex(float(a))
    def real(a):
        return a
    def imag(a):
        return 0
    def conjugate(a):
        return a
    def divmod(a,b):
        return (a//b, a%b)
====>
struct Plus_Real[S] (Plus_Complex[S]):
    _Number         : @Real[S]
struct Minus_Real[S] (Minus_Complex[S]):
    _Number         : @Real[S]
struct SupportsAbs_Real[S] (SupportsAbs_Complex[S]):
    _Complex        : @Real[S]
struct SupportsComplex_Real[S] (SupportsComplex_Complex[S]):
    _Complex        : @Real[S]
struct SupportsRound_Real[S] (SupportsRound[S]):
    _Real           : @Real[S]
struct SupportsFloat_Real[S] (SupportsFloat[S]):
    _Real           : @Real[S]
struct Real[S] (Complex[S]):
    _Plus           : @Plus_Real[S]
    _Minus          : @Minus_Real[S]
    _SuppAbs        : @SupportsAbs_Real[S]
    _SuppComplex    : @SupportsComplex_Real[S]
    _SuppRound      : @SupportsRound_Real[S]
    _SuppFloat      : @SupportsFloat_Real[S]
    
    @classmethod
    __fromfloat__   : (float) -> S
    @classmethod
    __trunc__       : (@Integral[T],S) -> T
    @classmethod
    __floor__       : (@Integral[T],S) -> T
    @classmethod
    __ceil__        : (@Integral[T],S) -> T
    @classmethod
    __divmod__      : (S,S) -> (S,S)
    @classmethod
    __floordiv__    : (@Integral[T],S,S) -> T
    @classmethod
    __mod__         : (S,S) -> S

    def __complex__(cls,a):
        return cls.__complex__(cls.__float__(a))
    def real(cls,_w,a):
        return _w.__fromfloat__(cls.__float__(a))
    def imag(cls,_w,a):
        return _w.__fromfloat__(0.0)
    def conjugate(cls,a):
        return a
    def divmod(cls,a,b):
        return (cls.__floordiv__(a,b), cls.__mod__(a,b))

-----------------------------------------------------------------

protocol Rational (Real):
    numerator       : () -> Integral
    denominator     : () -> Integral
    
    def __float__(a):
        return numerator(a) / denominator(a)
====>
"typeclass" (Real[S]) => Rational[S]:
    numerator       : (Integral[T]) => (S) -> T
    denominator     : (Integral[T]) => (S) -> T

    def __float__(a):
        return numerator(a) / denominator(a)
====>
struct Plus_Rational[S] (Plus_Real[S]):
    _Number         : @Rational[S]
struct Minus_Rational[S] (Minus_Real[S]):
    _Number         : @Rational[S]
struct SupportsAbs_Rational[S] (SupportsAbs_Real[S]):
    _Complex        : @Rational[S]
struct SupportsComplex_Rational[S] (SupportsComplex_Real[S]):
    _Complex        : @Rational[S]
struct SupportsRound_Rational[S] (SupportsRound_Real[S]):
    _Real           : @Rational[S]
struct SupportsFLoat_Rational[S] (SupportsFLoat_Real[S])
    _Real           : @Rational[S]

    def __float__(cls,a):
        Int = Integral_int
        return Real_float.__div__(Int._SuppFloat.__float__(cls._Rational.numerator(Int,a)), 
                                  Int._SuppFloat.__float__(cls._Rational.denominator(Int,a)))

struct Rational[S] (Real[S]):
    _Plus           : @Plus_Rational[S]
    _Minus          : @Minus_Rational[S]
    _SuppAbs        : @SupportsAbs_Rational[S]
    _SuppComplex    : @SupportsComplex_Rational[S]
    _SuppRound      : @SupportsRound_Rational[S]
    _SuppFloat      : @SupportsFloat_Rational[S]
    
    @classmethod
    numerator       : (@Integral[T], S) -> T
    @classmethod
    denominator     : (@Integral[T], S) -> T
    
-----------------------------------------------------------------

protocol Integral (Rational,SupportsInt,Logical):
    @staticmethod
    __fromint__     : (int) -> Self
    __index__       : () -> Self
    @staticmethod
    __lshift__      : (Self,Self) -> Self
    @staticmethod
    __rshift__      : (Self,Self) -> Self
    __invert__      : () -> Self
    
    def __index__(a):
        return int(a)
    def __lshift__(a,b):
        return fromint(int(a) << int(b))
    def __invert__(a):
        return fromint(~int(a))
    def __float__(a):
        return float(int(a))
    def numerator(a):
        return a
    def denominator(a):
        return 1
====>
"typeclass" (Rational[S],SupportsInt[S],Logical[S]) => Integral[S]:
    __fromint__     : (int) -> S
    __index__       : (S) -> S
    __lshift__      : (S,S) -> S
    __rshift__      : (S,S) -> S
    __invert__      : (S) -> S

    def __index__(a):
        return int(a)
    def __lshift__(a,b):
        return fromint(int(a) << int(b))
    def __float__(a):
        return float(int(a))
    def numerator(a):
        return a
    def denominator(a):
        return 1
====>
struct Plus_Integral[S] (Plus_Rational[S]):
    _Number         : @Integral[S]
struct Minus_Integral[S] (Minus_Rational[S]):
    _Number         : @Integral[S]
struct SupportsAbs_Integral[S] (SupportsAbs_Rational[S]):
    _Complex        : @Integral[S]
struct SupportsComplex_Integral[S] (SupportsComplex_Rational[S]):
    _Complex        : @Integral[S]
struct SupportsRound_Integral[S] (SupportsRound_Rational[S]):
    _Real           : @Integral[S]
struct SupportsFLoat_Integral[S] (SupportsFLoat_Rational[S])
    _Real           : @Integral[S]
struct SupportsInt_Integral[S] (SupportsInt[S])
    _Integral       : @Integral[S]
struct Logical_Integral[S] (Logical[S])
    _Integral       : @Integral[S]
struct Integral[S] (Rational[S]):
    _Plus           : @Plus_Integral[S]
    _Minus          : @Minus_Integral[S]
    _SuppAbs        : @SupportsAbs_Integral[S]
    _SuppComplex    : @SupportsComplex_Integral[S]
    _SuppRound      : @SupportsRound_Integral[S]
    _SuppFloat      : @SupportsFloat_Integral[S]
    _SuppInt        : @SupportsInt_Integral[S]
    _Logical        : @Logical_Integral[S]
    
    @staticmethod
    __fromint__     : (int) -> S
    __index__       : (S) -> S
    __lshift__      : (S,S) -> S
    __rshift__      : (S,S) -> S
    __invert__      : (S) -> S

    def __index__(cls,a):
        return cls._SuppInt.__int__(a)
    @classmethod
    def __lshift__(cls,a,b):
        return cls.__fromint__(Integral_int.__lshift__(cls._SuppInt.__int__(a), cls._SuppInt.__int__(b)))
    def __invert__(cls,a):
        return cls.__fromint__(Integral_int.__invert__(cls._SuppInt.__int__(a)))
    def __float__(cls,a):
        return Integral_int._SuppFloat.__float__(cls._SuppInt.__int__(a))
    def numerator(cls,_w,a):
        return _w.__fromint__(cls.__SuppInt.__int__(a))
    def denominator(cls,_w,a):
        return _w.__fromint__(0)

-----------------------------------------------------------------

extension int (Integral):
    def __fromint__(a):
        return a
    def __int__(self):
        return self
    def __and__(a,b):
        return prinandint(a,b)
====>    
"instance" Integral[int]:
    def __fromint__(a):
        return a
    def __int__(self):
        return self
    def __and__(a,b):
        return prinandint(a,b)
====>
struct Plus_Rational_int (Plus_Rational[int]):
    _Number         = Integral_int
struct Minus_Rational_int (Minus_Rational[int]):
    _Number         = Integral_int
struct SupportsAbs_Integral_int (SupportsAbs_Integral[int]):
    _Complex        = Integral_int
struct SupportsComplex_Integral_int (SupportsComplex_Integral[int]):
    _Complex        = Integral_int
struct SupportsRound_Integral_int (SupportsRound_Integral[int]):
    _Real           = Integral_int
struct SupportsFloat_Integral_int (SupportsFloat_Integral[int]):
    _Real           = Integral_int
struct SupportsInt_Integral_int (SupportsInt_Integral[int]):
    _Integral       = Integral_int
    def __int__(cls,self):
        return self
struct Logical_Integral_int (Logical_Integral[int]):
    _Integral       = Integral_int
    def __and__(cls,a,b):
        return prinandint(a,b)
struct Integral_int (Integral[int]):
    _Plus           = Plus_Integral_int
    _Minus          = Minus_Integral_int
    _SuppAbs        = SupportsAbs_Integral_int
    _SuppComplex    = SupportsComplex_Integral_int
    _SuppRound      = SupportsRound_Integral_int
    _SuppFloat      = SupportsFloat_Integral_int
    _SuppInt        = SupportsInt_Integral_int
    _Logical        = Logical_Integral_int
    def __fromint__(cls,a):
        return a

-----------------------------------------------------------------

extension bool (Logical,Ord):
    def __and__(a,b):
        return b if a else False
    def __eq__(a,b):
        return primeqbool(a,b)
    def __lt__(a,b):
        return a == False and b == True
====>
"instance" Ord[bool]
    def __eq__(a,b):
        return primeqbool(a,b)
    def __lt__(a,b):
        return a == True and b == False
"instance" Logical[bool]:
    def __and__(a,b):
        return b if a else False
====>
struct Ord_bool (Ord[bool]):
    _Logical = Logical_bool
    def __eq__(cls,a,b):
        return primeqbool(a,b)
    def __lt__(cls,a,b):
        return cls.__eq__(a,True) and cls.__eq__(b,False)
struct Logical_bool (Logical[bool]):
    _Ord = Ord_bool
    def __and__(a,b):
        return b if a else False

-----------------------------------------------------------------

extension bool (Logical):
    def __and__(a,b):
        return b if a else False
extension bool (Ord):
    def __eq__(a,b):
        return primeqbool(a,b)
    def __lt__(a,b):
        return a == False and b == True
====>
"instance" Ord[bool]
    def __eq__(a,b):
        return primeqbool(a,b)
    def __lt__(a,b):
        return a == True and b == False
"instance" Logical[bool]:
    def __and__(a,b):
        return b if a else False
====>
struct Ord_bool (Ord[bool]):
    def __eq__(cls,a,b):
        return primeqbool(a,b)
    def __lt__(cls,a,b):
        return cls.__eq__(a,True) and cls.__eq__(b,False)
struct Logical_bool (Logical[bool]):
    def __and__(a,b):
        return b if a else False

-----------------------------------------------------------------

protocol IncrementalLogical[A] (Logical):
    __iand__        : !(A) -> None
    __ior__         : !(A) -> None
    __ixor__        : !(A) -> None
====>
"typeclass" (Logical[S]) => IncrementalLogical[S,A]:
    __iand__        : !(S,A) -> None
    __ior__         : !(S,A) -> None
    __ixor__        : !(S,A) -> None
====>
struct IncrementalLogical[S,A] (Logical[S])
    @classmethod
    __iand__        : !(S,A) -> None
    @classmethod
    __ior__         : !(S,A) -> None
    @classmethod
    __ixor__        : !(S,A) -> None

-----------------------------------------------------------------

protocol IncrementalPlus[A] (Plus):
    __iadd__        : !(A) -> None
====>
"typeclass" (Plus[S]) => IncrementalPlus[S,A]:
    __iadd__        : !(S,A) -> None
====>
struct IncrementalPlus[S,A] (Plus[S]):
    @classmethod
    __iadd__        : !(S,A) -> None

-----------------------------------------------------------------

protocol IncrementalMinus[A] (Minus):
    __isub__        : !(A) -> None
====>
"typeclass" (Minus[S]) => IncrementalMinus[S,A]:
    __isub__        : !(S,A) -> None
====>
struct IncrementalMinus[S,A] (Minus[S]):
    @classmethod
    __isub__        : !(S,A) -> None

-----------------------------------------------------------------
    
protocol Foldable[A]:
    __fold__        : ((A,B)->B, B) -> B
====>
"typeclass" Foldable[S,A]:
    __fold__        : (S, (A,B)->B, B) -> B
====>
struct Foldable[S,A]:
    @classmethod
    __fold__        : (S, (A,B)->B, B) -> B

-----------------------------------------------------------------

### The collection hierarchy (from typing.py)

protocol Iterable[A]:
    __iter__        : () -> Iterator[A]
====>
"typeclass" Iterable[S,A]:
    __iter__        : (S) -> Iterator[A]
====>
struct Iterable[S,A]:
    @classmethod
    __iter__        : (S) -> Iterator[A]

-----------------------------------------------------------------

struct Iterator[A] (Iterable[A]):                   # MUTUAL RECURSION BETWEEN TYPE AND PROTOCOL
    __next__        : !() -> A
    
    def __iter__(self): return self
====>
struct Iterator[A]:
    __next__        : !() -> A
"instance" Iterable[Iterator[A],A]:
    def __iter__(self): return self
====>
struct Iterator[A]:
    __next__        : !() -> A
struct Iterable_Iterator (Iterable[Iterator[A],A]):
    @classmethod
    def __iter__(cls,self): return self

-----------------------------------------------------------------

protocol Reversible[A] (Iterable[A]):
    __reversed__    : () -> Self
====>
"typeclass"  (Iterable[S,A]) => Reversible[S,A]:
    __reversed__    : (S) -> S
====>
struct  Reversible[S,A] (Iterable[S,A]):
    @classmethod
    __reversed__    : (S) -> S

-----------------------------------------------------------------

protocol Container[A]:
    __contains__    : (A) -> bool
====>
"typeclass" Container[S,A]:
    __contains__    : (S,A) -> bool
====>
struct Container[S,A]:
    @classmethod
    __contains__    : (S,A) -> bool

-----------------------------------------------------------------

protocol Sized:
    __len__         : () -> int
====>
"typeclass" Sized[S]:
    __len__         : (S) -> int
====>
struct Sized[S]:
    @classmethod
    __len__         : (S) -> int

-----------------------------------------------------------------

protocol Indexed[A,B]:
    __getitem__     : (A) -> B
====>
"typeclass" Indexed[S,A,B]:
    __getitem__     : (S,A) -> B
====>
struct Indexed[S,A,B]:
    @classmethod
    __getitem__     : (S,A) -> B

-----------------------------------------------------------------

protocol Collection[A] (Container[A], Sized, Iterable[A]):
    __fromiter__    : (Iterable[A]) -> Self
====>
"typeclass" (Container[S,A], Sized[S], Iterable[S,A]) => Collection[S,A]:
    __fromiter__    : (Iterable[T,A]) => (T) -> S
====>
struct Sized_Collection[S,A] (Sized[S]):
    _Collection     : @Collection[A]
struct Iterable_Collection[S,A] (Iterable[S,A]):
    _Collection     : @Collection[A]
struct Collection[S,A] (Container[S,A]):
    _Sized          : @Sized_Collection[S,A]
    _Iterable       : @Iterable_Collection[S,A]

    @classmethod
    __fromiter__    : (@Iterable[A],T) -> S

-----------------------------------------------------------------

protocol Set[A] (Collection[A], Ord, Logical, Minus):
    isdisjoint      : (Set[A]) -> bool

    def __le__(a: Self, b: Self):
        if len(a) > len(b):
            return False
        for elem in a:
            if elem not in b:
                return False
        return True
    def __lt__(a: Self, b: Self):
        if len(a) >= len(b):
            return False
        for elem in a:
            if elem not in b:
                return False
        return True
    def __eq__(a: Self, b: Self):
        return len(a) == len(b) and a <= b
    def __or__(a: Self, b: Self):
        chain = [ e for s in [a,b] for e in s ]
        return fromiter(chain)
    def isdisjoint(self: Self, other: Set[A]):
        for value in b:
            if value in self:
                return False
        return True
====>
"typeclass" (Collection[S,A], Ord[S], Logical[S], Minus[S]) => Set[S,A]:
    isdisjoint      : (Set[T,A]) => (S,T) -> bool

    def __le__(a: S, b: S):
        if len(a) > len(b):
            return False
        for elem in a:
            if elem not in b:
                return False
        return True
    def __lt__(a: S, b: S):
        if len(a) >= len(b):
            return False
        for elem in a:
            if elem not in b:
                return False
        return True
    def __eq__(a: S, b: S):
        return len(a) == len(b) and a <= b
    def __or__(a: S, b: S):
        chain = [ e for s in [a,b] for e in s ]
        return fromiter(chain)
    def (Set[T,A]) => isdisjoint(self: S, other: T):
        for value in b:
            if value in self:
                return False
        return True
====>
struct Sized_Set[S,A] (Sized_Collection[S]):
    _Set            : @Set[S,A]
struct Iterable_Set[S,A] (Iterable_Collection[S,A]):
    _Set            : @Set[S,A]
struct Ord_Set[S,A] (Ord[S,A]):
    _Set            : @Set[S,A]
    def __le__(cls, a:S, b:S):
        if Ord_int.__gt__(cls._Set._Sized.__len__(a), cls._Set._Sized.__len__(b)):
            return False
        for elem in cls._Set._Iterable.__iter__(a):
            if not cls._Set.__contains__(b,elem):
                return False
        return True
    def __lt__(cls, a:S, b:S):
        if Ord_int.__ge__(cls._Set._Sized.__len__(a), cls._Set._Sized.__len__(b)):
            return False
        for elem in cls._Set._Iterable.__iter__(a):
            if not cls._Set.__contains__(b,elem):
                return False
        return True
    def __eq__(cls, a: S, b: S):
        return Eq_int.__eq__(cls._Set._Sized.__len__(a), cls._Set._Sized.__len__(b)) and cls.__le__(a,b)
struct Logical_Set[S,A] (Logical[S]):
    _Set            : @Set[S,A]
    def __or__(cls, a: S, b: S):
        chain = [ e for s in Iterable_list.__iter__([a,b]) for e in cls._Set._Iterable.__iter__(s) ]
        return cls.__fromiter__(Iterable_list,chain)
struct Minus_Set[S,A] (Minus[S]):
    _Set            : @Set[S,A]
struct Set[S,A] (Collection[S,A]):
    _Sized          : @Sized_Set[S,A]
    _Iterable       : @Iterable_Set[S,A]
    _Ord            : @Ord_Set[S,A]
    _Logical        : @Logical_Set[S,A]
    _Minus          : @Minus_Set[S,A]
    
    @classmethod
    isdisjoint      : (@Set[T,A],S,T) -> bool

    def isdisjoint(cls:@Self, _q:@Set[T,A]), self:S, other:T):
        for value in _q._Iterable.__iter__(other):
            if cls.__contains__(value):
                return False
        return True

-----------------------------------------------------------------

extension Set[A (Hashable)] (Hashable):
    def __hash__(self):
        n = 12345
        for k in self:
            n += hash(k)
        return n
====>
"instance" (Hashable[A], Set[S,A]) => Hashable[S]:
    def __hash__(self):
        n = 12345
        for k in self:
            n += hash(k)
        return n
====>

def Hashable_Set[S,A] (_qual: @Hashable[A], _Set: @Set[S,A]) -> @Hashable[S]:
    
    struct Hashable_Set[S,A] (Hashable[S]):    
        def __hash__(cls: struct Self, self:S):
            n = 12345
            for k in _Set._Iterable.__iter__(self):
                n += _qual.__hash__(k)
            return n
    
    return Hashable_Set

-----------------------------------------------------------------

struct frozenset[A (Hashable)] (Set[A], Hashable):
    _val        : frozendict[A,()]                          # HYPOTHETICAL!

    def __contains__(self, a):
        return a in self._val.keys()
    def __fromiter__(items: Iterable[A]):
        return frozenset(_val = { i:() for i in items })
    def __len__(self):
        return len(self._val)
    def __iter__(self):
        return iter(self._val.keys())
====>
"qualified struct" (Hashable[A]) => frozenset[A]:
    _val        : frozendict[A,()]
    
"instance" (Hashable[A]) => Set[frozenset[A],A]:
    def __contains__(self, a):
        return a in self._val.keys()
    def (Iterable[T,A]) => __fromiter__(items: T):
        return frozenset(_val = { i:() for i in items })

    def __len__(self):
        return len(self._val)
    def __iter__(self):
        return iter(self._val.keys())
"instance" (Hashable[A]) => Hashable[frozenset[A]]:
    def __hash__(self): ...
        n = 12345
        for i in self._val.keys():
            n += hash(i)
        return n
====>
struct frozenset[A]:
    _qual           = Hashable
    _val            : frozendict[A,()]

def Set_frozenset[A](_qual: @Hashable[A]) -> @Set[frozenset[A],A]:
    
    struct Sized_frozenset[A] (Sized_Set[frozenset[A],A]):
        _Collection     = Set_frozenset
        _Set            = Set_frozenset
        def __len__(cls,self):
            return len(self._val)
    struct Iterable_frozenset[A] (Iterable_Set[frozenset[A],A]):
        _Collection     = Set_frozenset
        _Set            = Set_frozenset
        def __iter__(cls,self):
            return iter(self._val.keys())
    struct Ord_frozenset[A] (Ord_Set[frozenset[A],A]):
        _Set            = Set_frozenset
    struct Logical_frozenset[A] (Logical_Set[frozenset[A],A]):
        _Set            = Set_frozenset
    struct Minus_frozenset[A] (Minus_Set[frozenset[A],A]):
        _Set            = Set_frozenset
    struct Set_frozenset[A] (Set[frozenset[A],A]):
        _Sized          = Sized_frozenset
        _Iterable       = Iterable_frozenset
        _Ord            = Ord_frozenset
        _Logical        = Logical_frozenset
        _Minus          = Minus_frozenset
        def __contains__(cls,self,a):
            return Mapping_frozendict.__contains__(self._val,a)
        def __fromiter__(_qual: @Iterable[T,A], items: T):
            return frozenset(_val = { i:() for i in _qual.__iter__(items) })

    return Set_frozenset

def Hashable_frozenset[A](_qual: @Hashable[A]) -> @Hashable[frozenset[A]]:
    struct Hashable_frozenset[A] (Hashable[frozenset[A]]):
        def __hash__(cls, self): ...
            n = 12345
            for k in Mapping_frozendict._Iterable.__iter__(self._val):
                n += _qual.__hash__(k)
            return n
    
    return Hashable_frozenset

---

Sized_frozenset_str = Sized_frozenset(Hashable_str)
Hashable_frozenset_str = Hashable_Set(Hashable_str, Set_frozenset_str)

---

x : frozenset[str]
x = frozenset(["apa","bepa"])
y = hash(x)
z = x.single()
====>
x : frozenset[str]
x = frozenset(["apa","bepa"])
y = __hash__(x)
z = x.single()
====>
x : frozenset[str]
x = frozenset(["apa","bepa"])
y = Hashable_frozenset_str.__hash__(x)
z = frozenset.single(x)

---

struct coloredset[A (Hashable)] (frozenset[A]):
    color           : str
    
    def isblue(self):
        return self.color == "blue"
====>
"qualified struct" (Hashable[A]) => coloredset[A] (frozenset[A])
    color           : str
    
    def isblue(self):
        return self.color == "blue"
====>
struct coloredset[A] (frozenset[A])
    color           : str

    def isblue      : () -> bool
    def isblue(self:Self):
        return self.color == "blue"

-----------------------------------------------------------------

protocol MutableIndexed[A,B] (Indexed[A,B]):
    __setitem__     : !(A,B) -> None
    __delitem__     : !(A) -> None
====>
"typeclass" (Indexed[S,A,B]) => MutableIndexed[S,A,B]:
    __setitem__     : !(S,A,B) -> None
    __delitem__     : !(S,A) -> None
====>
struct MutableIndexed[S,A,B] (Indexed[S,A,B]):
    @classmethod
    __setitem__     : !(S,A,B) -> None
    @classmethod
    __delitem__     : !(S,A) -> None

-----------------------------------------------------------------

protocol MutableSet[A] (Set[A], IncrementalLogical[Set[A]], IncrementalMinus[Set[A]]):
    add             : !(A) -> None
    pop             : !() -> A
    clear           : !() -> None
    
    def clear(self: Self):
        try:
            while True:
                self.pop()
        except KeyError:
            pass

    def __ior__(self: Self, other: Set[A]):
        for value in other:
            self.add(value)
====>
"typeclass" (Set[S,A], IncrementalLogical[S,EXISTS(T)[Set[T,A]], IncrementalMinus[S,EXISTS(T)[Set[T,A]]) => MutableSet[S,A]:
    add             : !(A,A) -> None
    pop             : !(S) -> A
    clear           : !(S) -> None
    def clear(self: S):
        try:
            while True:
                self.pop()
        except KeyError:
            pass
    def __ior__(self: S, other: Set[A]):
        for value in other:
            self.add(value)
====>
struct Sized_MutableSet[S,A] (Sized_Set[S,A]):
    _MutableSet         : @MutableSet[S,A]
struct Iterable_MutableSet[S,A] (Iterable_Set[S,A]):
    _MutableSet         : @MutableSet[S,A]
struct Ord_MutableSet[S,A] (Ord_Set[S,A]):
    _MutableSet         : @MutableSet[S,A]
struct Logical_MutableSet[S,A] (Logical_Set[S,A]):
    _MutableSet         : @MutableSet[S,A]
struct Minus_MutableSet[S,A] (Minus_Set[S,A]):
    _MutableSet         : @MutableSet[S,A]
struct IncrementalLogical_MutableSet[S,A] (IncrementalLogical[S,EXISTS(T)[Set[T,A]]):
    _MutableSet         : @MutableSet[S,A]
    def __ior__(cls,self: S, other: EXISTS(T)[Set[T,A]):
        for value in other._protocol._Iterable.__iter__(other._element):
            cls._MutableSet.add(value)
struct IncrementalMinus_MutableSet[S,A] (IncrementalMinus[S,EXISTS(T)[Set[T,A]]):
    _MutableSet         : @MutableSet[S,A]
struct MutableSet[S,A] (Set[S,A]):
    _Sized              : @Sized_MutableSet[S,A]
    _Iterable           : @Iterable_MutableSet[S,A]
    _Ord                : @Ord_MutableSet[S,A]
    _Logical            : @Logical_MutableSet[S,A]
    _Minus              : @Minus_MutableSet[S,A]
    _IncrementalLogical : @IncrementalLogical_MutableSet[S,A]
    _IncrementalMinus   : @IncrementalMinus_MutableSet[S,A]

    add             : !(S,A) -> None
    pop             : !(S) -> A
    @classmethod
    def clear(cls, self):
        try:
            while True:
                cls.pop(self)
        except KeyError:
            pass


-----------------------------------------------------------------

# Type-based transformations:
# 
# * Every protocol method not mentioning 'Self' is classified as an "instance" method
# * A protocol method mentioning 'Self' is classified as a static method
# * Every protocol "instance" method gets an extra first argument of type 'Self'
# * Every protocol decl turns into a "typeclass" with an additional first type parameter 'Self'
# * Every protocol base "class" turns into a "typeclass" ancestor (with 'Self' inserted first)
# * In every type scheme: 
#   * Every type expression that is a protocol application is replaced by a new universally 
#     quantified variable and a corresponding protocol constraint in the schema constaint tuple
#   * Every type variable with an explicit protocol bound has its bound moved to the schema constraint tuple
#   * Nested types not occurring in type schemas of their own are treated as being part of the enclosing schema
# * In every struct or class:
#   * Protocol bases turn into separate "instance" declarations for each protocol, with the declared struct/class
#     inserted as the first protocol type argument
#   * Every struct/class parameter with an explicit protocol bound has its bound moved into a constraint on the
#     struct/class constructor function
# * In every protocol extension:
#   * Every base protocol turns into an "instance" declaration for a fresh type variable, with the protocol constraint
#     on the new variable inserted as the first (last?) among the "instance" preconditions
# * 
# * Protocols with overlapping method names are disallowed (loosen this?)
# * Every selection expression mentioning a protocol "instance" method name generates the corresponding protocol constraint
# * Every variable expression mentioning a static protocol method name generates the corresponding protocol constraint
# * Constraint resolution and witness translation proceeds as in Haskell
    
# Superclass constraint (C) => means
# 
# * Additional function parameter                      (in a function/method type scheme)
# * Additional constructor parameter (and field)       (in a plain struct/class definition)
# * Additional constructor parameter (and field)       (in an "instance" (=struct) definition)
# * Additional "instance" existence requirement        (in a "typeclass" (=struct) definition)



-----------------------------------------------------------------

class set[A (Hashable)] (MutableSet[A]):
    def __len__(self): ...
    def __contains__(self, a): ...
    def __iter__(self): ...

    def __eq__(self, other): ...
    def __lt__(self, other): ...
    def __or__(self, other): ...

    def add(self, a): ...
    def update(self, other): ...
    def __ior__(self, other): ...
    def copy(self): ...
====>
"qualified class" (Hashable[A]) => set[A]:
    def __len__(self): ...
    def __contains__(self, a): ...
    def __iter__(self): ...

    def __eq__(self, other): ...
    def __lt__(self, other): ...
    def __or__(self, other): ...

    def add(self, a): ...
    def update(self, other): ...
    def __ior__(self, other): ...
    def copy(self): ...

"instance" (Hashable[A]) => MutableSet[set[A],A]:
    ...
====>
class set[A] (object):
    _qual           : @Hashable[A]
    ...

def MutableSet_set[A] (_qual: @Hashable[A]) -> @MutableSet[set[A],A]:
    struct MutableSet_set[A] (MutableSet[A]):
        ...        
    return MutableSet_set

-----------------------------------------------------------------

### More collections from typing.py

protocol Mapping[A,B] (Collection[A], Indexed[A,B]):
    keys            : () -> Iterator[A]
    values          : () -> Iterator[B]
    items           : () -> Iterator[(A,B)]
====>
"typeclass" (Collection[S,A], Indexed[S,A,B]) => Mapping[S,A,B]:
    keys            : (S) -> Iterator[A]
    values          : (S) -> Iterator[B]
    items           : (S) -> Iterator[(A,B)]
====>
struct Sized_Mapping[S,A,B] (Sized_Collection[S]):
    _Collection     = Mapping
    _Mapping        = Mapping
struct Iterable_Mapping[S,A,B] (Iterable_Collection[S,A]):
    _Collection     = Mapping
    _Mapping        = Mapping
struct Indexed_Mapping[S,A,B] (Indexed[S,A,B]):
    _Mapping        = Mapping
struct Mapping[S,A,B] (Collection[S,A]):
    _Sized          = Sized_Mapping
    _Iterable       = Iterable_Mapping
    _Indexed        = Indexed_Mapping
    
    @classmethod
    keys            : (S) -> Iterator[A]
    @classmethod
    values          : (S) -> Iterator[B]
    @classmethod
    items           : (S) -> Iterator[(A,B)]

-----------------------------------------------------------------

extension frozendict[A(Hashable),B] (Mapping[A,B]):
    def keys(self):
        ...
    ...
====>
"instance" (Hashable[A]) => Mapping[frozendict[A,B],A,B]:
    def keys(self):
        ...
    ...
====>
def Mapping_frozendict[A,B] (_qual: @Hashable[A]) -> @Mapping[frozendict[A,B],A,B]:
    
    struct Sized_Mapping_frozendict[A,B] (Sized_Mapping[frozendict[A,B],A,B]):
        _Collection     = Mapping_frozendict
        _Mapping        = Mapping_frozendict
        ...
    struct Iterable_Mapping_frozendict[A,B] (Iterable_Mapping[frozendict[A,B],A,B]):
        _Collection     = Mapping_frozendict
        _Mapping        = Mapping_frozendict
        ...
    struct Indexed_Mapping_frozendict[A,B] (Indexed_Mapping[frozendict[A,B],A,B]):
        _Mapping        = Mapping_frozendict
        ...
    struct Mapping_frozendict[A,B] (Mapping[frozendict[A,B],A,B]):
        _Sized          = Sized_Mapping_frozendict
        _Iterable       = Iterable_Mapping_frozendict
        _Indexed        = Indexed_Mapping_frozendict

        def keys(cls, self):
            ...
        ...
        
    return Mapping_frozendict

---

Mapping_frozendict_str = Mapping_frozendict(Hashable_str)

-----------------------------------------------------------------

extension Mapping[A, B(Eq)] (Eq):
    def __eq__(a: Self, b: Self):
        if len(a) != len(b):
            return False
        for i in a:
            if i not in b:
                return False
            if a[i] /= b[i]:
                return False
        return True
====>
"instance" (Eq[B], Mapping[S,A,B]) => Eq[S]
    def __eq__(a: S, b: S):
        if len(a) != len(b):
            return False
        for i in a:
            if i not in b:
                return False
            if a[i] /= b[i]:
                return False
        return True
====>
def Eq_Mapping[S,A,B] (_qual: @Eq[B], _Mapping: @Mapping[S,A,B]) -> @Eq[S]:

    struct Eq_Mapping[S,A,B] (Eq[S])
        def __eq__(cls, a: S, b: S):
            if Eq_int.__ne__(_Mapping._Sized.__len__(a), _Mapping._Sized.__len__(b))
                return False
            for k in _Mapping._Iterable.__iter__(a):
                if not _Mapping.__contains__(b,k):
                    return False
                if _qual.__ne__(_Mapping._Indexed.__getitem__(a,k), _Mapping._Indexed.__getitem__(b,k)):
                    return False
            return True
    
    return Eq_Mapping
    
---

def eqdict(a:frozendict[str,int], b:frozendict[str,int]) -> bool:
    return a == b
====>
def eqdict(a:frozendict[str,int], b:frozendict[str,int]) -> bool:
    return a == b
====>
def eqdict(a:frozendict[str,int], b:frozendict[str,int]) -> bool:
    Eq_Mapping_frozendict_str_int = Eq_Mapping(Eq_int, Mapping_frozendict(Hashable_str)
    return Eq_Mapping_frozendict_str_int.__eq__(a,b)

-----------------------------------------------------------------

protocol MutableMapping[A,B] (Mapping[A,B], MutableIndexed[A,B]):
    update          : !(Mapping[A,B]) -> None
====>
"typeclass" (Mapping[S,A,B], MutableIndexed[S,A,B]) => MutableMapping[S,A,B]:
    update          : (Mapping[T,A,B]) => !(S,T) -> None
====>
struct Sized_MutableMapping[S,A,B] (Sized_Mapping[S]):
    _MutableMapping : @MutableMapping[S,A,B]
struct Iterable_MutableMapping[S,A,B] (Iterable_Mapping[S,A]):
    _MutableMapping : @MutableMapping[S,A,B]
struct Indexed_MutableMapping[S,A,B] (Indexed_Mapping[S,A,B]):
    _MutableMapping : @MutableMapping[S,A,B]
struct MutableIndexed_MutableMapping[S,A,B] (MutableIndexed[S,A,B]):
    _MutableMapping : @MutableMapping[S,A,B]
struct MutableMapping[S,A,B] (Mapping[S,A,B]):
    _Sized          : @Sized_MutableMapping[S,A,B]
    _Iterable       : @Iterable_MutableMapping[S,A,B]
    _Indexed        : @Indexed_MutableMapping[S,A,B]
    _MutableIndexed : @MutableIndexed_MutableMapping[S,A,B]
    
    @classmethod
    update          : !(@Mapping[T,A,B],S,T) -> None

-----------------------------------------------------------------

protocol Sequence[A] (Collection[A], Indexed[int,A], Reversible[A], Plus):
    def __add__(self, other):       ...
    def __contains__(self, key):    ...
    def __iter__(self):             ...
    def __reversed__(self):         ...
====>
"typeclass" (Collection[S,A], Indexed[S,int,A], Reversible[S,A], Plus[S]) => Sequence[S,A]:
    def __add__(self, other):       ...
    def __contains__(self, key):    ...
    def __iter__(self):             ...
    def __reversed__(self):         ...
====>
struct Sized_Sequence[S,A] (Sized_Collection[S]):
    _Collection     = Sequence
    _Sequence       = Sequence
    @classmethod
    def __contains__(cls, self, key): ...
struct Iterable_Sequence[S,A] (Iterable_Collection[S,A]):
    _Collection     = Sequence
    _Sequence       = Sequence
    @classmethod
    def __iter__(cls, self): ...
struct Indexed_Sequence[S,A] (Indexed[S,int,A]):
    _Sequence       = Sequence
struct Reversible_Sequence[S,A] (Reversible[S,A]):
    _Sequence       = Sequence
    @classmethod
    def __reversed__(cls, self): ...
struct Plus_Sequence[S,A] (Plus[S]):
    _Sequence       = Sequence
    @classmethod
    def __add__(cls, self, other): ...
struct Sequence[S,A] (Collection[S,A]):
    _Sized          = Sized_Sequence
    _Iterable       = Iterable_Sequence
    _Indexed        = Indexed_Sequence
    _Reversible     = Reversible_Sequence
    _Plus           = Plus_Sequence

-----------------------------------------------------------------

extension Sequence[A (Eq)] (Eq):
    def __eq__(a: Self, b: Self):
        if len(a) != len(b):
            return False
        for i in range(0,len(a)):
            if a[i] != b[i]:
                return False
        return True
====>
"instance" (Eq[A], Sequence[S,A]) => Eq[S]:
    def __eq__(a: S, b: S):
        if len(a) != len(b):
            return False
        for i in range(0,len(a)):
            if a[i] != b[i]:
                return False
        return True
====>
def Eq_Sequence[S,A] (_qual: @Eq[A], _Sequence: @Sequence[S,A]) -> Eq[S]:

    struct Eq_Sequence[S,A] (Eq[S]):
        def __eq__(cls, a: S, b: S):                # OLD METHOD, SIGNATURE IS FIXED, CAN'T TAKE _qual AS A DIRECT ARGUMENT
            if Eq_int.__ne__(cls._Sequence._Sized.__len__(a), cls._Sequence._Sized.__len__(b)):
                return False
            for i in Iterable_range.__iter__(range(0,cls._Sequence._Sized.__len__(a))):
                if cls._qual.__ne__(cls._Sequence._Indexed.__getitem__(a,i), cls._Sequence._Indexed.__getitem__(b,i)):
                    return False
            return True

    return Eq_Sequence

-----------------------------------------------------------------

extension Sequence[A (Eq)]:                                             # EXTENDED PROTOCOL WITH NEW METHODS AND TIGHTER PRE-CONDITION
    index           : (A) -> int?       
    count           : (A) -> int
    
    def index(self, val):
        for i,x in enumerate(self):
            if x == val:
                return i
        return None
    def count(self, val):
        c = 0
        for x in self:
            if x == val:
                c += 1
        return c
f : [A(Eq),S(Sequence[A])] => (S,A) -> int                              # INFERRED
def f(s,a):
    return s.count(a)
====>
"typeclass" (Eq[A], Sequence[S,A]) => Sequence_EXT[S,A]:
    index           : (S,A) -> int?
    count           : (S,A) -> int
    def index(self, val):
        for i,x in enumerate(self):
            if x == val:
                return i
        return None
    def count(self, val):
        c = 0
        for x in self:
            if x == val:
                c += 1
        return c
f : (Eq[A],Sequence[S,A], Sequence_EXT[S,A]) => (S,A) -> int            # INFERRED
f : (Eq[A],Sequence[S,A]) => (S,A) -> int                               # CONTEXT-REDUCED (ALWAYS REDUCE AN EXTENDED PROTOCOL)
def f(s,a):
    return s.count(a)
====>
struct Sequence_EXT[S,A]:
    _Eq             : @Eq[A]
    _Sequence       : @Sequence[S,A]
    @classmethod
    index           : (S,A) -> int?
    @classmethod
    count           : (S,A) -> int

    def index(cls, self, val):
        for i,x in enumerate(cls._Sequence._Iterable.__iter__(self)):
            if cls._Eq.__eq__(x,val):
                return i
        return None
    def count(cls, self, val):
        c = 0
        for x in cls._Sequence._Iterable.__iter__(self):
            if cls._Eq.__eq__(x,val):
                c += 1
        return c

def f(_Sequence_EXT:@Sequence_EXT[S,A], s:S, a:A):
    return _Sequence_EXT.count(s,a)

def f(_Eq:@Eq[A], _Sequence:@Sequence[S,A], s:S, a:A):
    struct Sequence_EXT1[S,A] (Sequence_EXT[S,A]):
        _Eq       = _Eq
        _Sequence = _Sequence
    return _Sequence_EXT1.count(s,a)

### OR ####

struct Sequence_EXT[S,A]:
    @classmethod
    index           : (@Eq[A], @Sequence[S,A], S, A) -> int?            # NEW METHOD, SIGNATURE CAN BE ADAPTED
    @classmethod
    count           : (@Eq[A], @Sequence[S,A], S, A) -> int             # NEW METHOD, SIGNATURE CAN BE ADAPTED

    def index(cls, _Eq: @Eq[A], _Sequence:@Sequence[S,A], self:S, val:A) -> int?:
        for i,x in enumerate(_Sequence._Iterable.__iter__(self)):
            if _Eq.__eq__(x,val):
                return i
        return None
    def count(cls, _Eq: @Eq[A], _Sequence:@Sequence[S,A], self:S, val:A) -> int:
        c = 0
        for x in _Sequence._Iterable.__iter__(self):
            if _Eq.__eq__(x,val):
                c += 1
        return c

def f(_Eq:@Eq[A], _Sequence:@Sequence[S,A], s:S, a:A):
    return Sequence_EXT.count(_Eq,_Sequence,s,a)

-----------------------------------------------------------------

protocol MutableSequence[A] (Sequence[A], MutableIndexed[int,A], IncrementalPlus[Sequence[A]]):
    append          : !(A) -> None
    __iadd__        : !(Sequence[A]) -> None
    
    def append(self, other):
        ...
    def __iadd__(self, other):
        self.append(other)
====>
"typeclass" (Sequence[S,A], MutableIndexed[S,int,A], IncrementalPlus[S,Sequence[T,A]]) => MutableSequence[S,A]:
    append          : !(S,A) -> None
    __iadd__        : !(S,    Sequence[T,A]) -> None

    def append(self, other):
        ...
    def __iadd__(self, other):
        for v in other:
            self.append(v)
====>
struct Sized_MutableSequence[S,A] (Sized_Sequence[S]):
    _Collection         = MutableSequence
    _Sequence           = MutableSequence
    _MutableSequence    = MutableSequence
struct Iterable_MutableSequence[S,A] (Iterable_Sequence[S,A]):
    _Collection         = MutableSequence
    _Sequence           = MutableSequence
    _MutableSequence    = MutableSequence
struct Indexed_MutableSequence[S,A] (Indexed_Sequence[S,int,A]):
    _Sequence           = MutableSequence
    _MutableSequence    = MutableSequence
struct Reversible_MutableSequence[S,A] (Reversible_Sequence[S,A]):
    _Sequence           = MutableSequence
    _MutableSequence    = MutableSequence
struct Plus_MutableSequence[S,A] (Plus[S]):
    _Sequence           = MutableSequence
    _MutableSequence    = MutableSequence
struct MutableIndexed_MutableSequence[S,A] (MutableIndexed[S,int,A]):
    _MutableSequence    = MutableSequence
struct IncrementalPlus_MutableSequence[S,A] (IncrementalPlus[S,    Sequence[T,A]    ]):  #####################
    _MutableSequence    = MutableSequence

    __iadd__            : !(S,    Sequence[T,A]    ) -> None                             #####################

    def __iadd__(self, other):
        other_val : TTTTT = some_other.val
        other_w : Sequence[TTTTT,A] = some_other.proto
        for v in other_w._Iterable.__iter__(other_val):
            self.append(v)

struct MutableSequence[S,A] (Sequence[S,A]):
    _Sized              = Sized_MutableSequence
    _Iterable           = Iterable_MutableSequence
    _Indexed            = Indexed_MutableSequence
    _Reversible         = Reversible_MutableSequence
    _Plus               = Plus_MutableSequence
    _MutableIndexed     = MutableIndexed_MutableSequence
    _IncrementalPlus    = IncrementalPlus_MutableSequence[S,A])

    @classmethod
    append              : !(S,A) -> None

    def append(cls, self, val):
        ...

-----------------------------------------------------------------

protocol Apa:
    apa         : () -> int
    def apa(self):
        return 1
protocol Bepa (Apa):
    bepa        : () -> int
    def apa(self):
        return 2
    def bepa(self):
        return 20 + self.apa()
protocol Cepa (Apa):
    cepa        : () -> int
    def apa(self):
        return 3
    def cepa(self):
        return 30 + self.apa()
protocol Depa (Bepa, Cepa):
    depa        : () -> int
    def depa(self):
        return (self.bepa(), self.apa(), self.cepa(), Apa.apa(self))
====>
"typeclass" Apa[S]:
    apa         : (S) -> int
    def apa(self):
        return 1
"typeclass" (Apa[S]) => Bepa[S]:
    bepa        : (S) -> int
    def apa(self):                      # NOT POSSIBLE IN HASKELL
        return 2                        #
    def bepa(self):
        return 20 + self.apa()
"typeclass" (Apa[S]) => Cepa[S]:
    cepa        : (S) -> int
    def apa(self):                      # NOT POSSIBLE IN HASKELL
        return 3                        #
    def cepa(self):
        return 30 + self.apa()
"typeclass" (Bepa[S], Cepa[S]) => Depa[S]:
    depa        : (S) -> int
    def depa(self):
        return (self.bepa(), self.apa(), self.cepa(), Apa.apa(self))
====>
struct Apa[S]:
    @classmethod
    apa         : (S) -> int
    def apa(cls,self):
        return 1

struct Bepa[S]:
    _Apa        : @Apa[S]       = Apa
    @classmethod
    bepa        : (S) -> int
    def bepa(cls,self):
        return 20 + cls._Apa.apa(self)
struct Apa_Bepa[S] (Apa[S]):
    _Bepa       : @Bepa[S]      = Bepa
#    def apa(cls,self):
#        return 2
    pass

struct Cepa[S]:
    _Apa        : @Apa[S]       = Apa
    @classmethod
    cepa        : (S) -> int
    def cepa(cls,self):
        return 30 + cls.apa(self)
struct Apa_Cepa[S] (Apa[S]):
    _Cepa       : @Cepa[S]      = Cepa
    def apa(cls,self):
        return 3

struct Depa[S]:
    _Bepa       : @Bepa[S]      = Bepa_Depa
    _Cepa       : @Cepa[S]      = Cepa_Depa
    @classmethod
    depa        : (S) -> int
    def depa(cls,self):
        return (cls._Bepa.bepa(self), cls._Bepa._Apa.apa(self), cls._Cepa.cepa(self), Apa.apa(self))
struct Bepa_Depa[S] (Bepa[S]):
    _Apa        = Apa_Depa
    _Depa       : @Depa[S]      = Depa
struct Cepa_Depa[S] (Cepa[S]):
    _Apa        = Apa_Depa
    _Depa       : @Depa[S]      = Depa
struct Apa_Depa[S] (Apa_Bepa[S])
    apa         = Apa_Cepa.apa                  # DIAMON PROBLEM RESOLUTION

-----------------------------------------------------------------

extension int (Depa):
    pass
====>
"instance" Depa[int]:
    pass
====>
Depa_int = Depa
Cepa_int = Depa._Cepa                           # BECAUSE NO SEPARATE EXTENSIONS EXIST
Bepa_int = Depa._Bepa                           #               -"-
Apa_int  = Depa._Bepa._Apa                      #               -"-

7.depa() ====> 
Depa_int.depa(7) ====> 
Depa.depa(7) ====> 
(Depa._Bepa.bepa(self), Depa._Bepa._Apa.apa(self), Depa._Cepa.cepa(self), Apa.apa(self)) ====> 
(Bepa_Depa.bepa(self), Apa_Depa.apa(self), Cepa_Depa.cepa(self), 1) ====> 
(Bepa.bepa(self), Apa_Cepa.apa(self), Cepa.cepa(self), 1) ====> 
(20+Bepa_Depa._Apa.apa(), 3, 30+Cepa_Depa._Apa.apa(), 1) ====>
(20+Apa_Depa.apa(), 3, 30+Apa_Depa.apa(), 1) ====>
(20+Apa_Cepa.apa(), 3, 30+Apa_Cepa.apa(), 1) ====>
(20+3, 3, 30+3, 1) ====>
(23, 3, 33, 1)