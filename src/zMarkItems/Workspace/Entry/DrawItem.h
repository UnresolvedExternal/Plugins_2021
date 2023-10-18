namespace NAMESPACE
{
	bool inContainerDraw;
	bool inPlayerContainer;
	std::unordered_map<oCItem*, ItemState> itemStates;

	struct DrawElement
	{
		int prio;
		std::function<bool(oCItem*, zCWorld*, zCViewBase*, float)> func;

		DrawElement& operator=(const DrawElement& right)
		{
			prio = right.prio;
			func = right.func;
			return *this;
		}
	};

	std::vector<DrawElement> drawElements;

	void __fastcall Hook_oCItemContainer_Draw(oCItemContainer*, void*);
	Hook<void(__thiscall*)(oCItemContainer*), ActiveValue<bool>> Ivk_oCItemContainer_Draw(ZENFOR(0x00667660, 0x00695040, 0x006A9BC0, 0x007076B0), &Hook_oCItemContainer_Draw, HookMode::Patch, Options::HookRenderContainer);
	void __fastcall Hook_oCItemContainer_Draw(oCItemContainer* _this, void* vtable)
	{
		auto a = AssignTemp(inContainerDraw, true);
		auto b = AssignTemp(inPlayerContainer, _this == &player->inventory2);

		if (inPlayerContainer)
			GetInventory().BuildStates(player->inventory2);

		Ivk_oCItemContainer_Draw(_this);
	}

	bool TryDrawUnreadDoc(oCItem* item, zCWorld* rndWorld, zCViewBase* baseView, float time)
	{
		if (!inContainerDraw || !Options::UnreadDocPrio || !item->HasFlag(ITM_CAT_DOCS) || GetDocs().Contains(item->GetInstance()) || !item->scemeName.Length())
			return false;

		zCView* view = dynamic_cast<zCView*>(baseView);
		if (!view)
			return false;

		const auto& pos = *Options::UnreadDocTexPos;

		zCView itemView(pos[0], pos[1], pos[2], pos[3]);
		itemView.InsertBack(Options::UnreadDocTexName->GetVector());
		itemView.SetAlphaBlendFunc(zRND_ALPHA_FUNC_BLEND);
		itemView.SetTransparency(Options::UnreadDocOpacity);
		view->InsertItem(&itemView);
		itemView.Blit();
		view->RemoveItem(&itemView);

		return true;
	}

	bool TryDrawNewItem(oCItem* item, zCWorld* rndWorld, zCViewBase* baseView, float time)
	{
		if (!inContainerDraw || !inPlayerContainer || !Options::NewItemPrio || GetInventory().GetState(item) != ItemState::New)
			return false;

		zCView* view = dynamic_cast<zCView*>(baseView);
		if (!view)
			return false;

		const auto& pos = *Options::NewItemTexPos;

		zCView itemView(pos[0], pos[1], pos[2], pos[3]);
		itemView.InsertBack(Options::NewItemTexName->GetVector());
		itemView.SetAlphaBlendFunc(zRND_ALPHA_FUNC_BLEND);
		itemView.SetTransparency(Options::NewItemOpacity);
		view->InsertItem(&itemView);
		itemView.Blit();
		view->RemoveItem(&itemView);

		return true;
	}

	bool TryDrawAddItem(oCItem* item, zCWorld* rndWorld, zCViewBase* baseView, float time)
	{
		if (!inContainerDraw || !inPlayerContainer || !Options::AddItemPrio || GetInventory().GetState(item) != ItemState::Add)
			return false;

		zCView* view = dynamic_cast<zCView*>(baseView);
		if (!view)
			return false;

		const auto& pos = *Options::AddItemTexPos;

		zCView itemView(pos[0], pos[1], pos[2], pos[3]);
		itemView.InsertBack(Options::AddItemTexName->GetVector());
		itemView.SetAlphaBlendFunc(zRND_ALPHA_FUNC_BLEND);
		itemView.SetTransparency(Options::AddItemOpacity);
		view->InsertItem(&itemView);
		itemView.Blit();
		view->RemoveItem(&itemView);

		return true;
	}

	extern Hook<void(__thiscall*)(oCItem*, zCWorld*, zCViewBase*, float), ActiveValue<bool>> Ivk_oCItem_RenderItem;

	bool TryDrawItem(oCItem* item, zCWorld* rndWorld, zCViewBase* baseView, float time)
	{
		Ivk_oCItem_RenderItem(item, rndWorld, baseView, time);
		return true;
	}

	void __fastcall Hook_oCItemContainer_DrawItemInfo(oCItemContainer*, void*, oCItem*, zCWorld*);
	Hook<void(__thiscall*)(oCItemContainer*, oCItem*, zCWorld*), ActiveValue<bool>> Ivk_oCItemContainer_DrawItemInfo(ZENFOR(0x00667220, 0x00694AD0, 0x006A9350, 0x00706E40), &Hook_oCItemContainer_DrawItemInfo, HookMode::Patch, Options::HookRenderItem);
	void __fastcall Hook_oCItemContainer_DrawItemInfo(oCItemContainer* _this, void* vtable, oCItem* a0, zCWorld* a1)
	{
		auto scope = AssignTemp(inContainerDraw, false);
		Ivk_oCItemContainer_DrawItemInfo(_this, a0, a1);
	}

	void __fastcall Hook_oCItem_RenderItem(oCItem*, void*, zCWorld*, zCViewBase*, float);
	Hook<void(__thiscall*)(oCItem*, zCWorld*, zCViewBase*, float), ActiveValue<bool>> Ivk_oCItem_RenderItem(ZenDef<TInstance>(0x00672F70, 0x006A0A20, 0x006B5A30, 0x00713AC0), &Hook_oCItem_RenderItem, HookMode::Patch, Options::HookRenderItem);
	void __fastcall Hook_oCItem_RenderItem(oCItem* _this, void* vtable, zCWorld* world, zCViewBase* viewBase, float time)
	{
		if (!drawElements.size())
		{
			DrawElement ele;

			if (Options::UnreadDocPrio && Options::TrackReadDocs)
			{
				ele.prio = Options::UnreadDocPrio;
				ele.func = &TryDrawUnreadDoc;
				drawElements.push_back(ele);
			}

			if (Options::NewItemPrio && Options::TrackNewItems)
			{
				ele.prio = Options::NewItemPrio;
				ele.func = &TryDrawNewItem;
				drawElements.push_back(ele);
			}

			if (Options::AddItemPrio && Options::TrackNewItems)
			{
				ele.prio = Options::AddItemPrio;
				ele.func = &TryDrawAddItem;
				drawElements.push_back(ele);
			}

			ele.prio = 0;
			ele.func = &TryDrawItem;
			drawElements.push_back(ele);

			std::sort(begin(drawElements), end(drawElements), [](const DrawElement& left, const DrawElement& right)
				{
					return left.prio < right.prio;
				});
		}

		for (const DrawElement& ele : drawElements)
			ele.func(_this, world, viewBase, time);
	}

	Sub updateDrawElements(ZSUB(GameEvent::Execute), []()
		{
			auto clearDrawElements = []()
			{
				drawElements.clear();
			};

			Options::TrackNewItems.onChange += clearDrawElements;
			Options::TrackReadDocs.onChange += clearDrawElements;
			Options::NewItemPrio.onChange += clearDrawElements;
			Options::AddItemPrio.onChange += clearDrawElements;
			Options::UnreadDocPrio.onChange += clearDrawElements;
		});
}