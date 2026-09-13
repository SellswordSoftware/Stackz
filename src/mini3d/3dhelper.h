// Helper functions similar to the LUA Glue

#ifndef _3D_HELPER_H
#define _3D_HELPER_H

#include <math.h>
#include "mini3d.h"
#include "3dmath.h"
#include "shape.h"
#include "scene.h"

#define NUMCUBOIDFACES 6

enum CuboidFace {
    front,
    back,
    top,
    bottom,
    left,
    right,
};

Scene3D *scene_new(void);

void scene_setCameraOrigin(Scene3D *scene, float x, float y, float z);

void scene_setCameraUp(Scene3D *scene, float x, float y, float z, float ux, float uy, float uz);

void scene_setLight(Scene3D *scene, float x, float y, float z);

Shape3D *shape_new(void);

Shape3D *shape_new_cuboid(float width, float height, float depth, float colorBias);

void node_scaleBy(Scene3DNode *node, float sx, float sy, float sz);

void node_resetTranform(Scene3DNode* node);

void matrix_scaleByAndAddTranslation(Scene3DNode* node, float sx, float sy, float sz, float x, float y, float z);

Point3D *point_new(float x, float y, float z);

void shape_addFace(Shape3D *shape, Point3D *a, Point3D *b, Point3D *c, Point3D *d, float colorBias);

Matrix3D *matrix_new(void);

Matrix3D *matrix_newRotation(float angle, float x, float y, float z);

void *matrix_updateRotation(Matrix3D *p, float angle, float x, float y, float z);

Matrix3D matrix_addTranslation(float x, float y, float z);

Matrix3D matrix_addIdentityTranslation(float x, float y, float z);

void node_addTransform(Matrix3D *matrix);

#endif