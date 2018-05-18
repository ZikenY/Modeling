#include <windows.h>
#include <stdexcept>
#include <iostream>
#include <cmath>


#include <glew.h>
#include <glut.h>
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "opengl32.lib")  
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "glut32.lib")

#include "gllookat.h"
#include "RGObjLoader.h"

using namespace std;

vector<shape_r> shapes;
vector<material_r> materials;
GLuint polygonmode = GL_LINE;

#define pi (acos(-1.0))
float RAD = pi / 180.0f;
float phi = -90.0f;
float sita = -90.0f;
int oldmx = -1;
int oldmy = -1;
int old_x = -1;
int old_y = -1;
float max_r = 1.f;
float old_r = max_r;
float r = max_r;    //the radius of camera spherical surface based on x,y

bool scence_update = true;
bool leftbotton = false;
bool rightbotton = false;

GLfloat center_x = 0;
GLfloat center_y = 0;
GLfloat center_z = 0;
GLfloat up_x = 0;
GLfloat up_y = 1;

// control model position & angle transformation
GLfloat rotate_x = 0;
GLfloat rotate_y = 0;
GLfloat rotate_z = 0;
GLfloat translate_x = 0;
GLfloat translate_y = 0;
GLfloat translate_z = 0;

void glidle()
{
    if (scence_update)
    {
        glutPostRedisplay();
        scence_update = false;
    }
}

void init()
{
    polygonmode = GL_LINE;
    glDisable(GL_CULL_FACE);
    if (mtl_existed)
    {
        polygonmode = GL_FILL;
        glEnable(GL_CULL_FACE);
    }
    glPolygonMode(GL_FRONT_AND_BACK, polygonmode);
    glClearColor(.0f, .0f, .0f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glShadeModel(GL_SMOOTH);

    // 启用颜色追踪  ------------------------------------------
//    glEnable(GL_COLOR_MATERIAL);
    //物体正面的材料环境颜色和散射颜色，追踪glColor所设置的颜色
//    glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);
    // --------------------------------------------------------

    GLfloat ambient_light0[] = {0.3f, 0.3f, 0.3f, 1.0f};
    GLfloat diffuse_light0[] = {0.75f, 0.75f, 0.75f, 1.0f};
    GLfloat specular_light0[] = {0.5f, 0.4f, 0.4f, 1.0f};
    GLfloat light0_position[] = {0.0f, -40.0f, 100.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_light0);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular_light0);
    glLightfv(GL_LIGHT0, GL_POSITION, light0_position);

    GLfloat ambient_light1[] = {0.0f, 0.0f, 0.0f, 1.0f};
    GLfloat diffuse_light1[] = {0.45f, 0.45f, 0.45f, 1.0f};
    GLfloat specular_light1[] = {0.85f, 0.9f, 0.9f, 1.0f};
    glLightfv(GL_LIGHT1, GL_AMBIENT, ambient_light1);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse_light1);
    glLightfv(GL_LIGHT1, GL_SPECULAR, specular_light1);

    // globe ambient light
    GLfloat lmodel_ambient[] = {.6f, .6f, .6f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

    // 独立计算反射光，用于贴图
//    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);

    glDisable(GL_LIGHTING);
    if (mtl_existed)
        glEnable(GL_LIGHTING);

    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
}

void renderscene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //----------------------------------------------------------
    // computer the camera position and angle
    double camera_y = r*cos(RAD*phi);
    double camera_x = r*sin(RAD*phi)*cos(RAD*sita);
    double camera_z = r*sin(RAD*phi)*sin(RAD*sita);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gllookat(
//    gluLookAt(
        camera_x, camera_y, camera_z,
        center_x, center_y, center_z,
        up_x, up_y, 0
        );
    //----------------------------------------------------------

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glPushMatrix();
    glRotatef(rotate_x, 1.0f, 0.0f, 0.0f);
    glRotatef(rotate_y, 0.0f, 1.0f, 0.0f);
    glRotatef(rotate_z, 0.0f, 0.0f, 1.0f);
    glTranslatef(translate_x, translate_y, translate_z);

    for (int i=0; i<shapes.size(); i++)
    {
        // setup the materials quality of light
        glMaterialfv(GL_FRONT, GL_AMBIENT, materials[shapes[i].mesh.material_ids[0]].ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, materials[shapes[i].mesh.material_ids[0]].diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, materials[shapes[i].mesh.material_ids[0]].specular);
        glMaterialfv(GL_FRONT, GL_SHININESS, &materials[shapes[i].mesh.material_ids[0]].shininess);

        glVertexPointer(3, GL_FLOAT, 0, &shapes[i].mesh.positions[0]);
        glNormalPointer(GL_FLOAT, 0, &shapes[i].mesh.normals[0]);
        glDrawElements(GL_TRIANGLES, shapes[i].mesh.indices.size(), //indices.size()除以3就是多边形数目
            GL_UNSIGNED_INT, &shapes[i].mesh.indices[0]);
    }

    glPopMatrix();

    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    glutSwapBuffers();
}

