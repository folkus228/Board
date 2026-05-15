#pragma once

#include "PipeLine.h"
#include "../object/Scene.h"

class Base3DPass : public RenderPass
{
protected:

	void Init(RenderContext& ctx) override
	{
		ctx.ApplyTestFiches();
		ctx.Enable3DFilter();
		//ctx.EnableBlend();
	}
public:

	void Execute(RenderContext& ctx, Scene* scene, Camera2D* camera) override
	{
		ctx.ClearColor(0.1f, 0.1f, 0.1f, 1.f);
		ctx.Clear();

		ctx.Render(*scene, *camera);
	}
	friend PipeLine;
};
