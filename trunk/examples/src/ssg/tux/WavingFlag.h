#ifndef _WAVINGFLAG_
#define _WAVINGFLAG_

class WavingFlag
{
public:
	WavingFlag( sgVec4 color, char *texture1=NULL, char *texture2=NULL, int s=5 );
	~WavingFlag();
	void animate( float time, float windVelocity );
	ssgEntity *getObject();
private:
	ssgBranch *branch;
	ssgColourArray *colors;
	ssgNormalArray *normals;
	ssgTexCoordArray *texCoords;
	ssgVertexArray *vertices;
	int size;
};

#endif