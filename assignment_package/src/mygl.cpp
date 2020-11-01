#include "mygl.h"
#include <la.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>
#include <vertexdisplay.h>
#include <facedisplay.h>
#include <halfedgedisplay.h>
#include <smartpointerhelp.h>
#include <unordered_set>
#include <vertex.h>

MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      m_geomSquare(this),
      m_progLambert(this), m_progFlat(this),
      m_progSkeleton(this), m_mesh(Mesh(this)),
      m_skeleton(Joint(this)),
      m_glCamera(), selected(nullptr),
      m_mousePosPrev(), vertDisp(this),
      faceDisp(this), edgeDisp(this), meshBound(false)

{
    setFocusPolicy(Qt::StrongFocus);
}

MyGL::~MyGL()
{
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
    m_geomSquare.destroy();
    m_mesh.destroy();
    vertDisp.destroy();
    faceDisp.destroy();
    edgeDisp.destroy();
}

void MyGL::initializeGL()
{
    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    // Set the size with which points should be rendered
    glPointSize(5);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.5, 0.5, 0.5, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    // Create a cubic structure in m_mesh
    m_mesh.createCube();

    //Create the instances of Cylinder and Sphere.
    m_geomSquare.create();

    m_skeleton.addChild(glm::vec3(0, 1, 0));

    if (selected != nullptr){
        selected->create();
    }


    if (meshBound){
        bindVertices();
    }
    m_mesh.create();

    // Create and set up the diffuse shader
    m_progLambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    m_progFlat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");

    m_progSkeleton.create(":/glsl/skeleton.vert.glsl", ":/glsl/skeleton.frag.glsl");

    m_skeleton.create();
    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    glBindVertexArray(vao);

    emit ctxInitialized();
}

// Binds all the vertices in the mesh to the skeleton
void MyGL::bindVertices()
{
    for (uPtr<Vertex> &vert : m_mesh.vertices){
        vert.get()->bindJoints(&m_skeleton);
    }
}

// Sets all the bind matrices for the skeleton
void MyGL::bindSkeleton()
{
    std::vector<Joint*> skeleton;
    retrieveJoints(&m_skeleton, skeleton);
    for (Joint *j : skeleton){
        j->bindJoint();
    }
}

void MyGL::emitInit()
{
    emit ctxInitialized();
}

void MyGL::resetSelection()
{
    selected = nullptr;
    vertDisp.updateVertex(nullptr);
    edgeDisp.updateHalfEdge(nullptr);
    faceDisp.updateFace(nullptr);
}

Mesh *MyGL::getMesh()
{
    return &m_mesh;
}

void MyGL::resizeGL(int w, int h)
{
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    m_glCamera = Camera(w, h);
    glm::mat4 viewproj = m_glCamera.getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    m_progLambert.setViewProjMatrix(viewproj);
    m_progFlat.setViewProjMatrix(viewproj);
    m_progSkeleton.setViewProjMatrix(viewproj);

    printGLErrorLog();
}

Joint *MyGL::getJoint()
{
    return &m_skeleton;
}

void MyGL::drawJointFamily(Joint *j, ShaderProgram &shader)
{
    shader.draw(*j);
    for (uPtr<Joint> &child : j->children){
        drawJointFamily(child.get(), shader);
    }
}

