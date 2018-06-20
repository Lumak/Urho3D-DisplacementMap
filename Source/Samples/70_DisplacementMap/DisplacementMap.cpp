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

#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/Terrain.h>
#include <Urho3D/Resource/Image.h>
#include <Urho3D/IO/Log.h>

#include "DisplacementMap.h"

#include <Urho3D/DebugNew.h>
//=============================================================================
//=============================================================================
DisplacementMap::DisplacementMap(Context *context) 
    : Component(context)
    , mapUpdated_(false)
    , heightRange_(0.0f)
    , colorRange_(0.0f)
    , hasValidDataRange_(false)
{
}

void DisplacementMap::RegisterObject(Context* context)
{
    context->RegisterFactory<DisplacementMap>();
}

void DisplacementMap::SetTerrain(Node *terrNode)
{
    terrNode_ = terrNode;

    if (terrNode_)
    {
        terrain_ = terrNode_->GetComponent<Terrain>();

        if (terrain_)
        {
            patchSize_   = terrain_->GetPatchSize();
            spacing_     = terrain_->GetSpacing();
            numPatches_ = IntVector2( (terrain_->GetHeightMap()->GetWidth() - 1) / patchSize_, (terrain_->GetHeightMap()->GetHeight() - 1) / patchSize_ );
            numVertices_ = IntVector2( numPatches_.x_ * patchSize_ + 1, numPatches_.y_ * patchSize_ + 1 );
            patchWorldSize_ = Vector2( spacing_.x_ * (float)patchSize_, spacing_.z_ * (float)patchSize_ );
            patchWorldOrigin_ = Vector2( -0.5f * (float)numPatches_.x_ * patchWorldSize_.x_, -0.5f * (float)numPatches_.y_ *patchWorldSize_.y_ );

            // calculate bounding box, height and color vars
            CalBoundingBox();
        }
    }
}

void DisplacementMap::CalBoundingBox()
{
    Image *heightMap = terrain_->GetHeightMap();

    if (heightMap)
    {
        // init
        float minHeightColor = 1.0f;
        float maxHeightColor = 0.0f;

        int width = heightMap->GetWidth();
        int height = heightMap->GetHeight();

        for (int x = 0; x < width; ++x)
        {
            for (int y = 0; y < height; ++y)
            {
                // merge bbox
                Vector3 pos = terrain_->HeightMapToWorld(IntVector2(x, y));
                terrainBoundingBox_.Merge(pos);

                Color col = heightMap->GetPixel(x, y);
                if (col.r_ < minHeightColor)
                {
                    minHeightColor = col.r_;
                    minHeight_ = pos.y_;
                }
                if (col.r_ > maxHeightColor)
                {
                    maxHeightColor = col.r_;
                    maxHeight_ = pos.y_;
                }
            }
        }

        // data range
        colorRange_  = maxHeightColor - minHeightColor;
        heightRange_ = maxHeight_ - minHeight_;
        hasValidDataRange_ = !Equals(heightRange_, 0.0f) && !Equals(colorRange_, 0.0f);

        // subdivision
        xSubdivision_ = patchWorldSize_.x_ / (float)width;
        ySubdivision_ = patchWorldSize_.y_ / (float)height;
    }
}

void DisplacementMap::DisplaceBoundingBox(const Matrix3x4& worldMatrix, const BoundingBox &bBox)
{
    if (!hasValidDataRange_)
    {
        return;
    }

    BoundingBox worldBbox = bBox.Transformed(worldMatrix);

    if (terrainBoundingBox_.IsInside(worldBbox) == INSIDE)
    {
        Vector3 bbPos, scale;
        Quaternion qrot;

        worldMatrix.Decompose(bbPos, qrot, scale);

        if (bbPos.y_ > minHeight_ && bbPos.y_ < maxHeight_)
        {
            Image *heightMap = terrain_->GetHeightMap();

            float xmin = worldBbox.min_.x_;
            float xmax = worldBbox.max_.x_;
            if (worldBbox.min_.x_ > worldBbox.max_.x_)
            {
                xmin = worldBbox.max_.x_;
                xmax = worldBbox.min_.x_;
            }

            float zmin = worldBbox.min_.z_;
            float zmax = worldBbox.max_.z_;
            if (worldBbox.min_.z_ > worldBbox.max_.z_)
            {
                zmin = worldBbox.max_.z_;
                zmax = worldBbox.min_.z_;
            }

            for ( float x = xmin; x < xmax; x += xSubdivision_ )
            {
                for ( float z = zmin; z < zmax; z += ySubdivision_ )
                {
                    IntVector2 pixPos = terrain_->WorldToHeightMap(Vector3(x, 0.0f, z));
                    Color col = heightMap->GetPixel(pixPos.x_, pixPos.y_);

                    float deltaH = 1.0f - (maxHeight_ - bbPos.y_)/heightRange_;
                    float deltaC = deltaH * colorRange_;
                    if (deltaC - col.r_ < -M_EPSILON)
                    {
                        heightMap->SetPixel(pixPos.x_, pixPos.y_, Color(deltaC, deltaC, deltaC));
                        mapUpdated_ = true;
                    }
                }
            }
        }
    }
}

void DisplacementMap::ApplyMapUpdate()
{
    if (mapUpdated_)
    {
        terrain_->ApplyHeightMap();

        mapUpdated_ = false;
    }
}



