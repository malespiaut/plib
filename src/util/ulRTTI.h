#ifndef _INCLUDED_ULRTTI_H_
#define _INCLUDED_ULRTTI_H_

/*
You should define UL_RTTI_EMULATION if the compiler does not support RTTI
*/
//#define UL_RTTI_EMULATION

/*
**  portable RTTI
**
**  In order to enable RTTI features for a class, two things should be done:
**
**  1)	insert the text UL_TYPE_DATA (without ';') as the last item in the class-decl.
**  2)	in the .C file where the class's implementation resides, insert the following (without';'):
**		
**  UL_TYPE_DEF(classname,"classname")	     if the class has no bases with RTTI
**  UL_TYPE_DEFn(classname,"classname",b1,...bn) if the class has bases b1,...bn with RTTI
**
**  This code is a modified version of Alexandru Telea's <alext@win.tue.nl> RTTI library.
**    <http://www.win.tue.nl/math/an/alext/ALEX/C++/cplusplus.html>
**  Run-time object creation was removed for efficiency and names were mangled.
**  ---
**  Dave McClurg <dpm@efn.org>
*/

#include <string.h>

#ifdef UL_RTTI_EMULATION

/*
Bummer... the compiler does *not* support RTTI so we must emulate RTTI
*/

class ulTypeId;

class ulTypeInfo
{
private:
  friend class ulTypeId;

  char* n;  //type name 
  const ulTypeInfo** b; //base types (NULL-ended array)
  int ns; //#subtypes of this type
  const ulTypeInfo** subtypes; //types derived from this type
  static const ulTypeInfo null_type;			//convenience type info for a 'null' type

  //func to cast an obj of this type to
  //ith baseclass of it or to itself
  void* (*cast)(int,void*);
  
  void add_subtype(const ulTypeInfo* t)
  {
    const ulTypeInfo** ptr = new const ulTypeInfo*[ns+1];		//list is realloc'd with one extra entry.
    int i; for(i=0;i<ns;i++) ptr[i] = subtypes[i];				
    ptr[i] = t;						
    ns++;
    delete[] subtypes;   
    subtypes = ptr;
  }

  void del_subtype(const ulTypeInfo* t)
  {
    int i;
    for(i=0;i<ns && subtypes[i]!=t;i++);
    if (i<ns)
      for(;i<ns-1;i++) subtypes[i] = subtypes[i+1]; 	
  }

public:
  ulTypeInfo(const char* name, const ulTypeInfo* bb[], void* (*f)(int,void*))
  {
    n = new char[strlen(name) + 1];
    strcpy(n, name);

    b = bb; //ns = 0; subtypes = 0;		//Create default ulTypeInfo
    cast    = f;								//Attach casting func
    
    for(int i=0;b[i];i++)							//Add this as subtype to all its basetypes
      ((ulTypeInfo**)b)[i]->add_subtype(this);				//REMARK: Harmless const castaway
  }

  ~ulTypeInfo()
  {
    delete[] n ;
    n = 0 ;

    for(int i=0;b[i];i++)							//Del this subtype from all its basetypes
      ((ulTypeInfo**)b)[i]->del_subtype(this);				//REMARK: Harmless const castaway
  }

  int operator==(const ulTypeInfo& i) const
  { return this == &i; /* just compare physical address */ }

  int operator!=(const ulTypeInfo& i) const
  { return this != &i; /* just compare physical address */ }

  const char* name() const
  {
    return n;
  }

  int can_cast(const ulTypeInfo* p) const
  {  
    return this==p || p->has_base(this);
  }

  int has_base(const ulTypeInfo* p) const
  {  
    for(int i=0;b[i];i++)							//for all bases of this...
      if (p == b[i] || b[i]->has_base(p)) return 1;				//match found, return 1 or no match, search deeper
      return 0;									//no match at all, return 0
  }

  const ulTypeInfo* subclass(int i=0) const
  {
    return (i>=0 && i<ns)? subtypes[i]: 0;
  } 

  int num_subclasses() const
  {
    return ns;
  } 
};      	

/////////////////////////////////////////////////////////////////////////////////////

class ulTypeId	
{
protected:
  const ulTypeInfo* id;

public:
  static ulTypeId null_type() // the ulTypeId for NULL ptrs
  { return &(ulTypeInfo::null_type); }

  ulTypeId(const ulTypeInfo* p): id(p) {}
  ulTypeId():id(null_type().id) {}

  int operator==(ulTypeId i) const
  { return id == i.id; /* just compare physical address */ }

  int operator!=(ulTypeId i) const
  { return id != i.id; /* just compare physical address */ }

  const ulTypeInfo* get_info() const
  { return id; }

  const char* name() const
  { return id->name(); }

  int can_cast(ulTypeId i) const
  { return id->can_cast(i.id); }

  int num_subclasses() const
  { return id->num_subclasses(); }

  ulTypeId subclass(int i) const
  { return id->subclass(i); }

  int	num_baseclasses() const
  {
    int i;
    for(i=0;id->b[i];i++);
    return i;
  }

  ulTypeId baseclass(int i) const
  { return id->b[i]; }
};

/////////////////////////////////////////////////////////////////////////////////////
		
#define UL_STATIC_TYPEID(T)   T::RTTI_sinfo()
#define UL_TYPEID(p)          ((p)? (p)->RTTI_vinfo() : ulTypeId::null_type() )
#define UL_CAST(T,p) 	        ((p)? (T*)((p)->RTTI_cast(UL_STATIC_TYPEID(T))) : 0)

// Definition of TYPE_DATA for a RTTI-class: introduces one static ulTypeInfo data-member
// and a couple of virtuals.