//This function is called by Qt any time your GL window is supposed to update
//For example, when the function update() is called, paintGL is called implicitly.
void MyGL::paintGL()
{
    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    std::vector<Joint*> skeleton;
    retrieveJoints(&m_skeleton, skeleton);

    std::vector<glm::mat4> bindMats;
    std::vector<glm::mat4> transMats;

    // Sets the bind matrices if and only if the mesh is bound to the skeleton
    if (meshBound){
        for (Joint *jnt : skeleton){
            glm::mat4 bind = jnt->bind;
            glm::mat4 trans = jnt->getOverallTransformation();
            bindMats.push_back(bind);
            transMats.push_back(trans);
            glm::mat4 prod = bind * trans;
            prod = bind * trans;
        }
        m_progSkeleton.setBindMatrix(bindMats);
        m_progSkeleton.setTransMatrix(transMats);
    }

    m_progSkeleton.setViewProjMatrix(m_glCamera.getViewProj());
    m_progSkeleton.setModelMatrix(glm::mat4(1.f));

    m_progFlat.setViewProjMatrix(m_glCamera.getViewProj());
    m_progFlat.setBindMatrix(bindMats);
    m_progFlat.setTransMatrix(transMats);
    m_progLambert.setViewProjMatrix(m_glCamera.getViewProj());
    m_progLambert.setCamPos(m_glCamera.eye);
    m_progFlat.setModelMatrix(glm::mat4(1.f));

    // the model matrix is set to the identity so that we get
    // a neutral initial view
    glm::mat4 model = glm::mat4(1.0f);

    // m_progSkeleton is only used if the mesh is bound to the skeleton
    // otherwise, m_progFlat is used
    if (meshBound){
        m_progSkeleton.setModelMatrix(model);
        m_progSkeleton.draw(m_mesh);
    } else {
        m_progFlat.setModelMatrix(model);
        m_progFlat.draw(m_mesh);
    }

    // if selected isn't null, it's rendered, and the depth check
    // is overriden in order to render the component in front of
    // the mesh
    if (selected != nullptr){
        glDisable(GL_DEPTH_TEST);
        model = glm::mat4(1.0f);

        // m_progSkeleton is only used if the mesh is bound to the skeleton
        // otherwise, m_progFlat is used
        if (meshBound){
            m_progSkeleton.setModelMatrix(model);
            m_progSkeleton.draw(*selected);
        } else {
            m_progFlat.setModelMatrix(model);
            m_progFlat.draw(*selected);
        }
        glEnable(GL_DEPTH_TEST);
    }


    glDisable(GL_DEPTH_TEST);
    model = glm::mat4(1.0f);

    // m_progFlat is used for the joints so as to not disfigure the shape
    // of the joint pointers
    m_progFlat.setModelMatrix(model);
    drawJointFamily(&m_skeleton, m_progFlat);
    glEnable(GL_DEPTH_TEST);
}

void MyGL::selectVert(QListWidgetItem *comp)
{
    Vertex *v = dynamic_cast<Vertex*>(comp);
    Joint *j = dynamic_cast<Joint*>(selected);
    vertDisp.updateVertex(v);
    if (selected && j){
        j->selected = false;
    }
    selected = &vertDisp;
    refreshMesh();
}

void MyGL::selectFace(QListWidgetItem *comp)
{
    Face *f = dynamic_cast<Face*>(comp);
    Joint *j = dynamic_cast<Joint*>(selected);
    faceDisp.updateFace(f);
    if (selected && j){
        j->selected = false;
    }
    selected = &faceDisp;
    refreshMesh();
}

void MyGL::selectEdge(QListWidgetItem *comp)
{
    HalfEdge *e = dynamic_cast<HalfEdge*>(comp);
    Joint *j = dynamic_cast<Joint*>(selected);
    edgeDisp.updateHalfEdge(e);
    if (selected && j){
        j->selected = false;
    }
    selected = &edgeDisp;
    refreshMesh();
}

void MyGL::selectJoint(QTreeWidgetItem *comp)
{
    selected = dynamic_cast<Joint*>(comp);
    std::vector<Joint *> joints;
    retrieveJoints(&m_skeleton, joints);
    for (Joint *j : joints){
        j->selected = false;
    }
}

// Functions to change the selected vertex/joint's position to
// a given parameter val
void MyGL::setSelectedX(double val)
{
    Joint *currJ = dynamic_cast<Joint*>(selected);
    VertexDisplay *currV = dynamic_cast<VertexDisplay*>(selected);
    if (currJ){
        currJ->pos[0] = val;
    } else if (currV){
        currV->getSource()->pos[0] = val;
    }
    refreshMesh();
}

void MyGL::setSelectedY(double val)
{
    Joint *currJ = dynamic_cast<Joint*>(selected);
    VertexDisplay *currV = dynamic_cast<VertexDisplay*>(selected);
    if (currJ){
        currJ->pos[1] = val;
    } else if (currV){
        currV->getSource()->pos[1] = val;
    }
    refreshMesh();
}

void MyGL::setSelectedZ(double val)
{
    Joint *currJ = dynamic_cast<Joint*>(selected);
    VertexDisplay *currV = dynamic_cast<VertexDisplay*>(selected);
    if (currJ){
        currJ->pos[2] = val;
    } else if (currV){
        currV->getSource()->pos[2] = val;
    }
    refreshMesh();
}

