#include <cmath>
#include <functional>
#include <tuple>
#include <glm/glm.hpp>

/**
 * Typesafe way of computing of getting the sign of a value.
 * @param val the value to take the sign of
 * @return the sign (-1, 0, or 1) as an int
 */
template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

/**
 * Turn a vector 90 degrees counter clockwise
 * @param v the vector to rotate
 * @return the vector now rotated 90 degrees counter clockwise
 */
glm::vec2 turnNinetyDegrees(glm::vec2 v){
    return glm::vec2(-v.y, v.x);
}

/**
 * 
 */
glm::vec2 toClipSpace(float x, float y, float width, float height){
    float clip_x = 2.0 * x / width - 1.0;
    float clip_y = 2.0 * y / height - 1.0;
    return glm::vec2(clip_x, clip_y);
}

glm::vec2 toClipSpace(glm::vec2 p, float width, float height){
    return toClipSpace(p.x, p.y, width, height);
}

// sc = sin/cos of aperture
glm::vec3 sdgArc( glm::vec2 p, glm::vec2 sc, float ra, float rb )
{
    glm::vec2 q = p;
    float s = sgn(p.x); p.x = abs(p.x);
    if( sc.y*p.x > sc.x*p.y )
    {
        glm::vec2  w = p - ra*sc;
        float d = length(w);
        return glm::vec3( d-rb, glm::vec2(s*w.x,w.y)/d );
    }
    else
    {
        float l = length(q);
        float w = l - ra;
        return glm::vec3( abs(w)-rb, float(sgn(w))*q/l );
    }
}

glm::vec3 sdgCross( glm::vec2 p, glm::vec2 b ) 
{
    glm::vec2 s = glm::sign(p);
    p = glm::abs(p); 
    glm::vec2  q = ((p.y>p.x)? glm::vec2(p.y, p.x) : p ) - b;
    float h = fmax( q.x, q.y );
    glm::vec2  o = glm::max( ((h<0.0) ? glm::vec2(b.y-b.x,0.0)-q : q), glm::vec2(0.0, 0.0) );
    float l = length(o);
    glm::vec3  r = (h<0.0 && -q.x<l) ? glm::vec3(-q.x,1.0,0.0) : glm::vec3(l,o/l);

    return(p.y>p.x) ? glm::vec3( sgn(h)*r.x,  s.x*r.z, s.y*r.y) : glm::vec3( sgn(h)*r.x,  s.x*r.y, s.y*r.z );
}

