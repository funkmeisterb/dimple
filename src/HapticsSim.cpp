// -*- mode:c++; indent-tabs-mode:nil; c-basic-offset:4; compile-command:"scons debug=1" -*-

#include "dimple.h"
#include "HapticsSim.h"
#include "CODEPotentialProxy.h"

bool HapticsPrismFactory::create(const char *name, float x, float y, float z)
{
    printf("HapticsPrismFactory (%s) is creating a prism object called '%s'\n",
           m_parent->c_name(), name);

    OscPrismCHAI *obj = new OscPrismCHAI(simulation()->world(),
                                         name, m_parent);

    if (!(obj && simulation()->add_object(*obj)))
            return false;

    obj->m_position.set(x, y, z);

    return true;
}

bool HapticsSphereFactory::create(const char *name, float x, float y, float z)
{
    OscSphereCHAI *obj = new OscSphereCHAI(simulation()->world(),
                                           name, m_parent);

    if (!(obj && simulation()->add_object(*obj)))
            return false;

    obj->m_position.set(x, y, z);

    return true;
}

bool HapticsMeshFactory::create(const char *name, const char *filename,
                                float x, float y, float z)
{
    printf("HapticsMeshFactory (%s) is creating a mesh "
           "object called '%s' (%s)\n", m_parent->c_name(), name, filename);

    OscMeshCHAI *obj = new OscMeshCHAI(simulation()->world(),
                                       name, filename, m_parent);

    if (!obj->object()) {
        delete obj;
        obj = NULL;
    }

    if (!(obj && simulation()->add_object(*obj)))
            return false;

    obj->m_position.set(x, y, z);

    return true;
}


/****** HapticsSim ******/

HapticsSim::HapticsSim(const char *port)
    : Simulation(port, ST_HAPTICS)
{
    m_pPrismFactory = new HapticsPrismFactory(this);
    m_pSphereFactory = new HapticsSphereFactory(this);
    m_pMeshFactory = new HapticsMeshFactory(this);

    m_fTimestep = HAPTICS_TIMESTEP_MS/1000.0;
    printf("CHAI timestep: %f\n", m_fTimestep);
}

HapticsSim::~HapticsSim()
{
}

void HapticsSim::initialize()
{
    // create the world object
    m_chaiWorld = new cWorld();
    m_chaiWorld->setBackgroundColor(0.0f,0.0f,0.0f);

    // create an OscObject to point to the cursor
    m_cursor = new OscCursorCHAI(m_chaiWorld, "cursor", this);
    m_chaiWorld->addChild(m_cursor->object());

    // quit the haptics simulation if the cursor couldn't be initialized.
    if (!m_cursor->is_initialized())
        m_bDone = true;

    // initialize visual step count
    m_nVisualStepCount = 0;

    // initialize step count
    m_counter = 0;

    Simulation::initialize();
}

void HapticsSim::step()
{
    cMeta3dofPointer *cursor = m_cursor->object();
    cursor->updatePose();
    cursor->computeForces();

    m_cursor->m_position.set(cursor->m_deviceGlobalPos);
    m_cursor->m_velocity.set(cursor->m_deviceGlobalVel);

    cursor->applyForces();

    m_counter++;

    if (1 || ++m_nVisualStepCount >= (VISUAL_TIMESTEP_MS/HAPTICS_TIMESTEP_MS))
    {
        /* If in contact with an object, display the cursor at the
         * proxy location instead of the device location, so that it
         * does not show it penetrating the object. */
        cVector3d pos(cursor->m_deviceGlobalPos);
        cProxyPointForceAlgo *algo =
            (cProxyPointForceAlgo*) cursor->m_pointForceAlgos[0];
        if (algo->getContactObject())
            pos = algo->getProxyGlobalPosition();

        sendtotype(Simulation::ST_VISUAL, true,
                   "/world/cursor/position","fff",
                   pos.x, pos.y, pos.z);

        m_nVisualStepCount = 0;
    }

    findContactObject();

    if (m_pContactObject) {
        sendtotype(Simulation::ST_PHYSICS, true,
                   (m_pContactObject->path()+"/push").c_str(), "ffffff",
                   -m_lastForce.x * 0.00001,
                   -m_lastForce.y * 0.00001,
                   -m_lastForce.z * 0.00001,
                   m_lastContactPoint.x,
                   m_lastContactPoint.y,
                   m_lastContactPoint.z);

        bool co1 = m_pContactObject->collidedWith(m_cursor, m_counter);
        bool co2 = m_cursor->collidedWith(m_pContactObject, m_counter);
        if ( (co1 || co2) && m_collide.m_value ) {
            lo_send(address_send, "/world/collide", "ssf",
                    m_pContactObject->c_name(), m_cursor->c_name(),
                    (double)(m_pContactObject->m_velocity
                             - m_cursor->m_velocity).length());

            /*
             * TODO: The above should perhaps be derived from force
             * instead of velocity:
             *
             * lo_send(address_send, "/world/collide", "ssf",
             *      m_pContactObject->c_name(), "cursor",
             *      (double)(m_lastForce.length() * 0.00001));
            */
        }

        /* TODO: the above 0.00001 scaling are just temporary
           coefficients observed to work (somewhat) with the example
           patch.  They are to be removed when mass scaling is
           properly implemented. */
    }
}

