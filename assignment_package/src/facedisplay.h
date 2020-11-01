#ifndef FACEDISPLAY_H
#define FACEDISPLAY_H

#include <drawable.h>
#include <face.h>

class FaceDisplay : public Drawable
{
private:

    // The face which is represented by this display
    Face *src;
public:
    FaceDisplay(OpenGLContext *context);

    void create() override;

    // Overriden to set GL_LINES as opposed to GL_TRIANGLES
    GLenum drawMode() override;

    // Change which Vertex representedVertex points to
    void updateFace(Face* newVal);

    // Returns the src variable
    Face *getSource();
};

#endif // FACEDISPLAY_H