// Set the influence of a joint on a vertex to val, and the other
// to 1 - val
void MyGL::setInfluence(Vertex *v, double val, int idx)
{
    v->influence[idx] = val;
    v->influence[(idx + 1) % 2]  = 1 - val;
    refreshMesh();
}

// Add a vertex in the middle of the selected HalfEdge
void MyGL::addVertex(QListWidgetItem *selected)
{
    // The operation is only performed if the halfEdge is selected
    if (selected == edgeDisp.getSource() && this->selected == &edgeDisp){
        HalfEdge *e1 = dynamic_cast<HalfEdge*>(selected);
        HalfEdge *e2 = e1->sym;
        m_mesh.vertices.push_back(mkU<Vertex>());
        m_mesh.halfEdges.push_back(mkU<HalfEdge>());
        HalfEdge *e1b = m_mesh.edgePtr(m_mesh.halfEdges.size() - 1);
        Vertex *v = m_mesh.vertPtr(m_mesh.vertices.size() - 1);
        m_mesh.halfEdges.push_back(mkU<HalfEdge>());
        HalfEdge *e2b = m_mesh.edgePtr(m_mesh.halfEdges.size() - 1);
        e1b->vert = e1->vert;
        e1->vert->edge = e1b;
        e2b->vert = e2->vert;
        e1->vert = v;
        e2->vert = v;
        e1b->face = e1->face;
        e2b->face = e2->face;
        e1b->next = e1->next;
        e2b->next = e2->next;
        e1->next = e1b;
        e2->next = e2b;
        e1b->sym = e2;
        e2->sym = e1b;
        e2b->sym = e1;
        e1->sym = e2b;
        e2->face->edge = e2;
        e1->face->edge = e1;
        v->pos = glm::vec3((e1b->vert->pos + e2b->vert->pos) * 0.5f);
        v->edge = e1;

        // after the operation is over, the QListWidget is re-initialized
        // and the scene refreshed
        emit ctxInitialized();
        refreshMesh();
    }
}

// Noise functio to give pseudorandom colours to new faces created
// by triangulation
float noise3D(glm::vec2 p) {
    using namespace glm;
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}


void MyGL::triangulate(QListWidgetItem *selected)
{
    // The operation is only performed if the face is selected or if there is no selection yet; the program is
    // pre-triangulating.
    if ((selected == faceDisp.getSource() && this->selected == &faceDisp) || this->selected == nullptr){
        Face *f = dynamic_cast<Face*>(selected);
        HalfEdge *pivot = f->edge;

        // If the face has three edges/vertices, don't do anything
        if (pivot->next->next->next == pivot){
            return;
        }
        do {
            m_mesh.faces.push_back(mkU<Face>());
            Face *fPtr = m_mesh.facePtr(m_mesh.faces.size() - 1);
            m_mesh.halfEdges.push_back(mkU<HalfEdge>());
            HalfEdge *e0 = m_mesh.edgePtr(m_mesh.halfEdges.size() - 1);
            m_mesh.halfEdges.push_back(mkU<HalfEdge>());
            HalfEdge *e1 = m_mesh.edgePtr(m_mesh.halfEdges.size() - 1);
            e0->sym = e1;
            e1->sym = e0;
            e0->next = pivot->next;
            e0->vert = pivot->vert;
            e1->next = pivot->next->next->next;
            pivot->next->next->next = e0;
            e1->vert = pivot->next->next->vert;
            pivot->next->face = fPtr;
            pivot->next->next->face = fPtr;
            e0->face = fPtr;
            fPtr->edge = e0;
            e1->face = pivot->face;
            pivot->next = e1;
            if ((noise3D(glm::vec2(fPtr->iD, glm::sin(m_mesh.vertices.size() * 7.64)))) > 0.25){
                fPtr->colour = f->colour * noise3D(glm::vec2(fPtr->iD, m_mesh.halfEdges.size()));
            } else {
                fPtr->colour = f->colour / noise3D(glm::vec2(fPtr->iD, m_mesh.halfEdges.size()));
            }
        } while (pivot->next->next->next != pivot);

        // after the operation is over, the QListWidget is re-initialized
        // and the scene refreshed
        emit ctxInitialized();
        refreshMesh();
    }
}