void HapticsSim::findContactObject()
{
    m_pContactObject = NULL;
    cGenericObject *obj = NULL;

    cMeta3dofPointer *cursor = m_cursor->object();
    for (unsigned int i=0; i<cursor->m_pointForceAlgos.size(); i++)
    {
        cProxyPointForceAlgo* pointforce_proxy =
            dynamic_cast<cProxyPointForceAlgo*>(cursor->m_pointForceAlgos[i]);
        if ((pointforce_proxy != NULL)
            && (pointforce_proxy->getContactObject() != NULL))
        {
            m_lastContactPoint = pointforce_proxy->getContactPoint();
            m_lastForce = cursor->m_lastComputedGlobalForce;
            obj = pointforce_proxy->getContactObject();
            break;
        }

        cODEPotentialProxy* potential_proxy =
            dynamic_cast<cODEPotentialProxy*>(cursor->m_pointForceAlgos[i]);
        if ((potential_proxy != NULL)
            && (potential_proxy->getContactObject() != NULL))
        {
            m_lastContactPoint = potential_proxy->getContactPoint();
            m_lastForce = cursor->m_lastComputedGlobalForce;
            obj = potential_proxy->getContactObject();
            break;
        }
    }

    // User data is set in the Osc*CHAI constructors
    if (obj)
        m_pContactObject = (OscObject*)obj->m_userData;
}

/****** CHAIObject ******/

CHAIObject::CHAIObject(cWorld *world)
{
}

CHAIObject::~CHAIObject()
{
}

/****** OscSphereCHAI ******/

OscSphereCHAI::OscSphereCHAI(cWorld *world, const char *name, OscBase *parent)
    : OscSphere(NULL, name, parent), CHAIObject(world)
{
    m_pSphere = new cShapeSphere(0.01);
    world->addChild(m_pSphere);
    m_pSphere->computeGlobalPositions();

    // User data points to the OscObject, used for identification
    // during object contact.
    m_pSphere->setUserData(this, 1);
}

OscSphereCHAI::~OscSphereCHAI()
{
    if (m_pSphere)
        m_pSphere->getParent()->deleteChild(m_pSphere);
}

void OscSphereCHAI::on_radius()
{
    printf("OscSphereCHAI::on_radius(). radius = %f\n", m_radius.m_value);

    if (!m_pSphere)
        return;

    m_pSphere->setRadius(m_radius.m_value);
}

void OscSphereCHAI::on_grab()
{
    // remove object from haptic contact
    object()->setHapticEnabled(false, true);

    // make cursor invisible
    simulation()->sendtotype(Simulation::ST_VISUAL, 0,
                             "/world/cursor/visible", "i", 0);
}

/****** OscPrismCHAI ******/

OscPrismCHAI::OscPrismCHAI(cWorld *world, const char *name, OscBase *parent)
    : OscPrism(NULL, name, parent), CHAIObject(world)
{
    m_pPrism = new cMesh(world);
    m_size.set(0.01, 0.01, 0.01);
    createPrism();
    world->addChild(m_pPrism);
    m_pPrism->computeGlobalPositions();

    // User data points to the OscObject, used for identification
    // during object contact.
    m_pPrism->setUserData(this, 1);
}

OscPrismCHAI::~OscPrismCHAI()
{
    if (m_pPrism)
        m_pPrism->getParent()->deleteChild(m_pPrism);
}

