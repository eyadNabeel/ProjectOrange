#include "camera.h"

#include <la.h>
#include <iostream>


Camera::Camera():
    Camera(400, 400)
{
    look = glm::vec3(0,0,-1);
    up = glm::vec3(0,1,0);
    right = glm::vec3(1,0,0);
    translate = glm::vec3(0, 0, 0);
}

Camera::Camera(unsigned int w, unsigned int h):
    Camera(w, h, glm::vec3(0,0,0), glm::vec3(0,0,0), glm::vec3(0,1,0))
{}

Camera::Camera(unsigned int w, unsigned int h, const glm::vec3 &e, const glm::vec3 &r, const glm::vec3 &worldUp):
    fovy(45),
    width(w),
    height(h),
    near_clip(0.1f),
    far_clip(1000),
    r(10.f),
    translate{0, 0, 0},
    eye(e),
    ref(r),
    world_up(worldUp)
{
    RecomputeAttributes();
}

Camera::Camera(const Camera &c):
    fovy(c.fovy),
    width(c.width),
    height(c.height),
    near_clip(c.near_clip),
    far_clip(c.far_clip),
    r(c.r),
    theta(c.theta),
    phi(c.phi),
    translate(c.translate),
    aspect(c.aspect),
    eye(c.eye),
    ref(c.ref),
    look(c.look),
    up(c.up),
    right(c.right),
    world_up(c.world_up),
    V(c.V),
    H(c.H)
{}


void Camera::RecomputeAttributes()
{
    orientCamera();

    float tan_fovy = tan(glm::radians(fovy/2));
    float len = glm::length(ref - eye);
    aspect = width / static_cast<float>(height);
    V = up*len*tan_fovy;
    H = right*len*aspect*tan_fovy;
}

// This function sets the look, right, up, eye, and ref vectors based on
// theta, phi, r, and translation values
void Camera::orientCamera()
{
    glm::mat4 transform = glm::translate(glm::mat4(), translate);
    transform *= glm::rotate(glm::mat4(), theta, glm::vec3(0, 1, 0));
    transform *= glm::rotate(glm::mat4(), phi, glm::vec3(1, 0, 0));
    transform *= glm::translate(glm::mat4(), r * glm::vec3(0, 0, 1));
    eye = glm::vec3(transform * glm::vec4(0, 0, 0, 1));
    up = glm::vec3(transform * glm::vec4(0, 1, 0, 0));
    right = glm::vec3(transform * glm::vec4(1, 0, 0, 0));
    look = glm::vec3(transform * glm::vec4(0, 0, -1, 0));
    ref = glm::vec3(glm::translate(glm::mat4(), translate) * glm::vec4(0, 0, 0, 1));
}

glm::mat4 Camera::getViewProj()
{
    return glm::perspective(glm::radians(fovy), width / (float)height, near_clip, far_clip) * glm::lookAt(eye, ref, up);
}

void Camera::RotateTheta(float deg)
{
    theta += deg;
    RecomputeAttributes();
}

void Camera::RotatePhi(float deg)
{
    phi += deg;
    RecomputeAttributes();
}

void Camera::Zoom(float amt)
{
    r -= amt;
    RecomputeAttributes();
}

void Camera::TranslateRight(float amt)
{
    translate[0] += amt;
    RecomputeAttributes();
}

void Camera::TranslateUp(float amt)
{
    translate[1] += amt;
    RecomputeAttributes();
}
