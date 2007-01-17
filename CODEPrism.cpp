
#include "CODEPrism.h"

cODEPrism::cODEPrism(cWorld* a_parent, dWorldID a_odeWorld, dSpaceID a_odeSpace, const cVector3d& a_size)
	: cODEMesh(a_parent, a_odeWorld, a_odeSpace)
{
	m_size = a_size;
    m_objClass = CLASS_PRISM;
	create(false);
	initDynamic(BOX);
}

cODEPrism::~cODEPrism()
{
}

// This function stolen from dynamic_ode...
void cODEPrism::create(bool openbox)
{
  int n;
  int cur_index = 0;
  int start_index = 0;

  // +x face
  newVertex( m_size.x/2,  m_size.y/2, -m_size.z/2);
  newVertex( m_size.x/2,  m_size.y/2,  m_size.z/2);
  newVertex( m_size.x/2, -m_size.y/2, -m_size.z/2);
  newTriangle(cur_index,cur_index+1,cur_index+2);
  cur_index+=3;

  newVertex( m_size.x/2, -m_size.y/2, -m_size.z/2);
  newVertex( m_size.x/2,  m_size.y/2,  m_size.z/2);
  newVertex( m_size.x/2, -m_size.y/2,  m_size.z/2);
  newTriangle(cur_index,cur_index+1,cur_index+2);
  cur_index+=3;

  for(n=start_index; n<cur_index; n++) {
    cVertex* curVertex = getVertex(n);
    curVertex->setTexCoord(
      (curVertex->getPos().y + m_size.x/2) / (2.0 * m_size.z/2),
      (curVertex->getPos().z + m_size.x/2) / (2.0 * m_size.y/2)
      );
    curVertex->setNormal(1,0,0);
  }
  
  start_index += 6;

  // -x face
  newVertex(-m_size.x/2,  m_size.y/2,  m_size.z/2);
  newVertex(-m_size.x/2,  m_size.y/2, -m_size.z/2);
  newVertex(-m_size.x/2, -m_size.y/2, -m_size.z/2);
  newTriangle(cur_index,cur_index+1,cur_index+2);
  cur_index+=3;

  newVertex(-m_size.x/2,  m_size.y/2,  m_size.z/2);
  newVertex(-m_size.x/2, -m_size.y/2, -m_size.z/2);
  newVertex(-m_size.x/2, -m_size.y/2,  m_size.z/2);
  newTriangle(cur_index,cur_index+1,cur_index+2);
  cur_index+=3;

  for(n=start_index; n<cur_index; n++) {
    cVertex* curVertex = getVertex(n);
    curVertex->setTexCoord(
      (curVertex->getPos().y + m_size.x/2) / (2.0 * m_size.z/2),
      (curVertex->getPos().z + m_size.x/2) / (2.0 * m_size.y/2)
      );
    curVertex->setNormal(-1,0,0);
  }
  
  start_index += 6;

  // +y face
  newVertex(m_size.x/2,  m_size.y/2,  m_size.z/2);
  newVertex(m_size.x/2,  m_size.y/2, -m_size.z/2);
  newVertex(-m_size.x/2, m_size.y/2, -m_size.z/2);
  newTriangle(cur_index,cur_index+1,cur_index+2);
  cur_index+=3;

  newVertex(m_size.x/2,  m_size.y/2,  m_size.z/2);
  newVertex(-m_size.x/2, m_size.y/2, -m_size.z/2);
  newVertex(-m_size.x/2, m_size.y/2,  m_size.z/2);
  newTriangle(cur_index,cur_index+1,cur_index+2);
  cur_index+=3;

  for(n=start_index; n<cur_index; n++) {
    cVertex* curVertex = getVertex(n);
    curVertex->setTexCoord(
      (curVertex->getPos().x + m_size.y/2) / (2.0 * m_size.z/2),
      (curVertex->getPos().z + m_size.x/2) / (2.0 * m_size.y/2)
      );
    curVertex->setNormal(0,1,0);
  }
  
  start_index += 6;

  // -y face
  newVertex(m_size.x/2,  -m_size.y/2,  m_size.z/2);
  newVertex(-m_size.x/2, -m_size.y/2, -m_size.z/2);
  newVertex(m_size.x/2,  -m_size.y/2, -m_size.z/2);
  newTriangle(cur_index,cur_index+1,cur_index+2);
  cur_index+=3;

  newVertex(-m_size.x/2, -m_size.y/2, -m_size.z/2);
  newVertex(m_size.x/2,  -m_size.y/2,  m_size.z/2);
  newVertex(-m_size.x/2, -m_size.y/2,  m_size.z/2);
  newTriangle(cur_index,cur_index+1,cur_index+2);
  cur_index+=3;

  for(n=start_index; n<cur_index; n++) {
    cVertex* curVertex = getVertex(n);
    curVertex->setTexCoord(
      (curVertex->getPos().x + m_size.y/2) / (2.0 * m_size.z/2),
      (curVertex->getPos().z + m_size.x/2) / (2.0 * m_size.y/2)
      );
    curVertex->setNormal(0,-1,0);
  }
  
  start_index += 6;
  
  // -z face
  newVertex(-m_size.x/2, -m_size.y/2, -m_size.z/2);
  newVertex(m_size.x/2,   m_size.y/2, -m_size.z/2);
  newVertex(m_size.x/2,  -m_size.y/2, -m_size.z/2);
  newTriangle(cur_index,cur_index+1,cur_index+2);
  cur_index+=3;

  newVertex( m_size.x/2,  m_size.y/2, -m_size.z/2);
  newVertex(-m_size.x/2, -m_size.y/2, -m_size.z/2);
  newVertex(-m_size.x/2,  m_size.y/2, -m_size.z/2);
  newTriangle(cur_index,cur_index+1,cur_index+2);
  cur_index+=3;

  for(n=start_index; n<cur_index; n++) {
    cVertex* curVertex = getVertex(n);
    curVertex->setTexCoord(
      (curVertex->getPos().x + m_size.y/2) / (2.0 * m_size.z/2),
      (curVertex->getPos().y + m_size.x/2) / (2.0 * m_size.y/2)
      );
    curVertex->setNormal(0,0,-1);
  }
  
  start_index += 6;

  if (!openbox) {
  
    // +z face
    newVertex(-m_size.x/2, -m_size.y/2, m_size.z/2);
    newVertex(m_size.x/2,  -m_size.y/2, m_size.z/2);
    newVertex(m_size.x/2,  m_size.y/2,  m_size.z/2);
    newTriangle(cur_index,cur_index+1,cur_index+2);
    cur_index+=3;

    newVertex(-m_size.x/2, -m_size.y/2, m_size.z/2);
    newVertex( m_size.x/2,  m_size.y/2, m_size.z/2);
    newVertex(-m_size.x/2,  m_size.y/2, m_size.z/2);
    newTriangle(cur_index,cur_index+1,cur_index+2);
    cur_index+=3;

    for(n=start_index; n<cur_index; n++) {
      cVertex* curVertex = getVertex(n);
      curVertex->setTexCoord(
        (curVertex->getPos().x + m_size.y/2) / (2.0 * m_size.z/2),
        (curVertex->getPos().y + m_size.x/2) / (2.0 * m_size.y/2)
        );
      curVertex->setNormal(0,0,1);
	}
  
    start_index += 6;
  }

  // Give a color to each vertex
  for (unsigned int i=0; i<getNumVertices(); i++) {

    cVertex* nextVertex = getVertex(i);

    cColorb color;
    color.set(
      GLuint(0xff*(m_size.x + nextVertex->getPos().x ) / (2.0 * m_size.x)),
      GLuint(0xff*(m_size.x + nextVertex->getPos().y ) / (2.0 * m_size.y)),
      GLuint(0xff* nextVertex->getPos().z / 2*m_size.z)
      );

    nextVertex->setColor(color);
  }

  // Set object settings.  The parameters tell the object
  // to apply this alpha level to his textures and to his
  // children (of course he has neither right now).
  // object->setTransparencyLevel(0.5, true, true);

  // Give him some material properties...
  cMaterial material;

  material.m_ambient.set( 0.6, 0.3, 0.3, 1.0 );
  material.m_diffuse.set( 0.8, 0.6, 0.6, 1.0 );
  material.m_specular.set( 0.9, 0.9, 0.9, 1.0 );
  material.setShininess(100);
  m_material = material;
  
}

void cODEPrism::setSize(cVector3d& a_size)
{
    // For now, just re-create it at the given size
    m_size = a_size;
	create(false);
	initDynamic(BOX);
}
