#include "gllookat.h"
#include <memory.h>
#include <math.h>

#include <glew.h>

const float PI = 3.14159f;  // (acos(-1.0))

float v_dot3f(float *x, float *y)
{
    return x[0] * y[0] + x[1] * y[1] + x[2] * y[2];
}

float v_length3f(float *x)
{
    return (float)sqrt(v_dot3f(x, x));
}

void v_normalize3f(float *x)
{
    float module = v_length3f(x);
    x[0] = x[0] / module;
    x[1] = x[1] / module;
    x[2] = x[2] / module;
}

void v_normalize3f(float *v, float *x)
{
    memcpy(v, x, sizeof(float)*3);
    v_normalize3f(v);
}

void v_add3f(float *v, float *v1, float *v2)
{
    v[0] = v1[0] + v2[0];
    v[1] = v1[1] + v2[1];
    v[2] = v1[2] + v2[2];
}

void v_substract3f(float *v, float *v1, float *v2)
{
    v[0] = v1[0] - v2[0];
    v[1] = v1[1] - v2[1];
    v[2] = v1[2] - v2[2];
}

void v_cross3f(float *v, float *x, float *y)
{
    v[0] = x[1] * y[2] - x[2] * y[1];
    v[1] = x[2] * y[0] - x[0] * y[2];
    v[2] = x[0] * y[1] - x[1] * y[0];
}

void v_negate(float *v)
{
    v[0] = -v[0];
    v[1] = -v[1];
    v[2] = -v[2];
}

matrix4f::matrix4f()
{
    memset(_data, 0, sizeof(_data));
};

matrix4f::matrix4f(matrix4f& m)
{
    memcpy(_data, m._data, sizeof(_data));
};

matrix4f::matrix4f(float r0c0, float r1c0, float r2c0, float r3c0,
    float r0c1, float r1c1, float r2c1, float r3c1,
    float r0c2, float r1c2, float r2c2, float r3c2,
    float r0c3, float r1c3, float r2c3, float r3c3)
{
    this->update(r0c0, r1c0, r2c0, r3c0, r0c1, r1c1, r2c1, r3c1,
        r0c2, r1c2, r2c2, r3c2, r0c3, r1c3, r2c3, r3c3);
};

void matrix4f::update(float r0c0, float r1c0, float r2c0, float r3c0,
    float r0c1, float r1c1, float r2c1, float r3c1,
    float r0c2, float r1c2, float r2c2, float r3c2,
    float r0c3, float r1c3, float r2c3, float r3c3)
{
    //    r  c     r c
    _data[0][0] = r0c0;
    _data[0][1] = r1c0;
    _data[0][2] = r2c0;
    _data[0][3] = r3c0;
    _data[1][0] = r0c1;
    _data[1][1] = r1c1;
    _data[1][2] = r2c1;
    _data[1][3] = r3c1;
    _data[2][0] = r0c2;
    _data[2][1] = r1c2;
    _data[2][2] = r2c2;
    _data[2][3] = r3c2;
    _data[3][0] = r0c3;
    _data[3][1] = r1c3;
    _data[3][2] = r2c3;
    _data[3][3] = r3c3;
};

void matrix4f::diagonalize(float a)
{
    //    r  c
    _data[0][0] = a;
    _data[1][1] = a;
    _data[2][2] = a;
    _data[3][3] = a;
}

void matrix4f::copyto(matrix4f& m)
{
    memcpy(m._data, _data, sizeof(_data));
}

const float* matrix4f::pointer() const
{
    return &_data[0][0];
}

void m4f_multiply(matrix4f& c, const matrix4f& a, const matrix4f& b)
{
    for (int i = 0; i<4; ++i)
    {
        for (int k = 0; k<4; ++k)
        {
            float r = a._data[k][i];
            for (int j = 0; j<4; ++j)
            {
                c._data[j][i] += r*b._data[j][k];
            }
        }
    }
};

void m4f_rotate(matrix4f &m, float degree, float vec_x, float vec_y,
    float vec_z)
{
    double length = sqrt(vec_x*vec_x + vec_y*vec_y + vec_z*vec_z);
    double x = vec_x / length;
    double y = vec_y / length;
    double z = vec_z / length;

    double a = degree / 180 * 3.1415926;
    double s = sin(a);
    double c = cos(a);
    double t = 1.0f - c;

    float *matrix = (float*)m.pointer();

    matrix[0, 0] = (float)(t * x * x + c);
    matrix[0, 1] = (float)(t * x * y + s * z);
    matrix[0, 2] = (float)(t * x * z - s * y);
    matrix[0, 3] = 0.0f;
    matrix[1, 0] = (float)(t * x * y - s * z);
    matrix[1, 1] = (float)(t * y * y + c);
    matrix[1, 2] = (float)(t * y * z + s * x);
    matrix[1, 3] = 0.0f;
    matrix[2, 0] = (float)(t * x * z + s * y);
    matrix[2, 1] = (float)(t * y * z - s * x);
    matrix[2, 2] = (float)(t * z * z + c);
    matrix[2, 3] = 0.0f;
    matrix[3, 0] = 0.0f;
    matrix[3, 1] = 0.0f;
    matrix[3, 2] = 0.0f;
    matrix[3, 3] = 1.0f;
}

void gllookat(matrix4f &m, float camera_x, float camera_y, float camera_z,
    float target_x, float target_y, float target_z,
    float up_x, float up_y, float up_z)
{
    float camera[3];
    camera[0] = camera_x;
    camera[1] = camera_y;
    camera[2] = camera_z;

    float target[3];
    target[0] = target_x;
    target[1] = target_y;
    target[2] = target_z;

    float up[3];
    up[0] = up_x;
    up[1] = up_y;
    up[2] = up_z;

    float n[3];
    v_substract3f(n, camera, target);
    v_normalize3f(n);

    float u[3]; float up_normalized[3];
    v_normalize3f(up_normalized, up);
    v_cross3f(u, up_normalized, camera);
    v_normalize3f(u);

    float v[3];
    v_cross3f(v, n, u);

    matrix4f r(
        u[0], v[0], n[0], 0,    //col0
        u[1], v[1], n[1], 0,    //col1
        u[2], v[2], n[2], 0,    //col2
         0,    0,    0,   1);            //col3

    matrix4f t(
        1,          0,          0,          0,  //col0
        0,          1,          0,          0,  //col1
        0,          0,          1,          0,  //col2
        -camera[0], -camera[1], -camera[2], 1); //col3

    m4f_multiply(m, r, t);
}

void gllookat(float camera_x, float camera_y, float camera_z,
    float target_x, float target_y, float target_z,
    float up_x, float up_y, float up_z)
{
    matrix4f m;
    gllookat(m, camera_x, camera_y, camera_z, target_x, target_y, target_z,
        up_x, up_y, up_z);
    glLoadMatrixf(&m._data[0][0]);
}

void glperspective(matrix4f &r, float fovy, float aspect, float znear, float zfar)
{
    float c = float(PI / 180.0f);
    float d = float(1 / tan(fovy*c / 2));
    float a = -(zfar + znear) / (zfar - znear);
    float b = -2 * zfar*znear / (zfar - znear);

    r.update(d / aspect, 0, 0, 0,    //col0
             0, d, 0, 0,             //col1
             0, 0, a, -1,            //col2
             0, 0, b, 0);            //col3
}

void glperspective(float fovy, float aspect, float znear, float zfar)
{
    matrix4f r;
    glperspective(r, fovy, aspect, znear, zfar);
    glLoadMatrixf(&r._data[0][0]);
}
