namespace NAMESPACE
{
	CInvoke<zSTRING(__cdecl*)(int)> Ivk_oCAniCtrl_Human_GetWeaponHitString(ZENFOR(0x00626390, 0x0064B240, 0x00652670, 0x006AEEF0), nullptr, IVK_DISABLED);

#define SAVE_INT(value) arc.WriteInt(#value, (value))
#define LOAD_INT(value) value = arc.ReadInt(#value);

	class SkillInfo
	{
	private:
		int GetWeaponMode()
		{
			switch (talentId)
			{
			case oCNpcTalent::NPC_TAL_1H: return NPC_WEAPON_1HS;
			case oCNpcTalent::NPC_TAL_2H: return NPC_WEAPON_2HS;
			case oCNpcTalent::NPC_TAL_BOW: return NPC_WEAPON_BOW;
			case oCNpcTalent::NPC_TAL_CROSSBOW: return NPC_WEAPON_CBOW;
			}

			ASSERT(false);
			return -1;
		}

		zSTRING GetOverlay(oCNpc* npc, int skill)
		{
			zSTRING overlay = npc->GetModel()->modelProtoList[0]->modelProtoName;
			overlay += "_";

			zSTRING wStr = Ivk_oCAniCtrl_Human_GetWeaponHitString(GetWeaponMode());

			overlay += wStr;
			overlay += "T";
			overlay += Z skill;
			overlay += ".MDS";

			return overlay;
		}

	public:
		static constexpr int NotSet = -1;

		int talentId;
		int absoluteMax;
		int realValue;
		int selectedValue;

		SkillInfo(int talentId) :
			talentId{ talentId }
		{
			Clear();
		}

		SkillInfo(const SkillInfo&) = default;
		SkillInfo& operator=(const SkillInfo&) = default;

		void Clear()
		{
			absoluteMax = 0;
			realValue = NotSet;
			selectedValue = NotSet;
		}

		void Archive(zCArchiver& arc)
		{
			SAVE_INT(talentId);
			SAVE_INT(absoluteMax);
			SAVE_INT(realValue);
			SAVE_INT(selectedValue);
		}

		void Unarchive(zCArchiver& arc)
		{
			LOAD_INT(talentId);
			LOAD_INT(absoluteMax);
			LOAD_INT(realValue);
			LOAD_INT(selectedValue);
		}

		bool TestWeaponMode(oCNpc* npc)
		{
			return GetWeaponMode() == npc->GetWeaponMode();
		}

		void Apply(oCNpc* npc)
		{
			if (selectedValue == NotSet)
				return;

			if (!npc->GetModel() || !npc->GetAnictrl() || !npc->IsHuman())
				return;

			for (int i = 0; i <= absoluteMax; i++)
				npc->RemoveOverlay(GetOverlay(npc, i));

			npc->ApplyOverlay(GetOverlay(npc, selectedValue));

			if (TestWeaponMode(npc))
				npc->GetAnictrl()->SetFightAnis(npc->GetWeaponMode());
		}

		bool Change(int dSkill)
		{
			int newValue = (selectedValue == NotSet) ? realValue : selectedValue;
			newValue = CoerceInRange(newValue + dSkill, 0, 0, realValue);

			if (selectedValue == newValue || selectedValue == NotSet && newValue == realValue)
				return false;

			selectedValue = newValue;
			return true;
		}
	};

	class SkillController : public SaveData
	{
	private:
		std::vector<Sub<void>> subs;
		std::vector<SkillInfo> skills;

		void OnPreLoop()
		{
			if (!player || !player->IsHuman())
				return;

			for (SkillInfo& skill : skills)
			{
				const int realValue = player->GetTalentSkill(skill.talentId);
				bool needApply = realValue != skill.realValue;
				skill.realValue = realValue;
				skill.absoluteMax = std::max(skill.absoluteMax, realValue);
				
				if (skill.TestWeaponMode(player))
				{
					const int dSkill = Options::DecreaseSkillKey->GetToggled() ? -1 : Options::IncreaseSkillKey->GetToggled() ? +1 : 0;
					
					if (skill.Change(dSkill))
						needApply = true;
				}

				if (needApply)
					skill.Apply(player);
			}
		}

		void OnLoadEnd()
		{
			Load(GameEvent::LoadEnd);

			for (SkillInfo& skill : skills)
				skill.Apply(player);
		}

		void OnSaveBegin()
		{
			Save(GameEvent::SaveBegin);
		}

	public:
		SkillController(const string& name) :
			SaveData{ name }
		{
			skills.emplace_back(oCNpcTalent::NPC_TAL_1H);
			skills.emplace_back(oCNpcTalent::NPC_TAL_2H);
			skills.emplace_back(oCNpcTalent::NPC_TAL_BOW);
			skills.emplace_back(oCNpcTalent::NPC_TAL_CROSSBOW);

			ADDSUB(LoadEnd);
			ADDSUB(SaveBegin);
			ADDSUB(PreLoop);
		}

		virtual void Clear() override
		{
			for (SkillInfo& skill : skills)
				skill.Clear();
		}

		virtual void Archive(zCArchiver& arc) override
		{
			for (SkillInfo& skill : skills)
				skill.Archive(arc);
		}

		virtual void Unarchive(zCArchiver& arc) override
		{
			for (SkillInfo& skill : skills)
				skill.Unarchive(arc);
		}
	};

	Sub createSkills(ZSUB(GameEvent::Entry), []()
		{
			SaveData::Get<SkillController>("SkillInfo");
		});
}
