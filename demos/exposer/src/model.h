
ssgBranch *extractBones     ( ssgBranch *root ) ;
void       extractVertices  ( ssgBranch *root ) ;
void       transformModel   ( ssgBranch *boneRoot, float tim ) ;
void       addTweenBank     ( ssgBranch *root ) ;
void       offsetChildBones ( int root, sgVec3 v ) ;

float getLowestVertexZ () ;

int getNumBones () ;                                                           

