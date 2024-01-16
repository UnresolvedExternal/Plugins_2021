namespace NAMESPACE
{
	enum class ItemState
	{
		Old = 0,
		New,
		Add
	};

	class InventoryMonitor : public SaveData
	{
	private:
		std::unordered_map<int, int> savedInv;
		std::unordered_map<oCItem*, ItemState> states;

		void SaveItem(oCItem* item, std::unordered_map<int, int>& dest, bool inSlot = false)
		{
			if (!parser->GetSymbol(item->instanz))
			{
				cmd << endl;
				cmd << "zMarkItems: invalid item" << endl;
				LogItem(item);
				return;
			}

			static const int ItLsTorchBurning = parser->GetIndex("ITLSTORCHBURNING");
			static const int ItLsTorch = parser->GetIndex("ITLSTORCH");

			if (!inSlot || ItLsTorchBurning == Invalid || ItLsTorch == Invalid)
			{
				dest[item->instanz] += item->amount;
				return;
			}

			dest[(item->instanz == ItLsTorchBurning) ? ItLsTorch : item->instanz] += item->amount;
		}

		void SaveInventory(oCNpcInventory& inv, std::unordered_map<int, int>& dest)
		{
			dest.clear();

			for (oCItem* item : inv)
				SaveItem(item, dest, false);

			if (oCItem* item = COA(inv.owner, GetLeftHand(), CastTo<oCItem>()))
				SaveItem(item, dest, true);

			if (oCItem* item = COA(inv.owner, GetRightHand(), CastTo<oCItem>()))
				SaveItem(item, dest, true);
		}

	public:
		InventoryMonitor(const string& name) :
			SaveData{ name }
		{

		}

		virtual void Clear() override
		{
			savedInv.clear();
			states.clear();
		}

		virtual void Archive(zCArchiver& arc) override
		{
			arc.WriteInt("instanceCount", savedInv.size());

			for (const std::pair<int, int>& p : savedInv)
			{
				zSTRING name = parser->GetSymbol(p.first)->name;
				int amount = p.second;
				arc.WriteString("instance", name);
				arc.WriteInt("amount", amount);
			}
		}

		virtual void Unarchive(zCArchiver& arc) override
		{
			int size = arc.ReadInt("instanceCount");

			for (int i = 0; i < size; i++)
			{
				zSTRING name = arc.ReadString("instance");
				int amount = arc.ReadInt("amount");
				int instance = parser->GetIndex(name);

				if (instance >= 0)
					savedInv[instance] += amount;
			}
		}

		void Update(oCNpcInventory& inv)
		{
			if (!Options::StaleItemOnSelectOnly)
				return SaveInventory(inv, savedInv);

			BuildStates(inv);
			SaveInventory(inv, savedInv);

			for (const auto& pair : states)
				if (states[pair.first] == ItemState::New && parser->GetSymbol(pair.first->instanz))
					savedInv[pair.first->instanz] = 0;
		}

		void BuildStates(oCNpcInventory& inv)
		{
			states.clear();

			std::unordered_map<int, int> diff;
			SaveInventory(inv, diff);

			for (std::pair<const int, int>& p : diff)
				p.second -= savedInv[p.first];

			oCItem* const selectedItem = inv.GetSelectedItem();

			for (oCItem* item : inv)
			{
				if (item->HasFlag(ITM_FLAG_ACTIVE))
				{
					states[item] = ItemState::Old;
					continue;
				}

				auto it = diff.find(item->instanz);
				ItemState& state = states[item];

				if (state != ItemState::New || !Options::StaleItemOnSelectOnly)
					if (it->second > 0)
						state = (savedInv[item->instanz] > 0) ? ItemState::Add : ItemState::New;
					else
						state = ItemState::Old;

				if (Options::StaleItemOnSelectOnly && selectedItem == item && states[item] == ItemState::New)
				{
					states[item] = ItemState::Old;
					savedInv[item->instanz] += it->second;
				}

				it->second -= item->amount;
			}
		}

		ItemState GetState(oCItem* item)
		{
			return states[item];
		}
	};

	InventoryMonitor& GetInventory()
	{
		static InventoryMonitor& inventory = SaveData::Get<InventoryMonitor>("Inventory");
		return inventory;
	}

	Sub clearInventory(ZSUB(GameEvent::Execute), []()
		{
			Options::TrackNewItems.onChange += []()
			{
				if (!Options::TrackNewItems)
					GetInventory().Clear();
			};
		});

	Sub loadInventory(ZSUB(GameEvent::LoadEnd), Options::TrackNewItems, []()
		{
			if (!SaveLoadGameInfo.changeLevel)
				GetInventory().Load(GameEvent::LoadEnd);
		});

	Sub saveInventory(ZSUB(GameEvent::SaveBegin), Options::TrackNewItems, []()
		{
			if (!SaveLoadGameInfo.changeLevel)
				GetInventory().Save(GameEvent::SaveBegin);
		});

	void __fastcall Hook_oCNpcInventory_Close(oCNpcInventory*, void*);
	Hook<void(__thiscall*)(oCNpcInventory*), ActiveOption<bool>> Ivk_oCNpcInventory_Close(ZENFOR(0x0066C1E0, 0x00699960, 0x006AE810, 0x0070C2F0), &Hook_oCNpcInventory_Close, HookMode::Patch, Options::TrackNewItems);
	void __fastcall Hook_oCNpcInventory_Close(oCNpcInventory* _this, void* vtable)
	{
		if (player && &player->inventory2 == _this && _this->IsOpen() && _this->GetEnableHandleEvent())
			GetInventory().Update(*_this);

		Ivk_oCNpcInventory_Close(_this);
	}
}