// This function borrowed from dynamic_ode example in CHAI.
void OscPrismCHAI::createPrism(bool openbox)
{
    int n;
    int cur_index = 0;
    int start_index = 0;

    // +x face
    m_pPrism->newVertex( m_size.x/2,  m_size.y/2, -m_size.z/2);
    m_pPrism->newVertex( m_size.x/2,  m_size.y/2,  m_size.z/2);
    m_pPrism->newVertex( m_size.x/2, -m_size.y/2, -m_size.z/2);
    m_pPrism->newTriangle(cur_index,cur_index+1,cur_index+2);
    cur_index+=3;

    m_pPrism->newVertex( m_size.x/2, -m_size.y/2, -m_size.z/2);
    m_pPrism->newVertex( m_size.x/2,  m_size.y/2,  m_size.z/2);
    m_pPrism->newVertex( m_size.x/2, -m_size.y/2,  m_size.z/2);
    m_pPrism->newTriangle(cur_index,cur_index+1,cur_index+2);
    cur_index+=3;

    for(n=start_index; n<cur_index; n++) {
        cVertex* curVertex = m_pPrism->getVertex(n);
        curVertex->setTexCoord(
            (curVertex->getPos().y + m_size.x/2) / (2.0 * m_size.z/2),
            (curVertex->getPos().z + m_size.x/2) / (2.0 * m_size.y/2)
            );
        curVertex->setNormal(1,0,0);
    }

    start_index += 6;

    // -x face
    m_pPrism->newVertex(-m_size.x/2,  m_size.y/2,  m_size.z/2);
    m_pPrism->newVertex(-m_size.x/2,  m_size.y/2, -m_size.z/2);
    m_pPrism->newVertex(-m_size.x/2, -m_size.y/2, -m_size.z/2);
    m_pPrism->newTriangle(cur_index,cur_index+1,cur_index+2);
    cur_index+=3;

    m_pPrism->newVertex(-m_size.x/2,  m_size.y/2,  m_size.z/2);
    m_pPrism->newVertex(-m_size.x/2, -m_size.y/2, -m_size.z/2);
    m_pPrism->newVertex(-m_size.x/2, -m_size.y/2,  m_size.z/2);
    m_pPrism->newTriangle(cur_index,cur_index+1,cur_index+2);
    cur_index+=3;

    for(n=start_index; n<cur_index; n++) {
        cVertex* curVertex = m_pPrism->getVertex(n);
        curVertex->setTexCoord(
            (curVertex->getPos().y + m_size.x/2) / (2.0 * m_size.z/2),
            (curVertex->getPos().z + m_size.x/2) / (2.0 * m_size.y/2)
            );
        curVertex->setNormal(-1,0,0);
    }

    start_index += 6;

    // +y face
    m_pPrism->newVertex(m_size.x/2,  m_size.y/2,  m_size.z/2);
    m_pPrism->newVertex(m_size.x/2,  m_size.y/2, -m_size.z/2);
    m_pPrism->newVertex(-m_size.x/2, m_size.y/2, -m_size.z/2);
    m_pPrism->newTriangle(cur_index,cur_index+1,cur_index+2);
    cur_index+=3;

    m_pPrism->newVertex(m_size.x/2,  m_size.y/2,  m_size.z/2);
    m_pPrism->newVertex(-m_size.x/2, m_size.y/2, -m_size.z/2);
    m_pPrism->newVertex(-m_size.x/2, m_size.y/2,  m_size.z/2);
    m_pPrism->newTriangle(cur_index,cur_index+1,cur_index+2);
    cur_index+=3;

    for(n=start_index; n<cur_index; n++) {
        cVertex* curVertex = m_pPrism->getVertex(n);
        curVertex->setTexCoord(
            (curVertex->getPos().x + m_size.y/2) / (2.0 * m_size.z/2),
            (curVertex->getPos().z + m_size.x/2) / (2.0 * m_size.y/2)
            );
        curVertex->setNormal(0,1,0);
    }

    start_index += 6;

    // -y face
    m_pPrism->newVertex(m_size.x/2,  -m_size.y/2,  m_size.z/2);
    m_pPrism->newVertex(-m_size.x/2, -m_size.y/2, -m_size.z/2);
    m_pPrism->newVertex(m_size.x/2,  -m_size.y/2, -m_size.z/2);
    m_pPrism->newTriangle(cur_index,cur_index+1,cur_index+2);
    cur_index+=3;

    m_pPrism->newVertex(-m_size.x/2, -m_size.y/2, -m_size.z/2);
    m_pPrism->newVertex(m_size.x/2,  -m_size.y/2,  m_size.z/2);
    m_pPrism->newVertex(-m_size.x/2, -m_size.y/2,  m_size.z/2);
    m_pPrism->newTriangle(cur_index,cur_index+1,cur_index+2);
    cur_index+=3;

    for(n=start_index; n<cur_index; n++) {
        cVertex* curVertex = m_pPrism->getVertex(n);
        curVertex->setTexCoord(
            (curVertex->getPos().x + m_size.y/2) / (2.0 * m_size.z/2),
            (curVertex->getPos().z + m_size.x/2) / (2.0 * m_size.y/2)
            );
        curVertex->setNormal(0,-1,0);
    }

    start_index += 6;

    // -z face
    m_pPrism->newVertex(-m_size.x/2, -m_size.y/2, -m_size.z/2);
    m_pPrism->newVertex(m_size.x/2,   m_size.y/2, -m_size.z/2);
    m_pPrism->newVertex(m_size.x/2,  -m_size.y/2, -m_size.z/2);
    m_pPrism->newTriangle(cur_index,cur_index+1,cur_index+2);
    cur_index+=3;

    m_pPrism->newVertex( m_size.x/2,  m_size.y/2, -m_size.z/2);
    m_pPrism->newVertex(-m_size.x/2, -m_size.y/2, -m_size.z/2);
    m_pPrism->newVertex(-m_size.x/2,  m_size.y/2, -m_size.z/2);
    m_pPrism->newTriangle(cur_index,cur_index+1,cur_index+2);
    cur_index+=3;

    for(n=start_index; n<cur_index; n++) {
        cVertex* curVertex = m_pPrism->getVertex(n);
        curVertex->setTexCoord(
            (curVertex->getPos().x + m_size.y/2) / (2.0 * m_size.z/2),
            (curVertex->getPos().y + m_size.x/2) / (2.0 * m_size.y/2)
            );
        curVertex->setNormal(0,0,-1);
    }

    start_index += 6;

    if (!openbox) {

        // +z face
        m_pPrism->newVertex(-m_size.x/2, -m_size.y/2, m_size.z/2);
        m_pPrism->newVertex(m_size.x/2,  -m_size.y/2, m_size.z/2);
        m_pPrism->newVertex(m_size.x/2,  m_size.y/2,  m_size.z/2);
        m_pPrism->newTriangle(cur_index,cur_index+1,cur_index+2);
        cur_index+=3;

        m_pPrism->newVertex(-m_size.x/2, -m_size.y/2, m_size.z/2);
        m_pPrism->newVertex( m_size.x/2,  m_size.y/2, m_size.z/2);
        m_pPrism->newVertex(-m_size.x/2,  m_size.y/2, m_size.z/2);
        m_pPrism->newTriangle(cur_index,cur_index+1,cur_index+2);
        cur_index+=3;

        for(n=start_index; n<cur_index; n++) {
            cVertex* curVertex = m_pPrism->getVertex(n);
            curVertex->setTexCoord(
                (curVertex->getPos().x + m_size.y/2) / (2.0 * m_size.z/2),
                (curVertex->getPos().y + m_size.x/2) / (2.0 * m_size.y/2)
                );
            curVertex->setNormal(0,0,1);
        }

        start_index += 6;
    }

    // Give a color to each vertex
    for (unsigned int i=0; i<m_pPrism->getNumVertices(); i++) {

        cVertex* nextVertex = m_pPrism->getVertex(i);

        cColorb color;
        color.set(
            GLuint(0xff*(m_size.x + nextVertex->getPos().x ) / (2.0 * m_size.x)),
            GLuint(0xff*(m_size.x + nextVertex->getPos().y ) / (2.0 * m_size.y)),
            GLuint(0xff* nextVertex->getPos().z / 2*m_size.z)
            );

//        nextVertex->setColor(color);
    }

    // Set object settings.  The parameters tell the object
    // to apply this alpha level to his textures and to his
    // children (of course he has neither right now).
    // object->setTransparencyLevel(0.5, true, true);

    // Give him some material properties...
/*
    m_pPrism->m_material.m_ambient.set( 0.6, 0.3, 0.3, 1.0 );
    m_pPrism->m_material.m_diffuse.set( 0.8, 0.6, 0.6, 1.0 );
    m_pPrism->m_material.m_specular.set( 0.9, 0.9, 0.9, 1.0 );
    m_pPrism->m_material.setShininess(100);
*/
}

