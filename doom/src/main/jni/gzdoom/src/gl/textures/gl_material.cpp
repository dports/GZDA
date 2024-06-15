/*
** gl_material.cpp
** 
**---------------------------------------------------------------------------
** Copyright 2004-2009 Christoph Oelckers
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
** 4. When not used as part of GZDoom or a GZDoom derivative, this code will be
**    covered by the terms of the GNU Lesser General Public License as published
**    by the Free Software Foundation; either version 2.1 of the License, or (at
**    your option) any later version.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
**
*/

#include "gl/system/gl_system.h"
#include "w_wad.h"
#include "m_png.h"
#include "sbar.h"
#include "gi.h"
#include "cmdlib.h"
#include "c_dispatch.h"
#include "stats.h"
#include "r_utility.h"
#include "templates.h"
#include "sc_man.h"
#include "colormatcher.h"

//#include "gl/gl_intern.h"

#include "gl/system/gl_interface.h"
#include "gl/system/gl_framebuffer.h"
#include "gl/renderer/gl_lightdata.h"
#include "gl/renderer/gl_renderer.h"
#include "gl/data/gl_data.h"
#include "gl/textures/gl_texture.h"
#include "gl/textures/gl_translate.h"
#include "gl/textures/gl_bitmap.h"
#include "gl/textures/gl_material.h"
#include "gl/textures/gl_samplers.h"
#include "gl/shaders/gl_shader.h"

EXTERN_CVAR(Bool, gl_render_precise)
EXTERN_CVAR(Int, gl_lightmode)
EXTERN_CVAR(Bool, gl_precache)
EXTERN_CVAR(Bool, gl_texture_usehires)

//===========================================================================
//
// The GL texture maintenance class
//
//===========================================================================

//===========================================================================
//
// Constructor
//
//===========================================================================
FGLTexture::FGLTexture(FTexture * tx, bool expandpatches)
{
	assert(tx->gl_info.SystemTexture[expandpatches] == NULL);
	tex = tx;

	mHwTexture = NULL;
	HiresLump = -1;
	hirestexture = NULL;
	bHasColorkey = false;
	bIsTransparent = -1;
	bExpand = expandpatches;
	tex->gl_info.SystemTexture[expandpatches] = this;
}

//===========================================================================
//
// Destructor
//
//===========================================================================

FGLTexture::~FGLTexture()
{
	Clean(true);
	if (hirestexture) delete hirestexture;
}

//==========================================================================
//
// Checks for the presence of a hires texture replacement and loads it
//
//==========================================================================
unsigned char *FGLTexture::LoadHiresTexture(FTexture *tex, int *width, int *height)
{
	if (HiresLump==-1) 
	{
		bHasColorkey = false;
		HiresLump = CheckDDPK3(tex);
		if (HiresLump < 0) HiresLump = CheckExternalFile(tex, bHasColorkey);

		if (HiresLump >=0) 
		{
			hirestexture = FTexture::CreateTexture(HiresLump, FTexture::TEX_Any);
		}
	}
	if (hirestexture != NULL)
	{
		int w=hirestexture->GetWidth();
		int h=hirestexture->GetHeight();

		unsigned char * buffer=new unsigned char[w*(h+1)*4];
		memset(buffer, 0, w * (h+1) * 4);

		FGLBitmap bmp(buffer, w*4, w, h);
		
		int trans = hirestexture->CopyTrueColorPixels(&bmp, 0, 0);
		hirestexture->CheckTrans(buffer, w*h, trans);
		bIsTransparent = hirestexture->gl_info.mIsTransparent;

		if (bHasColorkey)
		{
			// This is a crappy Doomsday color keyed image
			// We have to remove the key manually. :(
			DWORD * dwdata=(DWORD*)buffer;
			for (int i=(w*h);i>0;i--)
			{
				if (dwdata[i]==0xffffff00 || dwdata[i]==0xffff00ff) dwdata[i]=0;
			}
		}
		*width = w;
		*height = h;
		return buffer;
	}
	return NULL;
}

//===========================================================================
// 
//	Deletes all allocated resources
//
//===========================================================================

void FGLTexture::Clean(bool all)
{
	if (mHwTexture) 
	{
		if (!all) mHwTexture->Clean(false);
		else
		{
			delete mHwTexture;
			mHwTexture = NULL;
		}
	}
}


//===========================================================================
// 
//	Initializes the buffer for the texture data
//
//===========================================================================

