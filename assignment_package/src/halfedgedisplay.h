#ifndef HALFEDGEDISPLAY_H
#define HALFEDGEDISPLAY_H

#include <drawable.h>
#include <halfedge.h>

class HalfEdgeDisplay : public Drawable
{
private:

    // The half edge which is represented by this display
    HalfEdge *src;
public:
    HalfEdgeDisplay(OpenGLContext *context);

    void create() override;

    // Overriden to set GL_LINES as opposed to GL_TRIANGLES
    GLenum drawMode() override;

    // Change which Vertex representedVertex points to
    void updateHalfEdge(HalfEdge* newVal);

    // Returns the src variable
    HalfEdge *getSource();
};

#endif // HALFEDGEDISPLAY_H