// Gets the sum of adjacent midpoints to an original vertex, as well as
// the number of adjacent faces and the sum of adjacent centroids in
// order to help find the smoothed positions for the original vertices
std::vector<glm::vec3> MyGL::getSumVals(Vertex *v, std::vector<glm::vec3> &centroids)
{
    std::vector<glm::vec3> sums;
    std::vector<int> faces;
    std::vector<int> verts;
    sums.push_back(glm::vec3());
    sums.push_back(glm::vec3());
    sums.push_back(glm::vec3());
    HalfEdge *curr = v->edge->next;
    HalfEdge *first = curr;
    if (curr->vert != v){
        curr = curr->sym;
        first = curr;
    }
    do {
        sums[0] += curr->sym->vert->pos;
        sums[1] += centroids[curr->sym->face->iD];
        sums[2] += glm::vec3(1.f);
        curr = curr->next->sym;
    } while (curr != first);
    return sums;
}

// Checks if the given half edge pointer is in the std::vector that's
// given
bool contains(std::vector<HalfEdge*> container, HalfEdge* element){
    for (HalfEdge* item : container){
        if (item == element){
            return true;
        }
    }
    return false;
}

// Takes a face and quadrangulates it and adds the centroid as
// a vertex
void MyGL::quadrangulate(Face *f, glm::vec3 centroid)
{
    m_mesh.vertices.push_back(mkU<Vertex>());
    Vertex *centroidV = m_mesh.vertPtr(m_mesh.vertices.size() - 1);
    centroidV->pos = centroid;
    HalfEdge *curr = f->edge->next;
    HalfEdge *ref = curr;
    Vertex *trailing = f->edge->vert;
    HalfEdge *finalSym;
    do {
        // New face to which halfEdges attach
        Face *currFace;
        if (curr == ref){
            currFace = f;
        } else {
            currFace = m_mesh.facePtr(m_mesh.faces.size() - 1);
        }

        // Stored in advance because value changes
        HalfEdge *next = curr->next->next;

        // Vertex for eFin to attach to
        Vertex *nextVert = curr->next->vert;

        // New edges created
        m_mesh.halfEdges.push_back(mkU<HalfEdge>());
        HalfEdge *eInit = m_mesh.edgePtr(m_mesh.halfEdges.size() - 1);
        m_mesh.halfEdges.push_back(mkU<HalfEdge>());
        HalfEdge *eFin = m_mesh.edgePtr(m_mesh.halfEdges.size() - 1);

        // Setting all edges to face
        curr->face = currFace;
        curr->next->face = currFace;
        eInit->face = currFace;
        eFin->face = currFace;

        // Setting face to new edge
        currFace->edge = curr;
        if ((noise3D(glm::vec2(currFace->iD, glm::sin(m_mesh.vertices.size() * 7.64)))) > 0.4){
            currFace->colour = f->colour * noise3D(glm::vec2(currFace->iD, m_mesh.halfEdges.size()));
        } else {
            currFace->colour = f->colour / noise3D(glm::vec2(currFace->iD, m_mesh.halfEdges.size()));
        }

        // Setting new next cycle
        curr->next->next = eInit;
        eInit->next = eFin;
        eFin->next = curr;
        eInit->vert = centroidV;
        eFin->vert = trailing;

        // Setting sym values
        if (curr != ref){
            eFin->sym = m_mesh.edgePtr(m_mesh.halfEdges.size() - 4);
            m_mesh.edgePtr(m_mesh.halfEdges.size() - 4)->sym = eFin;
        } else {
            finalSym = eFin;
        }

        // Creating a new face
        if (next != ref){
            m_mesh.faces.push_back(mkU<Face>());
        }

        // Passing on curr and trailing
        curr = next;
        trailing = nextVert;
    } while(curr != ref);

    // Setting the final sym
    finalSym->sym = m_mesh.edgePtr(m_mesh.halfEdges.size() - 2);
    m_mesh.edgePtr(m_mesh.halfEdges.size() - 2)->sym = finalSym;
}

