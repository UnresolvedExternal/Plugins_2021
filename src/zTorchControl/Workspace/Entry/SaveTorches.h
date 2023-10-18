namespace NAMESPACE
{
	class TorchData : public SaveData
	{
	private:
		class TempNonArchiving
		{
		private:
			oCItem* item;

		public:
			TempNonArchiving(oCItem* item) :
				item{ item }
			{
				item->dontWriteIntoArchive = true;
				item->AddRef();
			}

			TempNonArchiving(TempNonArchiving&& y) :
				item{ y.item }
			{
				y.item = nullptr;
			}

			~TempNonArchiving()
			{
				if (!item)
					return;

				item->dontWriteIntoArchive = false;
				item->Release();
			}
		};

		bool playerHasTorch_LevelChange;
		std::vector<TempNonArchiving> nonArchivingList;

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
			oCItem* playerTorch = COA(player, GetLeftHand(), CastTo<oCItem>());

			if (!IsBurningTorch(playerTorch))
				playerTorch = nullptr;

			if (playerTorch && !playerTorch->dontWriteIntoArchive)
				nonArchivingList.emplace_back(playerTorch);

			arc.WriteBool("PlayerHasTorch", playerTorch && !SaveLoadGameInfo.changeLevel);
			playerHasTorch_LevelChange = SaveLoadGameInfo.changeLevel && playerTorch;
			
			std::vector<oCItem*> torches;
			torches.reserve(64u);

			for (oCItem* item : ogame->GetGameWorld()->voblist_items)
				if (IsBurningTorch(item) && !item->HasFlag(ITM_FLAG_NFOCUS) && item->homeWorld == ogame->world)
					torches += item;

			arc.WriteInt("Count", torches.size());

			for (oCItem* torch : torches)
			{
				if (!torch->dontWriteIntoArchive)
					nonArchivingList.emplace_back(torch);

				arc.WriteRaw("Matrix", &torch->trafoObjToWorld, sizeof(torch->trafoObjToWorld));
			}
		}

		virtual void Unarchive(zCArchiver& arc) override
		{
			oCWorld* world = ogame->GetGameWorld();
			int ItLsTorchBurning = parser->GetIndex("ITLSTORCHBURNING");

			if (ItLsTorchBurning < 0)
				return;

			bool playerHasTorch = arc.ReadBool("PlayerHasTorch");

			if (SaveLoadGameInfo.changeLevel)
				playerHasTorch = playerHasTorch_LevelChange;

			if (player && playerHasTorch && !player->GetLeftHand())
				if (ZOwner<oCItem> torch{ static_cast<oCItem*>(world->CreateVob(zVOB_TYPE_ITEM, ItLsTorchBurning)) })
				{
					world->AddVob(torch.get());
					player->PutInSlot(NPC_NODE_LEFTHAND, torch.get(), 0);
				}

			int count = arc.ReadInt("Count");

			for (int i = 0; i < count; i++)
			{
				zMAT4 mat;
				arc.ReadRaw("Matrix", &mat, sizeof(mat));

				if (ZOwner<oCItem> torch{ static_cast<oCItem*>(world->CreateVob(zVOB_TYPE_ITEM, ItLsTorchBurning)) })
				{
					torch->SetTrafoObjToWorld(mat);
					world->AddVob(torch.get());
				}
			}
		}

		void RestoreArchiving()
		{
			nonArchivingList.clear();
		}
	};

#if ENGINE >= Engine_G2

	TorchData* lastTorchData = nullptr;

	Sub saveTorches(ZSUB(GameEvent::SaveBegin), Options::SaveTorches, []()
		{
			VarScope<oCNpc*> scope;

			if (SaveLoadGameInfo.changeLevel)
				scope = AssignTemp(player, oCNpc::dontArchiveThisNpc);

			lastTorchData = &SaveData::Get<TorchData>(ogame->GetGameWorld()->GetWorldName() + ".Torches");
			lastTorchData->Save(GameEvent::SaveBegin);
		});

	Sub restoreArchiving(ZSUB(GameEvent::SaveEnd), Options::SaveTorches, []()
		{
			if (!lastTorchData)
				return;
				
			lastTorchData->RestoreArchiving();
			lastTorchData = nullptr;
		});

	Sub loadTorches(ZSUB(GameEvent::LoadEnd), Options::SaveTorches, []()
		{
			SaveData::Get<TorchData>(ogame->GetGameWorld()->GetWorldName() + ".Torches").Load();
		});

#endif
}
