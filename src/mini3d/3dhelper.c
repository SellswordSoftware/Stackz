#include "3dhelper.h"

Scene3D *scene_new()
{
    Scene3D *scene = m3d_malloc(sizeof(Scene3D));
    Scene3D_init(scene);
    return scene;
}

static Point3D cameraOrigin = {.x = 0, .y = 0, .z = -1};
static Point3D cameraLookat = {.x = 0, .y = 0, .z = 0};
static float cameraScale = 1.0;
static Vector3D cameraUp = {.dx = 0, .dy = -1, .dz = 0};

void scene_setCameraOrigin(Scene3D *scene, float x, float y, float z)
{
    cameraOrigin.x = x;
    cameraOrigin.y = y;
    cameraOrigin.z = z;
    Scene3D_setCamera(scene, cameraOrigin, cameraLookat, cameraScale, cameraUp);
}

void scene_setCameraUp(Scene3D *scene, float x, float y, float z, float ux, float uy, float uz)
{
    cameraOrigin.x = x;
    cameraOrigin.y = y;
    cameraOrigin.z = z;
    cameraUp.dx = ux;
    cameraUp.dy = uy;
    cameraUp.dz = uz;
    Scene3D_setCamera(scene, cameraOrigin, cameraLookat, cameraScale, cameraUp);
}

void scene_setLight(Scene3D *scene, float x, float y, float z)
{
    Scene3D_setGlobalLight(scene, Vector3DMake(x, y, z));
}

Shape3D *shape_new()
{
    Shape3D *shape = m3d_malloc(sizeof(Shape3D));
    Shape3D_init(shape);
    Shape3D_retain(shape);
    return shape;
}

Shape3D *shape_new_cuboid(float width, float height, float depth, float colorBias)
{
    Shape3D *shape = shape_new();

    // front
    shape_addFace(shape,
                  point_new(-width, -height, depth),
                  point_new(width, -height, depth),
                  point_new(width, height, depth),
                  point_new(-width, height, depth),
                  colorBias);

    // back
    shape_addFace(shape,
                  point_new(-width, -height, -depth),
                  point_new(-width, height, -depth),
                  point_new(width, height, -depth),
                  point_new(width, -height, -depth),
                  colorBias);

    // top
    shape_addFace(shape,
                  point_new(width, height, depth),
                  point_new(width, height, -depth),
                  point_new(-width, height, -depth),
                  point_new(-width, height, depth),
                  colorBias + 0.5f);

    // bottom
    shape_addFace(shape,
                  point_new(-width, -height, depth),
                  point_new(-width, -height, -depth),
                  point_new(width, -height, -depth),
                  point_new(width, -height, depth),
                  colorBias);

    // left
    shape_addFace(shape,
                  point_new(-width, -height, depth),
                  point_new(-width, height, depth),
                  point_new(-width, height, -depth),
                  point_new(-width, -height, -depth),
                  colorBias);

    // right
    shape_addFace(shape,
                  point_new(width, -height, depth),
                  point_new(width, -height, -depth),
                  point_new(width, height, -depth),
                  point_new(width, height, depth),
                  colorBias);

    Shape3D_setClosed(shape, 1);

    return shape;
}

void node_scaleBy(Scene3DNode *node, float sx, float sy, float sz) {
	Matrix3D xform = node->transform;
	xform.isIdentity = 0;
	
	xform.m[0][0] *= sx;
	xform.m[1][0] *= sx;
	xform.m[2][0] *= sx;
	
	xform.m[0][1] *= sy;
	xform.m[1][1] *= sy;
	xform.m[2][1] *= sy;
	
	xform.m[0][2] *= sz;
	xform.m[1][2] *= sz;
	xform.m[2][2] *= sz;
	
	xform.dx *= sx;
	xform.dy *= sy;
	xform.dz *= sz;
	
	xform.inverting = (sx * sy * sz < 0);

	Scene3DNode_setTransform(node, &xform);
}

void node_resetTranform(Scene3DNode* node) {
    Matrix3D xform = identityMatrix;
    xform.isIdentity = 0;
    Scene3DNode_setTransform(node, &xform);
}