// This function does all the work needed to perform Catmull-Clark
// subdivision
void MyGL::catmullClark()
{
    // sets the number of original vertices in the mesh, so we
    // only loop at the first vertices in the mesh until we reach
    // this number of vertices
    int vertCutoff = m_mesh.vertices.size();
    std::vector<HalfEdge*> edges;
    std::vector<glm::vec3> centroids;

    // Looping on the faces to store the original edges (excluding their syms),
    // as well as the centroids
    for (int i = 0; i < m_mesh.faces.size(); i++){
        Face *f = m_mesh.facePtr(i);
        HalfEdge *curr = f->edge;
        glm::vec3 centroid;
        int n = 0;
        do {
            if (!contains(edges, curr) && !contains(edges, curr->sym)){
                edges.push_back(curr);
            }
            centroid += curr->vert->pos;
            curr = curr->next;
            n++;
        } while (curr != f->edge);
        centroid /= n;
        n = 0;
        centroids.push_back(centroid);
        Vertex v;
        v.pos = centroid;
        selectVert(&v);
    }

// Assigning midpoint positions
    for (HalfEdge *e : edges){
        edgeDisp.updateHalfEdge(e);
        selected = &edgeDisp;
        glm::vec3 midpoint = (e->vert->pos +
                              e->sym->vert->pos +
                              centroids[e->face->iD] +
                              centroids[e->sym->face->iD])/4.f;
        addVertex(e);
        m_mesh.vertPtr(m_mesh.vertices.size() - 1)->pos = midpoint;
    }
    selected = nullptr;
    for (int i = 0; i < vertCutoff; i++){
        Vertex *v = m_mesh.vertPtr(i);
        std::vector<glm::vec3> sums = getSumVals(v, centroids);
        v->pos = ((sums[2].x - 2) * v->pos)/sums[2].x +
                sums[0]/(sums[2].x * sums[2].x) +
                sums[1]/(sums[2].x * sums[2].x);
    }
    for (int i = 0; i < centroids.size(); i++){
        Face *f = m_mesh.facePtr(i);
        quadrangulate(f, centroids[f->iD]);
    }
    refreshMesh();
    emit ctxInitialized();
}

void MyGL::rotateJoint(float angle, glm::vec3 axis)
{
    Joint *j = dynamic_cast<Joint*>(selected);
    if (j){
        j->rotate(angle, axis);
    }
    refreshMesh();
}

void MyGL::keyPressEvent(QKeyEvent *e)
{
    float amount = 0.5f;
    if(e->modifiers() & Qt::ShiftModifier){
        amount = 2.5f;

    }
    // http://doc.qt.io/qt-5/qt.html#Key-enum
    // This could all be much more efficient if a switch
    // statement were used
    if (e->key() == Qt::Key_Escape) {
        QApplication::quit();
    } else if (e->key() == Qt::Key_Right) {
        m_glCamera.RotateTheta(-amount);
    } else if (e->key() == Qt::Key_Left) {
        m_glCamera.RotateTheta(amount);
    } else if (e->key() == Qt::Key_Up) {
        m_glCamera.RotatePhi(-amount);
    } else if (e->key() == Qt::Key_Down) {
        m_glCamera.RotatePhi(amount);
    } else if (e->key() == Qt::Key_1) {
        m_glCamera.fovy += amount;
    } else if (e->key() == Qt::Key_2) {
        m_glCamera.fovy -= amount;
    } else if (e->key() == Qt::Key_W) {
        m_glCamera.Zoom(amount);
    } else if (e->key() == Qt::Key_S) {
        m_glCamera.Zoom(-amount);
    } else if (e->key() == Qt::Key_D) {
        m_glCamera.TranslateRight(amount);
    } else if (e->key() == Qt::Key_A) {
        m_glCamera.TranslateRight(-amount);
    } else if (e->key() == Qt::Key_Q) {
        m_glCamera.TranslateUp(-amount);
    } else if (e->key() == Qt::Key_E) {
        m_glCamera.TranslateUp(amount);
    } else if (e->key() == Qt::Key_R) {
        m_glCamera = Camera(this->width(), this->height());
    } else if (e->key() == Qt::Key_N && selected == &edgeDisp && selected) {
        HalfEdge *e = edgeDisp.getSource()->next;
        emit halfEdgeSelected(e);
    } else if (e->key() == Qt::Key_M && selected == &edgeDisp && selected) {
        HalfEdge *e = edgeDisp.getSource()->sym;
        emit halfEdgeSelected(e);
    } else if (e->key() == Qt::Key_V && selected == &edgeDisp && selected) {
        Vertex *v = edgeDisp.getSource()->vert;
        emit vertSelected(v);
    } else if (e->key() == Qt::Key_H && selected == &vertDisp && selected) {
        HalfEdge *e = vertDisp.getSource()->edge;
        emit halfEdgeSelected(e);
    } else if (e->key() == Qt::Key_F && selected == &edgeDisp && selected) {
        Face *f = edgeDisp.getSource()->face;
        emit faceSelected(f);
    } else if (e->key() == Qt::Key_H && e->modifiers() & Qt::ShiftModifier
               && selected == &faceDisp && selected){
        HalfEdge *e = faceDisp.getSource()->edge;
        emit halfEdgeSelected(e);
    }
    m_glCamera.RecomputeAttributes();
    update();  // Calls paintGL, among other things

}

