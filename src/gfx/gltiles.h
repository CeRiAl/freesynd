#ifndef __gltiles_h__
#define __gltiles_h__

#include <GL/gl.h>

extern const GLfloat tiles_vertices[];

struct GLTile {
  unsigned  nb_vertices_;
  const GLubyte *faces_;
};

extern const GLTile gl_tiles[];

#endif // __gltiles_h__
