namespace NAMESPACE
{
	class TimedOverlaysSaveData : public SaveData
	{
	private:
		struct Overlay
		{
			zSTRING name;
			float time;
		};

		std::vector<Overlay> overlays;

	public:
		TimedOverlaysSaveData(const string& name) :
			SaveData{ name }
		{
			overlays.reserve(16u);
		}

		virtual void Clear() override
		{
			overlays.clear();
		}

		virtual void Archive(zCArchiver& arc) override
		{
			arc.WriteInt("SIZE", overlays.size());

			for (const Overlay& overlay : overlays)
			{
				arc.WriteString("NAME", overlay.name);
				arc.WriteFloat("TIME", overlay.time);
			}
		}

		virtual void Unarchive(zCArchiver& arc) override
		{
			int size = arc.ReadInt("SIZE");
			overlays.reserve(size);

			for (int i = 0; i < size; i++)
			{
				overlays += {};
				overlays.back().name = arc.ReadString("NAME");
				overlays.back().time = arc.ReadFloat("TIME");
			}
		}

		void Update(oCNpc* npc)
		{
			overlays.clear();
			overlays.reserve(npc->timedOverlays.GetNum());

			for (const oCNpc::oCNpcTimedOverlay* overlay : npc->timedOverlays)
				if (overlay->timer > 0.0f)
				{
					overlays += {};
					overlays.back().name = overlay->mdsOverlayName;
					overlays.back().time = overlay->timer;
				}
		}
		
		void Apply(oCNpc* npc)
		{
			for (const Overlay& overlay : overlays)
				npc->ApplyTimedOverlayMds(overlay.name, overlay.time);
		}
	};

	TimedOverlaysSaveData& GetOverlays()
	{
		static TimedOverlaysSaveData& overlays = SaveData::Get<TimedOverlaysSaveData>("Overlays");
		return overlays;
	}

	Sub saveOverlays(ZSUB(GameEvent::SaveBegin), Options::SaveTimedOverlays, []()
		{
			if (SaveLoadGameInfo.changeLevel)
			{
				if (oCNpc::dontArchiveThisNpc)
					GetOverlays().Update(oCNpc::dontArchiveThisNpc);

				return;
			}

			if (!player)
				return;

			GetOverlays().Update(player);
			GetOverlays().Save(GameEvent::SaveBegin);
			GetOverlays().Clear();
		});

	Sub loadOverlays(ZSUB(GameEvent::LoadEnd), Options::SaveTimedOverlays, []()
		{
			if (!player)
				return;

			if (!SaveLoadGameInfo.changeLevel)
				GetOverlays().Load(GameEvent::LoadEnd);

			GetOverlays().Apply(player);
			GetOverlays().Clear();
		});
}