void OscPrismCHAI::on_size()
{
    // reposition vertices
    int i,n;
    n = m_pPrism->getNumVertices();
    for (i=0; i<n; i++) {
		 cVector3d pos = m_pPrism->getVertex(i)->getPos();
		 pos.elementMul(cVector3d(1.0/fabs(pos.x), 1.0/fabs(pos.y), 1.0/fabs(pos.z)));
		 pos.elementMul(m_size/2.0);
		 m_pPrism->getVertex(i)->setPos(pos);
    }
}

/****** OscMeshCHAI ******/

OscMeshCHAI::OscMeshCHAI(cWorld *world, const char *name, const char *filename,
                         OscBase *parent)
    : OscMesh(NULL, name, filename, parent), CHAIObject(world)
{
    m_pMesh = new cMesh(world);

    if (!m_pMesh->loadFromFile(filename)) {
        printf("[%s] Unable to load %s for object %s.\n",
               simulation()->type_str(), filename, name);
        delete m_pMesh;
        m_pMesh = NULL;
        return;
    }

    printf("[%s] Loaded %s for object %s.\n",
           simulation()->type_str(), filename, name);

    // center the mesh
    m_pMesh->computeBoundaryBox();
    cVector3d vmin = m_pMesh->getBoundaryMin();
    cVector3d vmax = m_pMesh->getBoundaryMax();
    m_pMesh->translate((vmax-vmin*3)/2);

    // size it to 0.1 without changing proportions
    float size = (vmax-vmin).length();
    m_size.set(0.1/size, 0.1/size, 0.1/size);
    on_size();

    /* setup collision detector */
    m_pMesh->createAABBCollisionDetector(true, true);

    world->addChild(m_pMesh);
    m_pMesh->computeGlobalPositions();

    // User data points to the OscObject, used for identification
    // during object contact.
    m_pMesh->setUserData(this, 1);
}

