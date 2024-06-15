#ifndef __GL_SAMPLERS_H
#define __GL_SAMPLERS_H

#include "gl_hwtexture.h"

class FSamplerManager
{
	// We need 6 different samplers: 4 for the different clamping modes,
	// one for 2D-textures and one for voxel textures
	unsigned int mSamplers[6];
	unsigned int mLastBound[FHardwareTexture::MAX_TEXTURES];

	void UnbindAll();

public:

	FSamplerManager();
	~FSamplerManager();

	void Bind(int texunit, int num);
	void SetTextureFilterMode();


};


#endif