void reshape(GLsizei w, GLsizei h)
{
    // glOrtho函数只是负责使用什么样的视景体来截取图像，并不负责使用某种规则把图像呈现在屏幕上。
    // glViewport负责把视景体截取的图像按照怎样的高和宽显示到屏幕上。
    glViewport(0, 0, w, h);
    GLfloat fAspect = (GLfloat)w / (GLfloat)h;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glperspective(65.0f, fAspect, 1.0f, 1000.0f);
//    gluPerspective(65.0f, fAspect, 1.0f, 1000.0f);
}

void Mouse(int button, int state, int x, int y)
{
    leftbotton = false;
    rightbotton = false;

    if (button == GLUT_LEFT_BUTTON
        || button == GLUT_RIGHT_BUTTON
        || button == GLUT_MIDDLE_BUTTON
        || button == GLUT_WHEEL_UP
        || button == GLUT_WHEEL_DOWN)
    {
        scence_update = true;
    }

    if (button == GLUT_LEFT_BUTTON)
    {
        leftbotton = true;
        if (state == GLUT_DOWN)
        {
            oldmx = x, oldmy = y;
        }
    }

    if (button == GLUT_RIGHT_BUTTON)
    {
         rightbotton = true;
         old_r = r;
         old_x = x;
         old_y = y;
    }

    if (button == GLUT_MIDDLE_BUTTON)
    {
        up_x = 0;
        up_y = 1;
        center_x = 0.0f;
        center_y = 0.0f;
        center_z = 0.0f;
        phi = -90.0f;
        sita = -90.0f;
        r = max_r * 2;
        glutPostRedisplay();
    }

    if (button == GLUT_WHEEL_UP)
    {
        r = r + max_r/5;
    }

    if (button == GLUT_WHEEL_DOWN)
    {
        r = r - max_r/5;
    }
}

void onMouseDrag(int x, int y)
{
    if (leftbotton)
    {
//        sita += x - oldmx;
//        phi += y - oldmy;
        translate_x = translate_x + x - oldmx;
        translate_y = translate_y - y + oldmy;
    }

    if (rightbotton)
    {
        r = old_r - ((y-old_y)/max_r*1.0f);
        if (r < -max_r/3.0f)
        {
            r = -max_r/3.0f;
        }
        else if (r > max_r*3.0f)
        {
            r = max_r*3.0f;
        }

        up_x += (x-old_x)/46654.0f;
        if (up_x < -1)
            up_x = -1;
        else if(up_x > 1)
            up_x = 1;

        up_y = sqrt(1 - up_x*up_x);
    }

    oldmx = x;
    oldmy = y;

    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
    scence_update = true;

    if (key == 27)
        exit(0);

    switch(key)
    {
    case 32:    // space
        {
            if (polygonmode == GL_LINE)
            {
                polygonmode = GL_FILL;
                glEnable(GL_CULL_FACE);
            }
            else
            {
                polygonmode = GL_LINE;
                glDisable(GL_CULL_FACE);
            }

            glPolygonMode(GL_FRONT_AND_BACK, polygonmode);
        }
        break;

    case 119:   // w
        {
            translate_y += max_r / 10.0f;
//            center_y += max_r/10;
        }
        break;

    case 115:   // s
        {
            translate_y -= max_r / 10.0f;
//            center_y -= max_r/10;
        }
        break;

    case 97:   // a
        {
            translate_x -= max_r / 10.0f;
//            center_x -= max_r/10;
        }
        break;

    case 100:   // d
        {
            translate_x += max_r / 10.0f;
//            center_x += max_r/10;
        }
        break;

    case 113:   // q
        {
            translate_z += max_r / 10.0f;
        }
        break;

    case 101:   // e
        {
            translate_z -= max_r / 10.0f;
        }
        break;

    case 91:   // [
        {
            rotate_y += 10.0f;
        }
        break;

    case 93:   // ]
        {
            rotate_y -= 10.0f;
        }
        break;

    case 59:   // ;
        {
            rotate_z += 10.0f;
        }
        break;

    case 39:   // '
        {
            rotate_z -= 10.0f;
        }
        break;

    case 44:   // <
        {
            rotate_x += 10.0f;
        }
        break;

    case 46:   // >
        {
            rotate_x -= 10.0f;
        }
        break;

    case 8:   // backspace
        {
            rotate_x = 0;
            rotate_y = 0;
            rotate_z = 0;
            translate_x = 0;
            translate_y = 0;
            translate_z = 0;
        }
        break;

    case 61:   // +
        {
            r = r - max_r/5;
        }
        break;

    case 45:   // -
        {
            r = r + max_r/5;
        }
        break;

    default:
        {
        }
    }

    glutPostRedisplay();
}

