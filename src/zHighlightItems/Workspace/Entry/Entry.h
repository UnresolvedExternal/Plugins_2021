#include <queue>

namespace NAMESPACE
{
	class HighlightedItem;

	std::vector<std::unique_ptr<HighlightedItem>> highlightedItemList;
	std::unordered_set<oCItem*> highlightedItemSet;

	std::queue<ZOwner<oCItem>> highlightingQueue;
	std::unordered_set<oCItem*> highlightingQueueSet;

	Sub clearHighlightedItems(ZSUB(GameEvent::LoadBegin, GameEvent::Exit), []()
		{
			highlightedItemList.clear();
			
			while (!highlightingQueue.empty())
				highlightingQueue.pop();player->human_ai->

			highlightingQueueSet.clear();
		});

	class HighlightedItem
	{
	private:
		ZOwner<oCItem> item;
		ZOwner<oCVisualFX> effect;

	public:
		HighlightedItem(oCItem* item, oCVisualFX* effect)
		{
			item->AddRef();
			effect->AddRef();

			this->item.reset(item);
			this->effect.reset(effect);

			highlightedItemSet.insert(item);
		}

		~HighlightedItem()
		{
			effect->Kill();
			highlightedItemSet.erase(highlightedItemSet.find(GetItem()));
		}

		static HighlightedItem* TryCreate(oCItem* item, const string& effectName)
		{
			if (COA(item, GetHomeWorld()) != ogame->GetWorld())
				return nullptr;

			if (item->HasFlag(ITM_FLAG_NFOCUS) || item->GetIsProjectile() || !item->GetVisual())
				return nullptr;

			if (highlightedItemSet.find(item) != highlightedItemSet.end())
				return nullptr;

			ZOwner<oCVisualFX> effect{ oCVisualFX::CreateAndPlay(Z effectName, item, nullptr, 1, 0, 0, false) };

			if (!effect)
				return nullptr;

			effect->SetPFXShapeVisual(item->GetVisual(), false);
			highlightedItemList += std::make_unique<HighlightedItem>(item, effect.get());
			return highlightedItemList.back().get();
		}

		oCItem* GetItem()
		{
			return item.get();
		}
	};

	bool GetHighlightingEnabled()
	{
		if (!*Options::KeyPress && !*Options::KeyToggle)
			return false;

		if (Options::ScanRange == 0 || Options::EffectsPerFrame == 0)
			return false;

		if (Options::EffectName->IsEmpty())
			return false;

		static bool enabled = false;

		if (!oCInformationManager::GetInformationManager().HasFinished())
			return enabled;

		if (zCConsole::cur_console)
			return enabled;

		if (Options::KeyToggle->GetToggled())
			enabled = !enabled;

		return Options::KeyPress->GetPressed() ? !enabled : enabled;
	}

	bool NeedHighlight(oCItem* item)
	{
		if (!item || item->GetHomeWorld() != ogame->GetWorld() || !item->GetVisual())
			return false;

		if (!ogame->GetCameraVob() || item->GetDistanceToVob(*ogame->GetCameraVob()) > Options::ScanRange)
			return false;

		return true;
	}

	Sub removeHighlightings(ZSUB(GameEvent::Loop), []()
		{
			if (!GetHighlightingEnabled())
				return highlightedItemList.clear();

			auto isExpired = [](auto& highlightedItem)
			{
				return !NeedHighlight(highlightedItem->GetItem());
			};

			auto newEnd = std::remove_if(highlightedItemList.begin(), highlightedItemList.end(), isExpired);
			highlightedItemList.erase(newEnd, highlightedItemList.end());
		});

	Sub populateHighlightingQueue(ZSUB(GameEvent::Loop), []()
		{
			if (!GetHighlightingEnabled())
			{
				while (!highlightingQueue.empty())
					highlightingQueue.pop();

				highlightingQueueSet.clear();
				return;
			}

			static Timer timer;

			if (!timer[0u].Await(static_cast<uint>(std::max(0, *Options::ScanPeriodMs)), true))
				return;
			
			if (Options::ScanRange < 0)
			{
				for (oCItem* item : ogame->GetGameWorld()->voblist_items)
					if (highlightedItemSet.find(item) == highlightedItemSet.end() && highlightingQueueSet.find(item) == highlightingQueueSet.end() && NeedHighlight(item))
					{
						item->AddRef();
						highlightingQueue.emplace(item);
						highlightingQueueSet.insert(item);
					}

				return;
			}

			if (!ogame->GetCameraVob())
				return;

			zTBBox3D bbox;
			bbox.mins = bbox.maxs = ogame->GetCameraVob()->GetPositionWorld();
			bbox.mins -= *Options::ScanRange;
			bbox.maxs += *Options::ScanRange;

			zCArray<zCVob*> vobs;
			ogame->GetWorld()->bspTree.bspRoot->CollectVobsInBBox3D(vobs, bbox);

			for (zCVob* vob : vobs)
			{
				if (!vob || vob->GetVobType() != zVOB_TYPE_ITEM)
					continue;

				oCItem* const item = static_cast<oCItem*>(vob);

				if (highlightedItemSet.find(item) != highlightedItemSet.end() || highlightingQueueSet.find(item) != highlightingQueueSet.end())
					continue;

				if (!NeedHighlight(item))
					continue;

				item->AddRef();
				highlightingQueue.emplace(item);
				highlightingQueueSet.insert(item);
			}
		});

	Sub addHighlightings(ZSUB(GameEvent::Loop), []()
		{
			const zSTRING effectName = Options::EffectName;

			for (int effectsLimit = Options::EffectsPerFrame; effectsLimit && !highlightingQueue.empty(); highlightingQueue.pop())
			{
				oCItem* const item = highlightingQueue.front().get();
				highlightingQueueSet.erase(highlightingQueueSet.find(item));

				if (NeedHighlight(item) && HighlightedItem::TryCreate(item, effectName))
					effectsLimit = effectsLimit < 0 ? effectsLimit : (effectsLimit - 1);
			}
		});
}