#define UL_TYPE_DATA			 		          \
	protected:					          \
	   static  const  ulTypeInfo RTTI_obj; 		  \
	   static  void*  RTTI_scast(int,void*);	          \
	public:						          \
	   virtual ulTypeId RTTI_vinfo() const { return &RTTI_obj; }\
	   static  ulTypeId RTTI_sinfo()	 { return &RTTI_obj; }\
	   virtual void*  RTTI_cast(ulTypeId);		       
	
//////////////////////////////////////////////////////////////////
//
//	Top-level macros:
//

#define UL_TYPE_DEF_BASE(cls,name)					\
	static const ulTypeInfo* RTTI_base_ ## cls [] = { 0 };\
	void* cls::RTTI_cast(ulTypeId t)			\
	{							\
	   if (t == &RTTI_obj) return this;			\
	   return 0;						\
	}							\
	void* cls::RTTI_scast(int i,void* p)			\
	{  cls* ptr = (cls*)p; return ptr; }			\
  const ulTypeInfo cls::RTTI_obj(name,RTTI_base_ ## cls,cls::RTTI_scast);
	

#define UL_TYPE_DEF1_BASE(cls,name,b1)				\
        static const ulTypeInfo* RTTI_base_ ## cls [] = 	\
	       { UL_STATIC_TYPEID(b1).get_info(),0 };		\
  	void* cls::RTTI_cast(ulTypeId t)			\
	{							\
	   if (t == &RTTI_obj) return this;			\
	   void* ptr;						\
	   if ((ptr=b1::RTTI_cast(t))) return ptr;		\
	   return 0;						\
	}							\
	void* cls::RTTI_scast(int i,void* p)			\
	{  cls* ptr = (cls*)p;					\
	   switch(i)						\
	   {  case  0: return (b1*)ptr;	 }			\
	   return ptr;						\
	}							\
  const ulTypeInfo cls::RTTI_obj(name,RTTI_base_ ## cls,cls::RTTI_scast);
									

#define UL_TYPE_DEF2_BASE(cls,name,b1,b2)				\
        static const ulTypeInfo* RTTI_base_ ## cls [] = 	\
	       { UL_STATIC_TYPEID(b1).get_info(),		\
		 UL_STATIC_TYPEID(b2).get_info(),0 };		\
  	void* cls::RTTI_cast(ulTypeId t)			\
	{							\
	   if (t == &RTTI_obj) return this;			\
	   void* ptr;						\
	   if ((ptr=b1::RTTI_cast(t))) return ptr;		\
	   if ((ptr=b2::RTTI_cast(t))) return ptr;		\
	   return 0;						\
	}							\
	void* cls::RTTI_scast(int i,void* p)			\
	{  cls* ptr = (cls*)p;					\
	   switch(i)						\
	   {  case  0: return (b1*)ptr;				\
	      case  1: return (b2*)ptr;				\
	   }							\
	   return ptr;						\
	}							\
  const ulTypeInfo cls::RTTI_obj(name,RTTI_base_ ## cls,cls::RTTI_scast);
	
#define UL_TYPE_DEF3_BASE(cls,name,b1,b2,b3)			\
        static const ulTypeInfo* RTTI_base_ ## cls [] = 	\
	       { UL_STATIC_TYPEID(b1).get_info(),		\
		 UL_STATIC_TYPEID(b2).get_info(),		\
		 UL_STATIC_TYPEID(b3).get_info(), 0 };		\
  	void* cls::RTTI_cast(ulTypeId t)			\
	{							\
	   if (t == &RTTI_obj) return this;			\
	   void* ptr;						\
	   if ((ptr=b1::RTTI_cast(t))) return ptr;		\
	   if ((ptr=b2::RTTI_cast(t))) return ptr;		\
	   if ((ptr=b3::RTTI_cast(t))) return ptr;		\
	   return 0;						\
	}							\
	void* cls::RTTI_scast(int i,void* p)			\
	{  cls* ptr = (cls*)p;					\
	   switch(i)						\
	   {  case  0: return (b1*)ptr;				\
	      case  1: return (b2*)ptr;				\
	      case  2: return (b3*)ptr;				\
	   }							\
	   return ptr;						\
	}							\
  const ulTypeInfo cls::RTTI_obj(name,RTTI_base_ ## cls,cls::RTTI_scast);
	

#define UL_TYPE_DEF(cls,name)					\
	UL_TYPE_DEF_BASE(cls,name)

#define UL_TYPE_DEF1(cls,name,b1)					\
	UL_TYPE_DEF1_BASE(cls,name,b1)
	
#define UL_TYPE_DEF2(cls,name,b1,b2)				\
	UL_TYPE_DEF2_BASE(cls,name,b1,b2)

#define UL_TYPE_DEF3(cls,name,b1,b2,b3)				\
	UL_TYPE_DEF3_BASE(cls,name,b1,b2,b3)

#else

/*
Yea! the compiler supports RTTI and everything is simple.
*/

#define UL_STATIC_TYPEID(type)  (&typeid(type))
#define UL_TYPEID(ptr)          (&typeid(*ptr))
#define UL_CAST(type,ptr)       ((dynamic_cast<type*>(ptr)))

#define UL_TYPE_DATA
#define UL_TYPE_DEF(cls,name)
#define UL_TYPE_DEF1(cls,name,b1)
#define UL_TYPE_DEF2(cls,name,b1,b2)
#define UL_TYPE_DEF3(cls,name,b1,b2,b3)

#endif

// some useful macros
#define UL_ISA(type,ptr)        (UL_STATIC_TYPEID(type) == UL_TYPEID(ptr))
#define UL_ISKINDOF(type,ptr)   (UL_CAST(type,ptr) != NULL)

#endif