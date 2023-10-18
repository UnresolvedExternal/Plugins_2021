#include <array>

namespace NAMESPACE
{
	int GetMinimalDamage()
	{
		static Symbol symbol{ parser, "NPC_MINIMAL_DAMAGE" };

		if (!symbol)
			return 0;

		return symbol.GetValue<int>(0);
	}

	void __fastcall Hook_oCNpc_OnDamage_Hit(oCNpc*, void*, oCNpc::oSDamageDescriptor&);
	Hook<void(__thiscall*)(oCNpc*, oCNpc::oSDamageDescriptor&)> Ivk_oCNpc_OnDamage_Hit(ZENFOR(0x00731410, 0x007700A0, 0x0077D390, 0x00666610), &Hook_oCNpc_OnDamage_Hit, HookMode::Patch);
	void __fastcall Hook_oCNpc_OnDamage_Hit(oCNpc* victim, void* vtable, oCNpc::oSDamageDescriptor& desc)
	{
#define ASSERTRETURN(cond) { if (!(cond)) return Ivk_oCNpc_OnDamage_Hit(victim, desc); }

		ASSERTRETURN(desc.pNpcAttacker);
		ASSERTRETURN(desc.pItemWeapon);
		ASSERTRETURN(!desc.pItemWeapon->HasFlag(ITM_CAT_MUN));
		ASSERTRETURN(!victim->HasFlag(NPC_FLAG_IMMORTAL));

		const int mode = desc.pNpcAttacker->GetWeaponMode();
		ASSERTRETURN(mode == NPC_WEAPON_DAG || mode == NPC_WEAPON_1HS || mode == NPC_WEAPON_2HS);

		const int hitchanceNr = (mode == NPC_WEAPON_2HS) ? NPC_HITCHANCE_2H : NPC_HITCHANCE_1H;
		float hitchance = desc.pNpcAttacker->ZENDEF2(GetTalentValue, GetHitChance)(hitchanceNr) / 100.0f / 5.0f;
		hitchance = CoerceInRange(hitchance, 0.0f, 0.0f, 1.0f);
		const bool hit = rand() / static_cast<float>(RAND_MAX) < hitchance;

		// arrange according to attribute scaling priority
		static const std::array<int, oEDamageIndex::oEDamageIndex_MAX> indexes =
		{
				oEDamageIndex_Edge,
				oEDamageIndex_Blunt,
				oEDamageIndex_Point,
				oEDamageIndex_Barrier,
				oEDamageIndex_Fire,
				oEDamageIndex_Fly,
				oEDamageIndex_Magic,
				oEDamageIndex_Fall,
		};

		bool scaled = false;
		bool immortal = true;
		int damage = 0;

		for (size_t i = 0; i < indexes.size(); i++)
		{
			const int index = indexes[i];
			const bool active = desc.enuModeDamage & (1 << index);
			const bool scaledNow = !scaled && i < 3 && active;

			if (scaledNow)
			{
				const int attribute = (index == oEDamageIndex_Point) ? NPC_ATR_DEXTERITY : NPC_ATR_STRENGTH;
				desc.aryDamage[index] += desc.pNpcAttacker->GetAttribute(attribute);
				scaled = true;
			}

			const int protection = victim->GetProtectionByIndex(static_cast<oEIndexDamage>(index));

			if (immortal && active && desc.aryDamage[index] > 0 && protection >= 0)
				immortal = false;

			if (immortal || protection < 0)
				continue;

			float multiplier = desc.fDamageMultiplier;

			if (scaledNow)
				multiplier *= hit ? 2.0f : 0.5f;

			const float floatDamage = (desc.aryDamage[index] - protection) * multiplier;
			const int intDamage = static_cast<int>(floatDamage + 0.5f);
			damage += std::max(intDamage, 0);
		}

		desc.fDamageTotal = 0.0f;
		desc.fDamageEffective = 0.0f;
		desc.fDamageReal = 0.0f;

		if (!immortal)
		{
			desc.fDamageTotal = damage;
			desc.fDamageEffective = damage;

			damage = std::max(damage, GetMinimalDamage());
			desc.fDamageReal = damage;

			victim->ChangeAttribute(NPC_ATR_HITPOINTS, -damage);
		}

		victim->hpHeal = victim->GetAttribute(NPC_ATR_REGENERATEHP) * 1000.0f;
		victim->manaHeal = victim->GetAttribute(NPC_ATR_REGENERATEMANA) * 1000.0f;

#undef ASSERTRETURN
	}
}
