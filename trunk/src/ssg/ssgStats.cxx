#include "ssgLocal.h"

int stats_num_vertices    = 0 ;
int stats_isect_triangles = 0 ;
int stats_isect_test      = 0 ;
int stats_cull_test       = 0 ;
int stats_bind_textures   = 0 ;
int stats_num_leaves      = 0 ;

int stats_hot_triangles   = 0 ;
int stats_hot_test        = 0 ;
int stats_hot_no_trav     = 0 ;
int stats_hot_radius_reject=0 ;
int stats_hot_triv_accept = 0 ;
int stats_hot_straddle    = 0 ;

static char stats_string [ 1024 ] ;

char *ssgShowStats ()
{
  sprintf ( stats_string, "V=%4d, L=%3d H=%3d IS=%3d IT=%3d HT=%3d CT=%3d BT=%3d\n",
            stats_num_vertices   ,
            stats_num_leaves     ,
            stats_hot_triangles  ,
            stats_isect_triangles,
	    stats_isect_test     ,
	    stats_hot_test       ,
	    stats_cull_test      ,
	    stats_bind_textures  ) ;
/*
  sprintf ( stats_string, "Tri=%d, Tst=%d NoTr=%d Rej=%d Acp=%d Str=%d\n",
	    stats_hot_triangles   ,
	    stats_hot_test        ,
	    stats_hot_no_trav     ,
	    stats_hot_radius_reject,
	    stats_hot_triv_accept ,
	    stats_hot_straddle    ) ;
*/

  stats_num_vertices    = 0 ;
  stats_num_leaves      = 0 ;
  stats_isect_triangles = 0 ;
  stats_isect_test      = 0 ;
  stats_cull_test       = 0 ;
  stats_bind_textures   = 0 ;

  stats_hot_triangles   = 0 ;
  stats_hot_test        = 0 ;
  stats_hot_no_trav     = 0 ;
  stats_hot_radius_reject=0 ;
  stats_hot_triv_accept = 0 ;
  stats_hot_straddle    = 0 ;

  return stats_string ;
}