unsigned char * FGLTexture::CreateTexBuffer(int translation, int & w, int & h, FTexture *hirescheck)
{
	unsigned char * buffer;
	int W, H;


	// Textures that are already scaled in the texture lump will not get replaced
	// by hires textures
	if (gl_texture_usehires && hirescheck != NULL)
	{
		buffer = LoadHiresTexture (hirescheck, &w, &h);
		if (buffer)
		{
			return buffer;
		}
	}

	W = w = tex->GetWidth() + bExpand*2;
	H = h = tex->GetHeight() + bExpand*2;


	buffer=new unsigned char[W*(H+1)*4];
	memset(buffer, 0, W * (H+1) * 4);

	FGLBitmap bmp(buffer, W*4, W, H);
	bmp.SetTranslationInfo(translation);

	if (tex->bComplex)
	{
		FBitmap imgCreate;

		// The texture contains special processing so it must be composited using the
		// base bitmap class and then be converted as a whole.
		if (imgCreate.Create(W, H))
		{
			memset(imgCreate.GetPixels(), 0, W * H * 4);
			int trans = tex->CopyTrueColorPixels(&imgCreate, bExpand, bExpand);
			bmp.CopyPixelDataRGB(0, 0, imgCreate.GetPixels(), W, H, 4, W * 4, 0, CF_BGRA);
			tex->CheckTrans(buffer, W*H, trans);
			bIsTransparent = tex->gl_info.mIsTransparent;
		}
	}
	else if (translation<=0)
	{
		int trans = tex->CopyTrueColorPixels(&bmp, bExpand, bExpand);
		tex->CheckTrans(buffer, W*H, trans);
		bIsTransparent = tex->gl_info.mIsTransparent;
	}
	else
	{
		// When using translations everything must be mapped to the base palette.
		// Since FTexture's method is doing exactly that by calling GetPixels let's use that here
		// to do all the dirty work for us. ;)
		tex->FTexture::CopyTrueColorPixels(&bmp, bExpand, bExpand);
		bIsTransparent = 0;
	}

	// [BB] The hqnx upsampling (not the scaleN one) destroys partial transparency, don't upsamle textures using it.
	// [BB] Potentially upsample the buffer.
	return gl_CreateUpsampledTextureBuffer ( tex, buffer, W, H, w, h, !!bIsTransparent);
}


//===========================================================================
// 
//	Create hardware texture for world use
//
//===========================================================================

FHardwareTexture *FGLTexture::CreateHwTexture()
{
	if (tex->UseType==FTexture::TEX_Null) return NULL;		// Cannot register a NULL texture
	if (mHwTexture == NULL)
	{
		mHwTexture = new FHardwareTexture(tex->GetWidth() + bExpand*2, tex->GetHeight() + bExpand*2, tex->gl_info.bNoCompress);
	}
	return mHwTexture; 
}

//===========================================================================
// 
//	Binds a texture to the renderer
//
//===========================================================================

const FHardwareTexture *FGLTexture::Bind(int texunit, int clampmode, int translation, FTexture *hirescheck)
{
	int usebright = false;

	if (translation <= 0) translation = -translation;
	else translation = GLTranslationPalette::GetInternalTranslation(translation);

	bool needmipmap = (clampmode <= CLAMP_XY);

	FHardwareTexture *hwtex = CreateHwTexture();

	if (hwtex)
	{
		// Texture has become invalid
		if ((!tex->bHasCanvas && !tex->bWarped) && tex->CheckModified())
		{
			Clean(true);
			hwtex = CreateHwTexture();
		}

		// Bind it to the system.
		if (!hwtex->Bind(texunit, translation, needmipmap))
		{
			
			int w=0, h=0;

			// Create this texture
			unsigned char * buffer = NULL;
			
			if (!tex->bHasCanvas)
			{
				buffer = CreateTexBuffer(translation, w, h, hirescheck);
				tex->ProcessData(buffer, w, h, false);
			}
			if (!hwtex->CreateTexture(buffer, w, h, texunit, needmipmap, translation)) 
			{
				// could not create texture
				delete[] buffer;
				return NULL;
			}
			delete[] buffer;
		}

		if (tex->bHasCanvas) static_cast<FCanvasTexture*>(tex)->NeedUpdate();
		GLRenderer->mSamplerManager->Bind(texunit, clampmode);
		return hwtex; 
	}
	return NULL;
}

//===========================================================================
//
//
//
//===========================================================================

