#include "vertexdisplay.h"

VertexDisplay::VertexDisplay(OpenGLContext *context)
    :Drawable(context)
{}

void VertexDisplay::updateVertex(Vertex* newVal)
{
    src = newVal;
}

Vertex *VertexDisplay::getSource()
{
    return src;
}


void VertexDisplay::create()
{
    std::vector<glm::vec4> pos;
    std::vector<glm::vec4> col;
    std::vector<GLuint> idx;
    std::vector <int> jnt;
    std::vector <float> inf;
    bool vertsBound = false;


    pos.push_back(glm::vec4(src->pos, 1));
    col.push_back(glm::vec4(1.f, 0.f, 0.f, 1.f));
    idx.push_back(0);

    if (src->bound){
        vertsBound = true;
        jnt.push_back(src->skin[0]->iD);
        jnt.push_back(src->skin[1]->iD);

        inf.push_back(src->influence[0]);
        inf.push_back(src->influence[1]);
    }

    count = 1;

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

// Overriden to set GL_POINTS as opposed to GL_TRIANGLES
GLenum VertexDisplay::drawMode()
{
    return GL_POINTS;
}
