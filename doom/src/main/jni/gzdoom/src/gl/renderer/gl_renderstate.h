#ifndef __GL_RENDERSTATE_H
#define __GL_RENDERSTATE_H

#include <string.h>
#include "gl/system/gl_interface.h"
#include "gl/data/gl_data.h"
#include "gl/data/gl_matrix.h"
#include "gl/textures/gl_material.h"
#include "c_cvars.h"
#include "r_defs.h"
#include "r_data/r_translate.h"

class FVertexBuffer;
class FShader;
extern TArray<VSMatrix> gl_MatrixStack;

EXTERN_CVAR(Bool, gl_direct_state_change)

struct FStateVec4
{
	float vec[4];

	void Set(float r, float g, float b, float a)
	{
		vec[0] = r;
		vec[1] = g;
		vec[2] = b;
		vec[3] = a;
	}
};


enum EEffect
{
	EFF_NONE=-1,
	EFF_FOGBOUNDARY,
	EFF_SPHEREMAP,
	EFF_BURN,
	EFF_STENCIL,

	MAX_EFFECTS
};

class FRenderState
{
	bool mTextureEnabled;
	bool mFogEnabled;
	bool mGlowEnabled;
	bool mBrightmapEnabled;
	int mLightIndex;
	int mSpecialEffect;
	int mTextureMode;
	int mDesaturation;
	int mSoftLight;
	float mLightParms[4];
	int mSrcBlend, mDstBlend;
	float mAlphaThreshold;
	int mBlendEquation;
	bool mAlphaTest;
	bool m2D;
	bool mModelMatrixEnabled;
	bool mTextureMatrixEnabled;
	float mInterpolationFactor;
	float mClipHeightTop, mClipHeightBottom;
	float mShaderTimer;

	FVertexBuffer *mVertexBuffer, *mCurrentVertexBuffer;
	FStateVec4 mColor;
	FStateVec4 mCameraPos;
	FStateVec4 mGlowTop, mGlowBottom;
	FStateVec4 mGlowTopPlane, mGlowBottomPlane;
	PalEntry mFogColor;
	PalEntry mObjectColor;
	FStateVec4 mDynColor;

	int mEffectState;
	int mColormapState;

	float stAlphaThreshold;
	int stSrcBlend, stDstBlend;
	bool stAlphaTest;
	int stBlendEquation;

	FShader *activeShader;

	bool ApplyShader();

public:

	VSMatrix mProjectionMatrix;
	VSMatrix mViewMatrix;
	VSMatrix mModelMatrix;
	VSMatrix mTextureMatrix;

	FRenderState()
	{
		Reset();
	}

	void Reset();

	void SetMaterial(FMaterial *mat, int clampmode, int translation, int overrideshader, bool alphatexture)
	{
		// textures without their own palette are a special case for use as an alpha texture:
		// They use the color index directly as an alpha value instead of using the palette's red.
		// To handle this case, we need to set a special translation for such textures.
		if (alphatexture)
		{
			if (mat->tex->UseBasePalette()) translation = TRANSLATION(TRANSLATION_Standard, 8);
		}
		mEffectState = overrideshader >= 0? overrideshader : mat->mShaderIndex;
		mShaderTimer = mat->tex->gl_info.shaderspeed;
		mat->Bind(clampmode, translation);
	}

	void Apply();
	void ApplyMatrices();
	void ApplyLightIndex(int index);

	void SetVertexBuffer(FVertexBuffer *vb)
	{
		mVertexBuffer = vb;
	}

	void ResetVertexBuffer()
	{
		// forces rebinding with the next 'apply' call.
		mCurrentVertexBuffer = NULL;
	}

	void SetClipHeightTop(float clip)
	{
		mClipHeightTop = clip;
	}

	float GetClipHeightTop()
	{
		return mClipHeightTop;
	}

	void SetClipHeightBottom(float clip)
	{
		mClipHeightBottom = clip;
	}

	float GetClipHeightBottom()
	{
		return mClipHeightBottom;
	}