fixed_t FTexCoordInfo::RowOffset(fixed_t rowoffset) const
{
	if (mTempScaleY == FRACUNIT)
	{
		if (mScaleY==FRACUNIT || mWorldPanning) return rowoffset;
		else return FixedDiv(rowoffset, mScaleY);
	}
	else
	{
		if (mWorldPanning) return FixedDiv(rowoffset, mTempScaleY);
		else return FixedDiv(rowoffset, mScaleY);
	}
}

//===========================================================================
//
//
//
//===========================================================================

fixed_t FTexCoordInfo::TextureOffset(fixed_t textureoffset) const
{
	if (mTempScaleX == FRACUNIT)
	{
		if (mScaleX==FRACUNIT || mWorldPanning) return textureoffset;
		else return FixedDiv(textureoffset, mScaleX);
	}
	else
	{
		if (mWorldPanning) return FixedDiv(textureoffset, mTempScaleX);
		else return FixedDiv(textureoffset, mScaleX);
	}
}

//===========================================================================
//
// Returns the size for which texture offset coordinates are used.
//
//===========================================================================

fixed_t FTexCoordInfo::TextureAdjustWidth() const
{
	if (mWorldPanning) 
	{
		if (mTempScaleX == FRACUNIT) return mRenderWidth;
		else return FixedDiv(mWidth, mTempScaleX);
	}
	else return mWidth;
}



//===========================================================================
//
//
//
//===========================================================================
FGLTexture * FMaterial::ValidateSysTexture(FTexture * tex, bool expand)
{
	if (tex	&& tex->UseType!=FTexture::TEX_Null)
	{
		FGLTexture *gltex = tex->gl_info.SystemTexture[expand];
		if (gltex == NULL) 
		{
			gltex = new FGLTexture(tex, expand);
		}
		return gltex;
	}
	return NULL;
}

//===========================================================================
//
// Constructor
//
//===========================================================================
TArray<FMaterial *> FMaterial::mMaterials;
int FMaterial::mMaxBound;

FMaterial::FMaterial(FTexture * tx, bool expanded)
{
	mShaderIndex = 0;
	tex = tx;

	// TODO: apply custom shader object here
	/* if (tx->CustomShaderDefinition)
	{
	}
	else
	*/
	if (tx->bWarped)
	{
		mShaderIndex = tx->bWarped;
		expanded = false;
		tx->gl_info.shaderspeed = static_cast<FWarpTexture*>(tx)->GetSpeed();
	}
	else if (tx->bHasCanvas)
	{
		expanded = false;
	}
	else
	{
		if (tx->gl_info.shaderindex >= FIRST_USER_SHADER)
		{
			mShaderIndex = tx->gl_info.shaderindex;
			expanded = false;
		}
		else
		{
			tx->CreateDefaultBrightmap();
			if (tx->gl_info.Brightmap != NULL)
			{
				ValidateSysTexture(tx->gl_info.Brightmap, expanded);
				FTextureLayer layer = {tx->gl_info.Brightmap, false};
				mTextureLayers.Push(layer);
				mShaderIndex = 3;
			}
		}
	}
	assert(tx->gl_info.Material[expanded] == NULL);
	mBaseLayer = ValidateSysTexture(tx, true);


	mWidth = tx->GetWidth();
	mHeight = tx->GetHeight();
	mLeftOffset = tx->LeftOffset;
	mTopOffset = tx->TopOffset;
	mRenderWidth = tx->GetScaledWidth();
	mRenderHeight = tx->GetScaledHeight();
	mSpriteU[0] = mSpriteV[0] = 0.f;
	mSpriteU[1] = mSpriteV[1] = 1.f;

	FTexture *basetex = tx->GetRedirect(false);
	mBaseLayer = ValidateSysTexture(basetex, expanded);

	// mSpriteRect is for positioning the sprite in the scene.
	mSpriteRect.left = -mLeftOffset / FIXED2FLOAT(tx->xScale);
	mSpriteRect.top = -mTopOffset / FIXED2FLOAT(tx->yScale);
	mSpriteRect.width = mWidth / FIXED2FLOAT(tx->xScale);
	mSpriteRect.height = mHeight / FIXED2FLOAT(tx->yScale);

	if (expanded)
	{
		// a little adjustment to make sprites look better with texture filtering:
		// create a 1 pixel wide empty frame around them.
		mWidth+=2;
		mHeight+=2;
		mLeftOffset+=1;
		mTopOffset+=1;
		mRenderWidth = mRenderWidth * mWidth / (mWidth-2);
		mRenderHeight = mRenderHeight * mHeight / (mHeight-2);

		int trim[4];

		if (TrimBorders(trim))
		{
			mSpriteRect.left = -(mLeftOffset - trim[0]) / FIXED2FLOAT(tx->xScale);
			mSpriteRect.top = -(mTopOffset - trim[1]) / FIXED2FLOAT(tx->yScale);
			mSpriteRect.width = (trim[2] + 2) / FIXED2FLOAT(tx->xScale);
			mSpriteRect.height = (trim[3] + 2) / FIXED2FLOAT(tx->yScale);

			mSpriteU[0] = trim[0] / (float)mWidth;
			mSpriteV[0] = trim[1] / (float)mHeight;
			mSpriteU[1] *= (trim[0]+trim[2]+2) / (float)mWidth; 
			mSpriteV[1] *= (trim[1]+trim[3]+2) / (float)mHeight; 
		}
	}

	mTextureLayers.ShrinkToFit();
	mMaxBound = -1;
	mMaterials.Push(this);
	tx->gl_info.Material[expanded] = this;
	if (tx->bHasCanvas) tx->gl_info.mIsTransparent = 0;
	mExpanded = expanded;
}

