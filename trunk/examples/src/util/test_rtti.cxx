#include <stdio.h>
#include <assert.h>
#include <plib/ulRTTI.h>

class A
{
public:
  virtual void foo () {} //make this a polymorphic type
  UL_TYPE_DATA
};

class B
{
public:
  UL_TYPE_DATA
};

class C : public A
{
public:
  UL_TYPE_DATA
};

class D : public A, public B
{
public:
  UL_TYPE_DATA
};

class E : public D
{
public:
  UL_TYPE_DATA
};

UL_TYPE_DEF(A,"A")
UL_TYPE_DEF(B,"B")
UL_TYPE_DEF1(C,"C",A)
UL_TYPE_DEF2(D,"D",A,B)
UL_TYPE_DEF1(E,"E",D)

void main(void)
{
  C c;
  D d;
  E e;

  /* verify the class hierarchy */
  assert( UL_ISA(C,&c) ) ;
  assert( UL_ISKINDOF(A,&c) ) ;
  assert( UL_ISA(D,&d) ) ;
  assert( UL_ISKINDOF(A,&d) ) ;
  assert( UL_ISKINDOF(B,&d) ) ;
  assert( UL_ISA(E,&e) ) ;
  assert( UL_ISKINDOF(D,&e) ) ;
  assert( UL_ISKINDOF(A,&e) ) ;
  assert( UL_ISKINDOF(B,&e) ) ;

  /* test a case that shouldn't work */
  assert( !UL_ISA(C,&e) ) ;

  /* now do something we might try in an application */
  A* ap = &e;
  E* ep = UL_CAST(E,ap) ;
  if ( ep )
    printf("it works!\n");
}