void loadmodel(const string objfile)
{
    LoadObj(shapes, materials, objfile.c_str());

    r = abs(mesh_bound_x_max);
    if (r < abs(mesh_bound_x_min))
        r = abs(mesh_bound_x_min);

    if (r < abs(mesh_bound_y_max))
        r = abs(mesh_bound_y_max);

    if (r < abs(mesh_bound_y_min))
        r = abs(mesh_bound_y_min);

    if (r < abs(mesh_bound_z_max))
        r = abs(mesh_bound_z_max);

    if (r < abs(mesh_bound_x_min))
        r = abs(mesh_bound_z_min);

    r *= (float)1.7f;
    max_r = r/2;

    center_x = 0;
    center_y = 0;
    center_z = 0;
}

static long FindLastChar(const char* pc, const char find)
{
    char* p = (char*)pc;
    long offset = -1;
    while (0 != *p)
    {
        if (find == *p) offset = p - pc;
        p++;
    }

    return offset;
}

static string GetDirectoryPart(const string& pathfilename)
{
    long offset = FindLastChar(pathfilename.c_str(), '\\');
    if (0 > offset)
    {
        return string("");
    }

    return string(pathfilename.substr(0, offset));
}

int main(int args, char* argv[])
{
    cout << endl << "TIN viewer" << endl;
    cout << "-------------------------------------------------------" << endl << endl;

    leftbotton = false;
    rightbotton = false;

    // opengl框架初始化 ------------------------------
    glutInit(&args, argv);
    glutInitWindowPosition(200, 200);
    glutInitWindowSize(800, 600);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutCreateWindow("TIN viewer");
    glutIdleFunc(glidle);
    glutDisplayFunc(renderscene);
    glutReshapeFunc(reshape);
    glutMouseFunc(Mouse);
    glutMotionFunc(onMouseDrag);
    glutKeyboardFunc(keyboard);
    // opengl框架初始化 ------------------------------

    // initialise GLEW
    glewExperimental = GL_TRUE; //stops glew crashing on VS2013 :-/
    if (glewInit() != GLEW_OK)
    {
        throw runtime_error("glewInit failed");
    }

    // print out some info about the graphics drivers
    cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
//    cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
//    cout << "Vendor: " << glGetString(GL_VENDOR) << endl;
//    cout << "Renderer: " << glGetString(GL_RENDERER) << endl;
    string modelfile;
    if (args == 1)
    {
        // search the first .obj in the same directory
        WIN32_FIND_DATA findfiledata;
        string findkey = GetDirectoryPart(argv[0]) + "\\*.obj";
        HANDLE h = ::FindFirstFile(findkey.c_str(), &findfiledata);
        if (h == INVALID_HANDLE_VALUE)
        {
            cout << ".obj file not find" << endl;
            return 222;
        }

        modelfile = findfiledata.cFileName;
    }
    else
    {
        modelfile = argv[1];
    }

    //加载模型
    cout << "model file: " << modelfile << endl;
    cout << "loading model(s)..." << endl;
    loadmodel(modelfile);
    cout << "shapes count: " << shapes.size() << endl;
    cout << "materials count: " << materials.size() << endl;
    cout << "x_max: " << mesh_bound_x_max << endl;
    cout << "x_min: " << mesh_bound_x_min << endl;
    cout << "y_max: " << mesh_bound_y_max << endl;
    cout << "y_min: " << mesh_bound_y_min << endl;
    cout << "z_max: " << mesh_bound_z_max << endl;
    cout << "z_min: " << mesh_bound_z_min << endl;

    init();

    glutMainLoop();
    return 0;
}
