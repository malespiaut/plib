
ssgBranch *extractBones     ( ssgBranch *root ) ;
ssgEntity *makeTweenCopy    ( ssgEntity *root ) ;
void       extractVertices  ( ssgBranch *root ) ;
void       transformModel   ( ssgBranch *boneRoot, float tim ) ;
void       makeTweenCopy    ( ssgBranch *dest, ssgBranch *src ) ;
void       addTweenBank     ( ssgBranch *root ) ;
void       offsetChildBones ( int root, sgVec3 v ) ;

float getLowestVertexZ () ;

int getNumBones () ;                                                           