// given a mouse position, checks if it's adjacent to a vertex
Vertex *MyGL::checkBounds(glm::vec2 pos)
{
    Vertex *vx = nullptr;
    glm::vec4 vxPos;
    Vertex *curr;
    glm::vec4 currPos;
    glm::vec4 diffStd(100.0);
    for (int i = 0; i < m_mesh.vertices.size(); i++){
        curr = m_mesh.vertPtr(i);
        currPos = m_glCamera.getViewProj() * glm::vec4(curr->pos, 1);
        glm::vec4 diff = glm::abs((glm::vec4(pos, 0, 1) - currPos));
        if (glm::length(glm::vec2(diff.x, diff.y)) <= 0.25 && diff[2] < diffStd[2]){
            if (!vx) {
                vx = curr;
                diffStd = diff;
                vxPos = currPos;
            } else if (glm::length(diff) < glm::length(diffStd)){
                vx = curr;
                diffStd = diff;
                vxPos = currPos;
            }
        }
    }
    return vx;
}

// if the buttons are pressed, it calculates how much the mouse has moved
// and rotates or pans the camera based on that
void MyGL::mouseMoveEvent(QMouseEvent *e)
{
    glm::vec2 pos(e->pos().x(), e->pos().y());
    if(e->buttons() & Qt::LeftButton)
    {
        // Rotation
        glm::vec2 diff = 0.025f * (pos - m_mousePosPrev);
        m_mousePosPrev = pos;
        m_glCamera.RotateTheta(-diff.x);
        m_glCamera.RotatePhi(-diff.y);
    }
    else if(e->buttons() & Qt::RightButton)
    {
        // Panning
        glm::vec2 diff = 0.025f * (pos - m_mousePosPrev);
        m_mousePosPrev = pos;
        m_glCamera.TranslateRight(-diff.x);
        m_glCamera.TranslateUp(diff.y);
    }
    m_glCamera.RecomputeAttributes();
    update();
}

// Sets the position variable for the mouse when clicked in order to
// compare it to it's position when the mouse is released and rotate or pan
// accordingly
void MyGL::mousePressEvent(QMouseEvent *e)
{
    if(e->buttons() & (Qt::LeftButton | Qt::RightButton))
    {
        m_mousePosPrev = glm::vec2(e->pos().x(), e->pos().y());
    }
    float x;
    if (int(e->pos().x()/618) % 2 == 1){
        x = ((618 - e->pos().x() % 618) - 309) * ((m_glCamera.r * 1.1)/309.0);
    } else {
        x = ((e->pos().x() % 618) - 309) * ((m_glCamera.r * 1.1)/309.0);
    }
    float y;
    if (int(e->pos().y()/432) % 2 == 1){
        y = (216 - (432 - (e->pos().y() % 432))) * ((m_glCamera.r * 1.1)/216.0);
    } else {
        y = (216 - (e->pos().y() % 432)) * ((m_glCamera.r * 1.1)/216.0);
    }

    // This segment is for detecting vertices that the mouse clicks on in order to
    // select them
    Vertex *v = checkBounds(glm::vec2(x, y));
    if (v){
        emit vertSelected(v);
    }
    m_glCamera.RecomputeAttributes();
    update();
}

// refreshes mygl and the mesh and the selected component
void MyGL::refreshMesh()
{
    m_mesh.destroy();
    m_mesh.create();
    m_skeleton.destroy();
    m_skeleton.create();
    if (selected){
        selected->destroy();
        selected->create();
    }
    update();
}

// Load the skeleton from a .json file
void MyGL::initSkeleton(std::string fileName)
{
    m_skeleton.loadJSON(fileName);
}