OscMeshCHAI::~OscMeshCHAI()
{
    if (m_pMesh)
        m_pMesh->getParent()->deleteChild(m_pMesh);
}

void OscMeshCHAI::on_size()
{
    m_pMesh->computeBoundaryBox(true);
    cVector3d vmin(m_pMesh->getBoundaryMin());
    cVector3d vmax(m_pMesh->getBoundaryMax());
    cVector3d scale(vmax - vmin);
    scale.x = m_size.x / scale.x;
    scale.y = m_size.y / scale.y;
    scale.z = m_size.z / scale.z;
    m_pMesh->scale(scale);
}

/****** OscCursorCHAI ******/

OscCursorCHAI::OscCursorCHAI(cWorld *world, const char *name, OscBase *parent)
    : OscSphere(NULL, name, parent), CHAIObject(world)
{
    // create the cursor object
    m_pCursor = new cMeta3dofPointer(world);
    world->addChild(m_pCursor);

    // User data points to the OscObject, used for identification
    // during object contact.
    m_pCursor->setUserData(this, 1);

    // replace the potential proxy algorithm with our own
    cGenericPointForceAlgo *old_proxy, *new_proxy;
    old_proxy = m_pCursor->m_pointForceAlgos[1];
    new_proxy = new cODEPotentialProxy(
        dynamic_cast<cPotentialFieldForceAlgo*>(old_proxy));
    m_pCursor->m_pointForceAlgos[1] = new_proxy;
    delete old_proxy;

    if (m_pCursor->initialize()) {
        m_bInitialized = false;
        printf("Could not initialize haptics.\n");
    } else {
        m_bInitialized = true;
        m_pCursor->start();
    }

    // rotate the cursor to match visual rotation
    m_pCursor->rotate(cVector3d(0,0,1),-90.0*M_PI/180.0);

    // make it a cursor tuned for a dynamic environment
    ((cProxyPointForceAlgo*)m_pCursor->m_pointForceAlgos[0])
        ->enableDynamicProxy(true);

    // this is necessary for the above rotation to take effect
    m_pCursor->computeGlobalPositions();
}

OscCursorCHAI::~OscCursorCHAI()
{
    if (m_pCursor)
        m_pCursor->getParent()->deleteChild(m_pCursor);
}

void OscCursorCHAI::on_radius()
{
    printf("OscCursorCHAI::on_radius(). radius = %f\n", m_radius.m_value);

    if (!m_pCursor)
        return;

    m_pCursor->setRadius(m_radius.m_value);
}