//===========================================================================
//
// Destructor
//
//===========================================================================

FMaterial::~FMaterial()
{
	for(unsigned i=0;i<mMaterials.Size();i++)
	{
		if (mMaterials[i]==this) 
		{
			mMaterials.Delete(i);
			break;
		}
	}

}


//===========================================================================
// 
//	Finds gaps in the texture which can be skipped by the renderer
//  This was mainly added to speed up one area in E4M6 of 007LTSD
//
//===========================================================================

bool FMaterial::TrimBorders(int *rect)
{
	PalEntry col;
	int w;
	int h;

	unsigned char *buffer = CreateTexBuffer(0, w, h);

	if (buffer == NULL) 
	{
		return false;
	}
	if (w != mWidth || h != mHeight)
	{
		// external Hires replacements cannot be trimmed.
		delete [] buffer;
		return false;
	}

	int size = w*h;

	int first, last;

	for(first = 0; first < size; first++)
	{
		if (buffer[first*4+3] != 0) break;
	}
	if (first >= size)
	{
		rect[0] = 0;
		rect[1] = 0;
		rect[2] = 1;
		rect[3] = 1;
		delete [] buffer;
		return true;
	}

	for(last = size-1; last >= first; last--)
	{
		if (buffer[last*4+3] != 0) break;
	}

	rect[1] = first / w;
	rect[3] = 1 + last/w - rect[1];

	rect[0] = 0;
	rect[2] = w;

	unsigned char *bufferoff = buffer + (rect[1] * w * 4);
	h = rect[3];

	for(int x = 0; x < w; x++)
	{
		for(int y = 0; y < h; y++)
		{
			if (bufferoff[(x+y*w)*4+3] != 0) goto outl;
		}
		rect[0]++;
	}
outl:
	rect[2] -= rect[0];

	for(int x = w-1; rect[2] > 1; x--)
	{
		for(int y = 0; y < h; y++)
		{
			if (bufferoff[(x+y*w)*4+3] != 0) 
			{
				delete [] buffer;
				return true;
			}
		}
		rect[2]--;
	}
	delete [] buffer;
	return true;
}


//===========================================================================
// 
//	Binds a texture to the renderer
//
//===========================================================================

static FMaterial *last;
static int lastclamp;
static int lasttrans;


void FMaterial::Bind(int clampmode, int translation)
{
	// avoid rebinding the same texture multiple times.
	if (this == last && lastclamp == clampmode && translation == lasttrans) return;
	last = this;
	lastclamp = clampmode;
	lasttrans = translation;

	int usebright = false;
	int maxbound = 0;
	bool allowhires = tex->xScale == FRACUNIT && tex->yScale == FRACUNIT && clampmode <= CLAMP_XY && !mExpanded;

	if (tex->bHasCanvas) clampmode = CLAMP_CAMTEX;
	else if (tex->bWarped && clampmode <= CLAMP_XY) clampmode = CLAMP_NONE;

	const FHardwareTexture *gltexture = mBaseLayer->Bind(0, clampmode, translation, allowhires? tex:NULL);
	if (gltexture != NULL)
	{
		for(unsigned i=0;i<mTextureLayers.Size();i++)
		{
			FTexture *layer;
			if (mTextureLayers[i].animated)
			{
				FTextureID id = mTextureLayers[i].texture->id;
				layer = TexMan(id);
				ValidateSysTexture(layer, mExpanded);
			}
			else
			{
				layer = mTextureLayers[i].texture;
			}
			layer->gl_info.SystemTexture[mExpanded]->Bind(i+1, clampmode, 0, NULL);
			maxbound = i+1;
		}
	}
	// unbind everything from the last texture that's still active
	for(int i=maxbound+1; i<=mMaxBound;i++)
	{
		FHardwareTexture::Unbind(i);
		mMaxBound = maxbound;
	}
}


