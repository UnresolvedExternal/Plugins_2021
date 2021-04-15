namespace NAMESPACE
{
	void __fastcall Hook_zCVob_RemoveWorldDependencies(zCVob*, void*, int);
	Hook<void(__thiscall*)(zCVob*, int), ActiveOption<bool>> Ivk_zCVob_RemoveWorldDependencies(ZENFOR(0x005D6790, 0x005F5CD0, 0x005FAF70, 0x00601DA0), &Hook_zCVob_RemoveWorldDependencies, HookMode::Patch, Options::RemoveLightFix);
	void __fastcall Hook_zCVob_RemoveWorldDependencies(zCVob* _this, void* vtable, int a0)
	{
		if (_this->homeWorld && _this->type == zVOB_TYPE_LIGHT)
		{
			zCVobLight* light = static_cast<zCVobLight*>(_this);

			if (light->lightData.range > 0)
			{
				const zVEC3& center = light->GetPositionWorld();
				const zVEC3& range = light->lightData.range;

				zTBBox3D bbox;
				bbox.mins = center - range;
				bbox.maxs = center + range;

				zCPolygon** polygons;
				int count;

				_this->homeWorld->bspTree.bspRoot->CollectPolysInBBox3D(bbox, polygons, count);

				for (int i = 0; i < count; i++)
					polygons[i]->ResetLightDynToLightStat();
			}
		}

		Ivk_zCVob_RemoveWorldDependencies(_this, a0);
	}
}
