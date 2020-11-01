#include "halfedgedisplay.h"

HalfEdgeDisplay::HalfEdgeDisplay(OpenGLContext *context)
    :Drawable(context)
{}

void HalfEdgeDisplay::updateHalfEdge(HalfEdge* newVal)
{
    src = newVal;
}

HalfEdge *HalfEdgeDisplay::getSource()
{
    return src;
}


void HalfEdgeDisplay::create()
{
    std::vector<glm::vec4> pos;
    std::vector<glm::vec4> col;
    std::vector<GLuint> idx;
    std::vector <int> jnt;
    std::vector <float> inf;
    bool vertsBound = false;


    pos.push_back(glm::vec4(src->vert->pos, 1));
    pos.push_back(glm::vec4(src->sym->vert->pos, 1));
    col.push_back(glm::vec4(1.0, 1.0, 0, 1));
    col.push_back(glm::vec4(1.0, 0, 0, 1));
    idx.push_back(0);
    idx.push_back(1);

    if (src->vert->bound){
        vertsBound = true;
        jnt.push_back(src->vert->skin[0]->iD);
        jnt.push_back(src->vert->skin[1]->iD);
        jnt.push_back(src->sym->vert->skin[0]->iD);
        jnt.push_back(src->sym->vert->skin[1]->iD);

        inf.push_back(src->vert->influence[0]);
        inf.push_back(src->vert->influence[1]);
        inf.push_back(src->sym->vert->influence[0]);
        inf.push_back(src->sym->vert->influence[1]);
    }

    count = 2;

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
GLenum HalfEdgeDisplay::drawMode()
{
    return GL_LINES;
}
