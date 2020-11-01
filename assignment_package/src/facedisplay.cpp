#include "facedisplay.h"

FaceDisplay::FaceDisplay(OpenGLContext *context)
    :Drawable(context)
{}

void FaceDisplay::updateFace(Face* newVal)
{
    src = newVal;
}

Face *FaceDisplay::getSource()
{
    return src;
}

void FaceDisplay::create()
{
    std::vector<glm::vec4> pos;
    std::vector<glm::vec4> col;
    std::vector<GLuint> idx;
    std::vector <int> jnt;
    std::vector <float> inf;
    bool vertsBound = false;


    std::vector<Vertex*> edges;
    HalfEdge *firstEdge = src->edge;
    HalfEdge *currentEdge = firstEdge;
    int counter = 0;
    do{
        Vertex &v = *(currentEdge->vert);
        pos.push_back(glm::vec4(v.pos, 1));
        if (v.bound){
            vertsBound = true;
            jnt.push_back(v.skin[0]->iD);
            jnt.push_back(v.skin[1]->iD);

            inf.push_back(v.influence[0]);
            inf.push_back(v.influence[1]);
        }
        col.push_back(glm::vec4(1.f) - glm::vec4(src->colour, 1.f));
        currentEdge = currentEdge->next;

        // The indices of each vertex in the face followed by the vertex
        // which forms the other endpoint in the edge to form a line for
        // each edge.
        idx.push_back(counter);
        counter++;
        idx.push_back(counter);
    } while(currentEdge != firstEdge);

    // The last value would be over the index so it's popped off
    idx.pop_back();

    // We need the lines to loop around the face so the end index
    // has to be same as the beginning index
    idx.push_back(0);
    col.push_back(glm::vec4(1.f) - glm::vec4(src->colour, 1.f));
    col.push_back(glm::vec4(1.f) - glm::vec4(src->colour, 1.f));

    count = idx.size();

    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec4), pos.data(), GL_STATIC_DRAW);

    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, col.size() * sizeof(glm::vec4), col.data(), GL_STATIC_DRAW);

    if (vertsBound){
        generateJnt();
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufJnt);
        mp_context->glBufferData(GL_ARRAY_BUFFER, jnt.size() * sizeof(GLuint), jnt.data(), GL_STATIC_DRAW);

        generateInf();
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufInf);
        mp_context->glBufferData(GL_ARRAY_BUFFER, inf.size() * sizeof(GLfloat), inf.data(), GL_STATIC_DRAW);
    }
}

// Overriden to set GL_LINES as opposed to GL_TRIANGLES
GLenum FaceDisplay::drawMode()
{
    return GL_LINES;
}
