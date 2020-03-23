/******************************************************************************

 @File Name : {PROJECT_DIR}/tag.cc

 @Creation Date : 31-01-2012

 @Last Modified : Wed 01 Feb 2012 12:05:20 AM CST

 @Created By: Wenxuan Zhou

 @Purpose :

*******************************************************************************/


#include <stdint.h>

static int64_t system_tag = 0;

int64_t allocate_tag()
{
  return system_tag++;
}
