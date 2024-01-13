#include <regex>

namespace NAMESPACE
{
	std::unordered_map<zCModelAni*, int> cache;

	Sub listenAnimations(ZSUB(GameEvent::Execute), []
		{
			Options::Animations.onChange += []
			{
				cache.clear();
			};
		});

	Sub clearCache(ZSUB(GameEvent::LoadBegin), []
		{
			cache.clear();
		});

	float __cdecl RescaleAni(zCModel* model, zCModelAniActive* ani, float frameDelta)
	{
		if (!player || model->homeVob != player || !ani->protoAni)
			return frameDelta;
	
		auto it = cache.find(ani->protoAni);

		if (it != cache.end())
			if (it->second == -1)
				return frameDelta;
			else
				return frameDelta * (*Options::Animations)[it->second].speed;

		for (size_t i = 0; i < Options::Animations->size(); i++)
		{
			const Options::AniSpeedEntry& e = (*Options::Animations)[i];
			std::regex regex{ e.regex };
			
			if (!std::regex_search(std::string{ ani->protoAni->aniName }, regex))
				continue;

			cache.insert({ ani->protoAni, i });
			return frameDelta * e.speed;
		}

		cache.insert({ ani->protoAni, -1 });
		return frameDelta;
	}

	Sub executePatch(ZSUB(GameEvent::Init), []
		{
			CPatchInteger rescaleAni;
			rescaleAni.Init();
			rescaleAni.SetObjectName("RescaleAni");
			rescaleAni.SetValue(reinterpret_cast<int>(&RescaleAni));
			rescaleAni.DontRemove();

			CPatch::ExecuteResource(CPlugin::GetCurrentPlugin()->GetModule(), MAKEINTRESOURCE(IDR_PATCH1), "PATCH");
		});
}