//===========================================================================
//
//
//
//===========================================================================
void FMaterial::Precache()
{
	Bind(0, 0);
}

//===========================================================================
//
// Retrieve texture coordinate info for per-wall scaling
//
//===========================================================================

void FMaterial::GetTexCoordInfo(FTexCoordInfo *tci, fixed_t x, fixed_t y) const
{
	if (x == FRACUNIT)
	{
		tci->mRenderWidth = mRenderWidth;
		tci->mScaleX = tex->xScale;
		tci->mTempScaleX = FRACUNIT;
	}
	else
	{
		fixed_t scale_x = FixedMul(x, tex->xScale);
		int foo = (mWidth << 17) / scale_x; 
		tci->mRenderWidth = (foo >> 1) + (foo & 1); 
		tci->mScaleX = scale_x;
		tci->mTempScaleX = x;
	}

	if (y == FRACUNIT)
	{
		tci->mRenderHeight = mRenderHeight;
		tci->mScaleY = tex->yScale;
		tci->mTempScaleY = FRACUNIT;
	}
	else
	{
		fixed_t scale_y = FixedMul(y, tex->yScale);
		int foo = (mHeight << 17) / scale_y; 
		tci->mRenderHeight = (foo >> 1) + (foo & 1); 
		tci->mScaleY = scale_y;
		tci->mTempScaleY = y;
	}
	if (tex->bHasCanvas) 
	{
		tci->mScaleY = -tci->mScaleY;
		tci->mRenderHeight = -tci->mRenderHeight;
	}
	tci->mWorldPanning = tex->bWorldPanning;
	tci->mWidth = mWidth;
}

//===========================================================================
//
//
//
//===========================================================================

int FMaterial::GetAreas(FloatRect **pAreas) const
{
	if (mShaderIndex == 0)	// texture splitting can only be done if there's no attached effects
	{
		FTexture *tex = mBaseLayer->tex;
		*pAreas = tex->gl_info.areas;
		return tex->gl_info.areacount;
	}
	else
	{
		return 0;
	}
}

//===========================================================================
//
//
//
//===========================================================================

void FMaterial::BindToFrameBuffer()
{
	if (mBaseLayer->mHwTexture == NULL)
	{
		// must create the hardware texture first
		mBaseLayer->Bind(0, 0, 0, NULL);
		FHardwareTexture::Unbind(0);
		ClearLastTexture();
	}
	mBaseLayer->mHwTexture->BindToFrameBuffer();
}

//==========================================================================
//
// Gets a texture from the texture manager and checks its validity for
// GL rendering. 
//
//==========================================================================

FMaterial * FMaterial::ValidateTexture(FTexture * tex, bool expand)
{
	if (tex	&& tex->UseType!=FTexture::TEX_Null)
	{
		FMaterial *gltex = tex->gl_info.Material[expand];
		if (gltex == NULL) 
		{
			gltex = new FMaterial(tex, expand);
		}
		return gltex;
	}
	return NULL;
}

FMaterial * FMaterial::ValidateTexture(FTextureID no, bool expand, bool translate)
{
	return ValidateTexture(translate? TexMan(no) : TexMan[no], expand);
}


//==========================================================================
//
// Flushes all hardware dependent data
//
//==========================================================================

void FMaterial::FlushAll()
{
	for(int i=mMaterials.Size()-1;i>=0;i--)
	{
		mMaterials[i]->Clean(true);
	}
	// This is for shader layers. All shader layers must be managed by the texture manager
	// so this will catch everything.
	for(int i=TexMan.NumTextures()-1;i>=0;i--)
	{
		for (int j = 0; j < 2; j++)
		{
			FGLTexture *gltex = TexMan.ByIndex(i)->gl_info.SystemTexture[j];
			if (gltex != NULL) gltex->Clean(true);
		}
	}
}

void FMaterial::ClearLastTexture()
{
	last = NULL;
}
