namespace NAMESPACE
{
	class TorchData : public SaveData
	{
	public:
		TorchData(const string& name) :
			SaveData{ name }
		{

		}

		virtual void Clear() override
		{

		}

		virtual void Archive(zCArchiver& arc) override
		{
			arc.WriteBool("PlayerHasTorch", IsBurningTorch(COA(player, GetLeftHand())));

			std::vector<oCItem*> torches;
			torches.reserve(64u);

			for (oCItem* item : ogame->GetGameWorld()->voblist_items)
				if (IsBurningTorch(item) && !item->HasFlag(ITM_FLAG_NFOCUS) && item->homeWorld == ogame->world)
					torches += item;

			arc.WriteInt("Count", torches.size());

			for (oCItem* torch : torches)
			{
				torch->dontWriteIntoArchive = true;
				arc.WriteRaw("Matrix", &torch->trafoObjToWorld, sizeof(torch->trafoObjToWorld));
			}
		}

		virtual void Unarchive(zCArchiver& arc) override
		{
			oCWorld* world = ogame->GetGameWorld();
			int instance = parser->GetIndex("ITLSTORCHBURNING");

			if (instance < 0)
				return;

			bool playerHasTorch = arc.ReadBool("PlayerHasTorch");

			if (player && playerHasTorch && !player->GetLeftHand())
				if (ZOwner<oCItem> torch{ static_cast<oCItem*>(world->CreateVob(zVOB_TYPE_ITEM, instance)) })
				{
					world->AddVob(torch.get());
					player->PutInSlot(NPC_NODE_LEFTHAND, torch.get(), 0);
				}

			int count = arc.ReadInt("Count");

			for (int i = 0; i < count; i++)
				if (ZOwner<oCItem> torch{ static_cast<oCItem*>(world->CreateVob(zVOB_TYPE_ITEM, instance)) })
				{
					zMAT4 mat;
					arc.ReadRaw("Matrix", &mat, sizeof(mat));
					torch->SetTrafoObjToWorld(mat);
					world->AddVob(torch.get());
				}
				else
					break;
		}
	};

	Sub saveTorches(ZSUB(GameEvent::SaveBegin), Settings::SaveTorches, []()
		{
			SaveData::Get<TorchData>(ogame->GetGameWorld()->GetWorldName() + ".Torches").Save(GameEvent::SaveBegin);
		});

	Sub loadTorches(ZSUB(GameEvent::LoadEnd), Settings::SaveTorches, []()
		{
			SaveData::Get<TorchData>(ogame->GetGameWorld()->GetWorldName() + ".Torches").Load(GameEvent::LoadEnd);
		});
}
