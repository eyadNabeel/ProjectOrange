#ifndef MYGL_H
#define MYGL_H

#include <openglcontext.h>
#include <utils.h>
#include <shaderprogram.h>
#include <scene/squareplane.h>
#include "camera.h"
#include <mesh.h>
#include <vertexdisplay.h>
#include <facedisplay.h>
#include <halfedgedisplay.h>
#include <joint.h>

#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>


class MyGL
    : public OpenGLContext
{
    Q_OBJECT
private:
    SquarePlane m_geomSquare;// The instance of a unit cylinder we can use to render any cylinder
    ShaderProgram m_progLambert;// A shader program that uses lambertian reflection
    ShaderProgram m_progFlat;// A shader program that uses "flat" reflection (no shadowing at all)
    ShaderProgram m_progSkeleton;

    Mesh m_mesh; // our rendered mesh (initially a cube)

    Joint m_skeleton;

    GLuint vao; // A handle for our vertex array object. This will store the VBOs created in our geometry classes.
                // Don't worry too much about this. Just know it is necessary in order to render geometry.

    Camera m_glCamera;

    Drawable *selected; // stores a pointer to the selected vertex/half edge/face in order to render it

    glm::vec2 m_mousePosPrev; // stores the mouse position for rotation

    VertexDisplay vertDisp; // holds a display item representing the currently selected item in ui->verticesListWidget
    FaceDisplay faceDisp; // holds a display item representing the currently selected item in ui->facesListWidget
    HalfEdgeDisplay edgeDisp; // holds a display item representing the currently selected item in ui->halfEdgesListWidget

public:
    explicit MyGL(QWidget *parent = nullptr);
    ~MyGL();

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    // emits the ctxInitialized signal to refresh the QListWidgets and the QTreeWidget
    void emitInit();

    // deselects the previously selected edges/vertices/faces
    void resetSelection();

    // indicates if the m_mesh is bound to m_skeleton
    bool meshBound;

    // returns a pointer to m_mesh, used to add all the elements as QListWidgetItems to their respective QListWidget
    Mesh *getMesh();

    // these functions set the selected variable to the selected face/half edge/vertex and store its value in
    // vertDisp, edgeDisp, or faceDisp.
    void selectVert(QListWidgetItem *comp);
    void selectFace(QListWidgetItem *comp);
    void selectEdge(QListWidgetItem *comp);
    void selectJoint(QTreeWidgetItem *comp);

    // refreshes the mesh after any changes by destroying then creating m_mesh and selected, and then calling update()
    void refreshMesh();

    // Used to split an edge into two edges with a vertex in the middle
    void addVertex(QListWidgetItem *selected);

    // Used to triangulate polygons with more than 3 angles using the fan out method
    void triangulate(QListWidgetItem *selected);

    // Checks if the mouseclick was in the vicinity of a vertex
    Vertex *checkBounds(glm::vec2 pos);

    // Functions to perform the Catmull-Clark subdivision process
    void catmullClark();
    std::vector<glm::vec3> getSumVals(Vertex *v, std::vector<glm::vec3> &centroids);
    void quadrangulate(Face *f, glm::vec3 centroid);

    // takes in the root joint, and calls draw() on all its children
    // using the given ShaderProgram
    void drawJointFamily(Joint *j, ShaderProgram &shader);

    // Calls the function in Joint that loads a new skeleton from a .json file
    void initSkeleton(std::string fileName);

    // Returns m_skeleton
    Joint *getJoint();

    // Rotates the joint around axis by angle
    void rotateJoint(float angle, glm::vec3 axis);

    // Sets the x/y/z value for the selected vertex or joint to val
    void setSelectedX(double val);
    void setSelectedY(double val);
    void setSelectedZ(double val);

    // Binds all the vertices in the mesh to m_skeleton
    void bindVertices();

    // Sets all the bind matrices for all the joints
    void bindSkeleton();

    // Sets the influence of a joint for the selected vertex to val
    void setInfluence(Vertex *v, double val, int idx);

protected:
    void keyPressEvent(QKeyEvent *e);

    // Used for mouse controlled rotation of the camera
    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);

signals:

    // emitted after GLInitialize() is done and if any changes occur in the mesh. It causes mesh
    // components to be added to their respective QListWidgets.
    void ctxInitialized();

    // Causes the QListWidget's current item to change based on the selected component. Used for
    // keyboard events.
    void vertSelected(QListWidgetItem*);
    void faceSelected(QListWidgetItem*);
    void halfEdgeSelected(QListWidgetItem*);
};


#endif // MYGL_H
