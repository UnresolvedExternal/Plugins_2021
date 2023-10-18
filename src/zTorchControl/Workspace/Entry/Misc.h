namespace NAMESPACE
{
	bool IsBurningTorch(zCVob* vob)
	{
		static int instance = parser->GetIndex("ITLSTORCHBURNING");

		if (instance < 0)
			return false;

		return vob && vob->type == zVOB_TYPE_ITEM && static_cast<oCItem*>(vob)->instanz == instance;
	}

	bool TryExchangeTorch(oCNpc* npc)
	{
		oCWorld* world = ogame->GetGameWorld();

		if (!world)
			return false;

		oCItem* torch = npc->GetSlotItem(NPC_NODE_LEFTHAND);

		if (!IsBurningTorch(torch))
			return false;

		int instance = torch->instanz;
		npc->DropFromSlot(NPC_NODE_LEFTHAND);
		
		torch->AddRef();
		world->RemoveVobSubtree(torch);
		world->RemoveVob(torch);
		torch->Release();

		torch = static_cast<oCItem*>(world->CreateVob(zVOB_TYPE_ITEM, instance));

		if (!torch)
			return false;

		world->AddVob(torch);
		npc->PutInSlot(NPC_NODE_LEFTHAND, torch, false);
		torch->Release();

		return true;
	}

	Sub clearTorches(ZSUB(GameEvent::LoadBegin_ChangeLevel), Options::TorchRemoveRange, []()
		{
			std::vector<oCItem*> torches;
			torches.reserve(64u);

			for (oCItem* item : ogame->GetGameWorld()->voblist_items)
				if (IsBurningTorch(item) && !item->HasFlag(ITM_FLAG_NFOCUS))
					torches += item;
			
			for (oCItem* torch : torches)
			{
				torch->AddRef();
				ogame->world->RemoveVobSubtree(torch);
				ogame->world->RemoveVob(torch);
				torch->Release();
			}
		});

	Sub removeFarTorches(ZSUB(GameEvent::Loop), Options::TorchRemoveRange, []()
		{
			static Timer timer;

			if (!ogame->GetCameraVob() || !timer[0u].Await(1000u))
				return;
			
			oCWorld* world = ogame->GetGameWorld();

			std::vector<oCItem*> torches;
			torches.reserve(64u);

			for (oCItem* item : world->voblist_items)
			{
				if (!IsBurningTorch(item) || item->HasFlag(ITM_FLAG_NFOCUS))
					continue;

				if (player && item->GetDistanceToVobApprox(*player) <= Options::TorchRemoveRange)
					continue;

				if (item->GetDistanceToVobApprox(*ogame->GetCameraVob()) <= Options::TorchRemoveRange)
					continue;

				torches += item;
			}

			for (oCItem* torch : torches)
			{
				torch->AddRef();
				world->RemoveVobSubtree(torch);
				world->RemoveVob(torch);
				torch->Release();
			}
		});

	Sub exchangeTorchOnTeleport(ZSUB(GameEvent::Loop), Options::ExchangeTorchOnTeleport, []()
		{
			if (!player)
				return;

			static zVEC3 lastPos = player->GetPositionWorld();
			zVEC3 pos = player->GetPositionWorld();

			if (lastPos.Distance(pos) > 500.0f)
				TryExchangeTorch(player);

			lastPos = pos;
		});

	Sub exchangeTorchOnLevelChange(ZSUB(GameEvent::LoadEnd_ChangeLevel), Options::ExchangeTorchOnTeleport, []()
		{
			if (player)
				TryExchangeTorch(player);
		});
}