	void SetColor(float r, float g, float b, float a = 1.f, int desat = 0)
	{
		mColor.Set(r, g, b, a);
		mDesaturation = desat;
	}

	void SetColor(PalEntry pe, int desat = 0)
	{
		mColor.Set(pe.r/255.f, pe.g/255.f, pe.b/255.f, pe.a/255.f);
		mDesaturation = desat;
	}

	void SetColorAlpha(PalEntry pe, float alpha = 1.f, int desat = 0)
	{
		mColor.Set(pe.r/255.f, pe.g/255.f, pe.b/255.f, alpha);
		mDesaturation = desat;
	}

	void ResetColor()
	{
		mColor.Set(1,1,1,1);
		mDesaturation = 0;
	}

	void SetTextureMode(int mode)
	{
		mTextureMode = mode;
	}

	void EnableTexture(bool on)
	{
		mTextureEnabled = on;
	}

	void EnableFog(bool on)
	{
		mFogEnabled = on;
	}

	void SetEffect(int eff)
	{
		mSpecialEffect = eff;
	}

	void EnableGlow(bool on)
	{
		mGlowEnabled = on;
	}

	void SetLightIndex(int n)
	{
		mLightIndex = n;
	}

	void EnableBrightmap(bool on)
	{
		mBrightmapEnabled = on;
	}

	void EnableModelMatrix(bool on)
	{
		mModelMatrixEnabled = on;
	}

	void EnableTextureMatrix(bool on)
	{
		mTextureMatrixEnabled = on;
	}

	void SetCameraPos(float x, float y, float z)
	{
		mCameraPos.Set(x, z, y, 0);
	}

	void SetGlowParams(float *t, float *b)
	{
		mGlowTop.Set(t[0], t[1], t[2], t[3]);
		mGlowBottom.Set(b[0], b[1], b[2], b[3]);
	}

	void SetSoftLightLevel(int level)
	{
		if (glset.lightmode == 8) mLightParms[3] = level / 255.f;
		else mLightParms[3] = -1.f;
	}

	void SetGlowPlanes(const secplane_t &top, const secplane_t &bottom)
	{
		mGlowTopPlane.Set(FIXED2FLOAT(top.a), FIXED2FLOAT(top.b), FIXED2FLOAT(top.ic), FIXED2FLOAT(top.d));
		mGlowBottomPlane.Set(FIXED2FLOAT(bottom.a), FIXED2FLOAT(bottom.b), FIXED2FLOAT(bottom.ic), FIXED2FLOAT(bottom.d));
	}

	void SetDynLight(float r, float g, float b)
	{
		mDynColor.Set(r, g, b, 0);
	}

	void SetObjectColor(PalEntry pe)
	{
		mObjectColor = pe;
	}

	void SetFog(PalEntry c, float d)
	{
		const float LOG2E = 1.442692f;	// = 1/log(2)
		mFogColor = c;
		if (d >= 0.0f) mLightParms[2] = d * (-LOG2E / 64000.f);
	}

	void SetLightParms(float f, float d)
	{
		mLightParms[1] = f;
		mLightParms[0] = d;
	}

	void SetFixedColormap(int cm)
	{
		mColormapState = cm;
	}

	PalEntry GetFogColor() const
	{
		return mFogColor;
	}

	void BlendFunc(int src, int dst)
	{
		if (!gl_direct_state_change)
		{
			mSrcBlend = src;
			mDstBlend = dst;
		}
		else
		{
			glBlendFunc(src, dst);
		}
	}

	void AlphaFunc(int func, float thresh)
	{
		if (func == GL_GREATER) mAlphaThreshold = thresh;
		else mAlphaThreshold = thresh - 0.001f;
	}

	void BlendEquation(int eq)
	{
		if (!gl_direct_state_change)
		{
			mBlendEquation = eq;
		}
		else
		{
			glBlendEquation(eq);
		}
	}

	void Set2DMode(bool on)
	{
		m2D = on;
	}

	void SetInterpolationFactor(float fac)
	{
		mInterpolationFactor = fac;
	}
};

extern FRenderState gl_RenderState;

#endif
