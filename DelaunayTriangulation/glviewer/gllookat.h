#if !defined(GLLOOKAT_INCLUDED_)
#define GLLOOKAT_INCLUDED_

float v_dot3f(float *x, float *y);
float v_length3f(float *x);
void v_normalize3f(float *x);
void v_normalize3f(float *v, float *x);
void v_add3f(float *v, float *v1, float *v2);
void v_substract3f(float *v, float *v1, float *v2);
void v_cross3f(float *v, float *x, float *y);

struct matrix4f
{
    float _data[4][4];
    matrix4f();
    matrix4f(matrix4f &m);
    matrix4f(float r0c0, float r1c0, float r2c0, float r3c0,
        float r0c1, float r1c1, float r2c1, float r3c1,
        float r0c2, float r1c2, float r2c2, float r3c2,
        float r0c3 = 0, float r1c3 = 0, float r2c3 = 0, float r3c3 = 1);
    void update(float r0c0, float r1c0, float r2c0, float r3c0,
        float r0c1, float r1c1, float r2c1, float r3c1,
        float r0c2, float r1c2, float r2c2, float r3c2,
        float r0c3, float r1c3, float r2c3, float r3c3);
    void diagonalize(float a);
    void copyto(matrix4f &m);
    const float* pointer() const;
};

// 注意这里针对的是列优先的opengl矩阵在行优先存储方式的C语言二维数组
void m4f_multiply(matrix4f &c, const matrix4f &a, const matrix4f &b);
// 计算旋转矩阵，注意是右乘以m
void m4f_rotate(matrix4f &m, float degree, float vec_x, float vec_y, float vec_z);

void lookat(matrix4f &m, float *camera, float *target, float *up);
void gllookat(matrix4f &m, float camera_x, float camera_y, float camera_z,
    float target_x, float target_y, float target_z,
    float up_x, float up_y, float up_z);
void gllookat(float camera_x, float camera_y, float camera_z,
    float target_x, float target_y, float target_z,
    float up_x, float up_y, float up_z);
void glperspective(matrix4f &r, float fovy, float aspect, float znear, float zfar);
void glperspective(float fovy, float aspect, float znear, float zfar);

#endif