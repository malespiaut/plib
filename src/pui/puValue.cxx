
#include "puLocal.h"

void puValue::re_eval ()
{
  if ( res_floater ) setValue ( *res_floater ) ; else
  if ( res_integer ) setValue ( *res_integer ) ; else
  if ( res_string  ) setValue ( res_string ) ;
}

void puValue::update_res ()
{
  if ( res_integer ) *res_integer = integer ;
  if ( res_floater ) *res_floater = floater ;
  if ( res_string  ) strcpy ( res_string, string ) ;
}


