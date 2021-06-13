#if ENGINE >= Engine_G2

namespace NAMESPACE
{
	void RemoveLight(zCVobLight* light)
	{
		if (light->lightData.range <= 0)
			return;

		if (light->lightData.isStatic)
			return;

		const zVEC3 center = light->GetPositionWorld();
		const zVEC3 range = light->lightData.range;
		
		zTBBox3D bbox;
		bbox.mins = center - range;
		bbox.maxs = center + range;

		zCPolygon** polygons;
		int count;

		light->homeWorld->bspTree.bspRoot->CollectPolysInBBox3D(bbox, polygons, count);

		for (int i = 0; i < count; i++)
			polygons[i]->flags.mustRelight = true;
	}

	void __fastcall Hook_zCVob_RemoveWorldDependencies(zCVob*, void*, int);
	Hook<void(__thiscall*)(zCVob*, int), ActiveOption<bool>> Ivk_zCVob_RemoveWorldDependencies(ZENFOR(0x005D6790, 0x005F5CD0, 0x005FAF70, 0x00601DA0), &Hook_zCVob_RemoveWorldDependencies, HookMode::Patch, Options::RemoveLightFix);
	void __fastcall Hook_zCVob_RemoveWorldDependencies(zCVob* _this, void* vtable, int a0)
	{
		if (_this->homeWorld && _this->type == zVOB_TYPE_LIGHT && zCCamera::activeCam)
			RemoveLight(static_cast<zCVobLight*>(_this));

		Ivk_zCVob_RemoveWorldDependencies(_this, a0);
	}
}

#endif