void matrix_scaleByAndAddTranslation(Scene3DNode* node, float sx, float sy, float sz, float x, float y, float z) {
    Matrix3D xform = node->transform;
    xform.isIdentity = 0;

    xform.m[0][0] *= sx;
    xform.m[1][0] *= sx;
    xform.m[2][0] *= sx;

    xform.m[0][1] *= sy;
    xform.m[1][1] *= sy;
    xform.m[2][1] *= sy;

    xform.m[0][2] *= sz;
    xform.m[1][2] *= sz;
    xform.m[2][2] *= sz;

    xform.dx = x;// * sx;
    xform.dy = y;// *sy;
    xform.dz = z;// *sz;

    xform.inverting = (sx * sy * sz < 0);
    //xform = Matrix3D_multiply(xform, Matrix3DMakeTranslate(x, y, z));
    //;//  matrix_addTranslation(x, y, z);

    Scene3DNode_setTransform(node, &xform);
}

Point3D *point_new(float x, float y, float z)
{
    Point3D *p = m3d_malloc(sizeof(Point3D));
    p->x = x;
    p->y = y;
    p->z = z;
    return p;
}

void shape_addFace(Shape3D *shape, Point3D *a, Point3D *b, Point3D *c, Point3D *d, float colorBias)
{
    Shape3D_addFace(shape, a, b, c, d, colorBias);
    return;
}

Matrix3D *matrix_new(void) {
    Matrix3D* p = m3d_malloc(sizeof(Matrix3D));
	
	p->isIdentity = 0;
    p->inverting = 0;
	p->m[0][0] = 1.f;
	p->m[0][1] = 0.f;
	p->m[0][2] = 0.f;
	p->m[1][0] = 0.f;
	p->m[1][1] = 1.f;
	p->m[1][2] = 0.f;
	p->m[2][0] = 0.f;
	p->m[2][1] = 0.f;
	p->m[2][2] = 1.f;
	p->dx = p->dy = p->dz = 0;
    
    return p;
}

Matrix3D *matrix_newRotation(float angle, float x, float y, float z)
{
    Matrix3D *p = m3d_malloc(sizeof(Matrix3D));

#undef M_PI
#define M_PI 3.14159265358979323846f

    float c = cosf(angle * M_PI / 180);
    float s = sinf(angle * M_PI / 180);

    float d = sqrtf(x * x + y * y + z * z);

    x /= d;
    y /= d;
    z /= d;

    p->isIdentity = 0;
    p->inverting = 0;

    p->m[0][0] = c + x * x * (1 - c);
    p->m[0][1] = x * y * (1 - c) - z * s;
    p->m[0][2] = x * z * (1 - c) + y * s;
    p->m[1][0] = y * x * (1 - c) + z * s;
    p->m[1][1] = c + y * y * (1 - c);
    p->m[1][2] = y * z * (1 - c) - x * s;
    p->m[2][0] = z * x * (1 - c) - y * s;
    p->m[2][1] = z * y * (1 - c) + x * s;
    p->m[2][2] = c + z * z * (1 - c);
    p->dx = p->dy = p->dz = 0;
    return p;
}

void *matrix_updateRotation(Matrix3D *p, float angle, float x, float y, float z)
{

#undef M_PI
#define M_PI 3.14159265358979323846f

    float c = cosf(angle * M_PI / 180);
    float s = sinf(angle * M_PI / 180);

    float d = sqrtf(x * x + y * y + z * z);

    x /= d;
    y /= d;
    z /= d;

    p->isIdentity = 0;
    p->inverting = 0;

    p->m[0][0] = c + x * x * (1 - c);
    p->m[0][1] = x * y * (1 - c) - z * s;
    p->m[0][2] = x * z * (1 - c) + y * s;
    p->m[1][0] = y * x * (1 - c) + z * s;
    p->m[1][1] = c + y * y * (1 - c);
    p->m[1][2] = y * z * (1 - c) - x * s;
    p->m[2][0] = z * x * (1 - c) - y * s;
    p->m[2][1] = z * y * (1 - c) + x * s;
    p->m[2][2] = c + z * z * (1 - c);
    p->dx = p->dy = p->dz = 0;
    return p;
}

Matrix3D matrix_addTranslation(float x, float y, float z) {
    Matrix3D xform = identityMatrix;
    //Matrix3D* l = matrix_new();
    xform.isIdentity = 0;
    xform.dx = x;
    xform.dy = y;
    xform.dz = z;
    return xform;
}

Matrix3D matrix_addIdentityTranslation(float x, float y, float z) {
    Matrix3D xform = identityMatrix;
    //Matrix3D* l = matrix_new();
    xform.isIdentity = 1;
    xform.dx = x;
    xform.dy = y;
    xform.dz = z;
    return xform;
}