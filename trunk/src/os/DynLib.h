//DynLib.h
//From Util.lib
//part of OSCAR v0.7
//Copyright 1996-1999 Sean L. Palmer 
//contact: spalmer@pobox.com

//crossplatform support for dynamic-load code libraries
#ifndef __DynLib_h
#define __DynLib_h

#include "Util.h"

#ifdef ONCE_SUPPORTED
  #pragma once
#endif

typedef void* Module;

Module ModuleOpen(const char* modulename); //without path or extension
void* ModuleFunc(Module,const char* funcname); //undecorated
void ModuleClose(Module);


#endif // __DynLib_h