namespace NAMESPACE
{
	class BlockedItem
	{
	private:
		oCItem* item;

	public:
		BlockedItem(oCItem* item) :
			item{ item }
		{
			item->SetFlag(ITM_FLAG_ACTIVE);
		}

		BlockedItem(BlockedItem&& y) :
			item{ y.item }
		{
			y.item = nullptr;
		}

		BlockedItem& operator=(BlockedItem&& y)
		{
			this->~BlockedItem();
			item = y.item;
			y.item = nullptr;
		}

		~BlockedItem()
		{
			if (item)
				item->ClearFlag(ITM_FLAG_ACTIVE);
		}
	};

	class TradersInfo : public SaveData
	{
	private:
		std::unordered_map<int, std::unordered_set<int>> infos;
		std::vector<BlockedItem> items;

		static bool IsCurrency(oCItem* item)
		{
#if ENGINE >= Engine_G2
			return item->instanz == oCItemContainer::GetCurrencyInstance();
#else
			static const int currency = parser->GetIndex("ItmiNugget");
			return item->instanz == currency;
#endif
		}

		void UpdateTrader(oCNpc* trader)
		{
			for (oCItem* item : trader->inventory2)
				if (!item->HasFlag(ITM_FLAG_ACTIVE) && !IsCurrency(item))
					infos[trader->instanz].insert(item->mainflag);
		}

	public:
		TradersInfo(const string& name) :
			SaveData{ name }
		{

		}

		virtual void Clear() override
		{
			infos.clear();
		}

		virtual void Archive(zCArchiver& arc) override
		{
			arc.WriteInt("Entries", infos.size());

			for (const auto& pair : infos)
			{
				arc.WriteString("Trader", parser->GetSymbol(pair.first)->name);
				arc.WriteInt("Categories", pair.second.size());
				
				for (int category : pair.second)
					arc.WriteInt("Category", category);
			}
		}

		virtual void Unarchive(zCArchiver& arc) override
		{
			int size = arc.ReadInt("Entries");
			infos.reserve(size);

			for (int i = 0; i < size; i++)
			{
				const string name = arc.ReadString("Trader");
				const int categories = arc.ReadInt("Categories");

				for (int k = 0; k < categories; k++)
				{
					const int category = arc.ReadInt("Category");
					const int index = parser->GetIndex(name);

					if (index != -1)
						infos[index].insert(category);
				}
			}
		}

		void BlockPlayerItems(oCNpc* trader)
		{
			if (!trader)
			{
				items.clear();
				return;
			}

			UpdateTrader(trader);

			const auto& categories = infos[trader->instanz];

			for (oCItem* item : player->inventory2)
				if (!item->HasFlag(ITM_FLAG_ACTIVE) && !IsCurrency(item) && categories.find(item->mainflag) == categories.end())
					items.emplace_back(item);
		}
	};

	TradersInfo& GetTradersInfo()
	{
		static TradersInfo& info = SaveData::Get<TradersInfo>("TradersInfo");
		return info;
	}

	Sub saveTraders(ZSUB(GameEvent::SaveBegin), []()
		{
			GetTradersInfo().Save(GameEvent::SaveBegin);
		});

	Sub loadTraders(ZSUB(GameEvent::LoadEnd), []()
		{
			GetTradersInfo().Load(GameEvent::LoadEnd);
		});

	void __fastcall Hook_oCViewDialogTrade_SetNpcLeft(oCViewDialogTrade*, oCNpc*);
	Hook<void(__fastcall*)(oCViewDialogTrade*, oCNpc*), ActiveOption<bool>> Ivk_oCViewDialogTrade_SetNpcLeft(ZENFOR(0x00729440, 0x007672E0, 0x00775450, 0x0068B180), &Hook_oCViewDialogTrade_SetNpcLeft, HookMode::Patch, Options::FilterSellItems);
	void __fastcall Hook_oCViewDialogTrade_SetNpcLeft(oCViewDialogTrade* _this, oCNpc* a0)
	{
		if (a0)
			GetTradersInfo().BlockPlayerItems(a0);

		Ivk_oCViewDialogTrade_SetNpcLeft(_this, a0);
	}

	void __fastcall Hook_oCViewDialogTrade_OnExit(oCViewDialogTrade*);
	Hook<void(__fastcall*)(oCViewDialogTrade*), ActiveOption<bool>> Ivk_oCViewDialogTrade_OnExit(ZENFOR(0x0072AAB0, 0x00768BB0, 0x007762D0, 0x0068C000), &Hook_oCViewDialogTrade_OnExit, HookMode::Patch, Options::FilterSellItems);
	void __fastcall Hook_oCViewDialogTrade_OnExit(oCViewDialogTrade* _this)
	{
		GetTradersInfo().BlockPlayerItems(nullptr);
		Ivk_oCViewDialogTrade_OnExit(_this);
	}

	Sub listenFilterOption(ZSUB(GameEvent::Execute), []()
		{
			Options::FilterSellItems.onChange += []()
			{
				if (!Options::FilterSellItems)
					GetTradersInfo().BlockPlayerItems(nullptr);
			};
		});
}
