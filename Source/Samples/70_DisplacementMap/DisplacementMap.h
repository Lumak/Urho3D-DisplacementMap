//
// Copyright (c) 2008-2016 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once
#include <Urho3D/Scene/Component.h>

namespace Urho3D
{
class Node;
class Terrain;
class Image;
class Texture2D;
}
using namespace Urho3D;

//=============================================================================
//=============================================================================
class DisplacementMap : public Component
{
    URHO3D_OBJECT(DisplacementMap, Component);

public:
    DisplacementMap(Context *context); 
    virtual ~DisplacementMap() {}

    static void RegisterObject(Context* context);

    void SetTerrain(Node *terrNode);
    void DisplaceBoundingBox(const Matrix3x4& worldMatrix, const BoundingBox &charBbox);
    void ApplyMapUpdate();

protected:
    void CalBoundingBox();

protected:
    WeakPtr<Node>       terrNode_;
    WeakPtr<Terrain>    terrain_;

    int                 patchSize_;
    Vector3             spacing_;
    IntVector2          numPatches_;
    IntVector2          numVertices_;
    Vector2             patchWorldSize_;
    Vector2             patchWorldOrigin_;

    BoundingBox         terrainBoundingBox_;

    // displacement
    bool                hasValidDataRange_;
    bool                mapUpdated_;
    float               minHeight_;
    float               maxHeight_;
    float               colorRange_;
    float               heightRange_;
    float               xSubdivision_;
    float               ySubdivision_;

};


