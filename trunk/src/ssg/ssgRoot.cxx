
#include "ssgLocal.h"

void ssgRoot::copy_from ( ssgRoot *src, int clone_flags )
{
  ssgBranch::copy_from ( src, clone_flags ) ;
}

ssgBase *ssgRoot::clone ( int clone_flags )
{
  ssgRoot *b = new ssgRoot ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}


ssgRoot::ssgRoot (void)
{
  type |= SSG_TYPE_ROOT ;
}

ssgRoot::~ssgRoot (void)
{
}


int ssgRoot::load ( FILE *fd )
{
  return ssgBranch::load(fd) ;
}

int ssgRoot::save ( FILE *fd )
{
  return ssgBranch::save(fd) ;
}



