//DynLib.cpp
//From Util.lib
//part of OSCAR v0.7
//Copyright 1996-1999 Sean L. Palmer 
//contact: spalmer@pobox.com

//crossplatform support for dynamic-load code libraries
#include "Pch.h"

#include "DynLib.h"

#include "Os.h"

#include <string.h>

#if defined(__WIN32__)

#include <windows.h>

Module ModuleOpen(const char* modulename) {
  char dllname[MAX_PATH];
  strcpy(dllname,modulename);
  strcat(dllname,".dll");
  HMODULE hmod=(HMODULE)LoadLibrary(dllname);
  //generates standard call to DllMain
  return hmod;
}

void* ModuleFunc(Module hmod,const char* funcname) {
  return GetProcAddress((HMODULE)hmod, funcname);
}

void ModuleClose(Module hmod) {
  //generates standard call to DllMain
  FreeLibrary((HMODULE)hmod);
}

#elif defined(__UNIX__)

#include <dlfcn.h>

Module ModuleOpen(const char* modulename) {
  char dlname[MAX_PATH];
  strcpy(dlname,modulename);
  strcat(dlname,".dl");
  void* libhandle=dlopen(dlname,RTLD_NOW);
  if (libhandle) {
    //notify dl it has been loaded
    void (*initdl)() = dlsym(libhandle, "initdl");
    if (initdl) 
      initdl();
  }
  return libhandle;
}

void* ModuleFunc(Module libhandle,const char* funcname) {
  return dlsym(libhandle, funcname);
}

void ModuleClose(Module libhandle) {
  if (libhandle) {
    //notify dl it is about to be unloaded
    void (*donedl)() = dlsym(libhandle, "donedl");
    if (donedl) 
      donedl();
    dlclose(libhandle);
  }
}

#elif defined(__BEOS__) 

#include <be/kernel/image.h>

Module ModuleOpen(const char* modulename) {
  char addonname[MAX_PATH];
  strcpy(addonname,modulename);
  strcat(addonname,".so");
  image_id* image=new image_id;
  *image=load_add_on(addonname);
	if (*image==B_ERROR) {
    delete image;
    image=0;
  } else {
    //notify addon it has been loaded
    void (*initso)()=ModuleFunc(image,"initso");
    if (initso) 
      initso();
  }
  return image;
}

void* ModuleFunc(Module hmod,const char* funcname) {
  image_id* image=(image_id*)hmod;
  void* sym=0;
  if (image)
    if (get_image_symbol(image, "funcname", B_SYMBOL_TYPE_TEXT, &sym)!=B_NO_ERROR)
      return 0;
  return sym;
}

void ModuleClose(Module hmod) {
  image_id* image=(image_id*)hmod;
  if (image) {
    //notify addon it is about to be unloaded
    void (*doneso)()=ModuleFunc(hmod, "doneso");
    if (doneso) 
      doneso();
    unload_add_on(image);
    delete (image_id*)image;
  }
}

#else

#error "Don't know how to load dynamic link libraries on this platform"

#endif 
