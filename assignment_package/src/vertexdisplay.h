#ifndef VERTEXDISPLAY_H
#define VERTEXDISPLAY_H
#include <vertex.h>
#include <drawable.h>

class VertexDisplay : public Drawable {
protected:

    // The vertex which is represented by this display
    Vertex *src;

public:
    VertexDisplay(OpenGLContext *context);

    void create() override;

    // Overriden to set GL_POINTS as opposed to GL_TRIANGLES
    GLenum drawMode() override;

    // Change which Vertex representedVertex points to
    void updateVertex(Vertex* newVal);

    // Returns the src variable
    Vertex *getSource();
};

#endif // VERTEXDISPLAY_H
