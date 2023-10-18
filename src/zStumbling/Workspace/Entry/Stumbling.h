#include <array>

namespace NAMESPACE
{
	float GetTenacity(oCNpc* npc, oEDamageIndex index)
	{
		// func int GetTenacity(var C_NPC npc, var int damageIndex);
		static int scriptFunc = parser->GetIndex("GETTENACITY");
		
		if (scriptFunc != -1)
			return static_cast<float>(CallParser<int>(parser, scriptFunc, npc, npc->GetProtectionByIndex(index)));

		if (npc->GetProtectionByIndex(index) < 0)
			return std::numeric_limits<float>::max();

		const float base = (npc->GetAttribute(NPC_ATR_STRENGTH) + npc->GetProtectionByIndex(index)) / 4.0f;
		float tenacity = base;
		
		if (npc->GetAttribute(NPC_ATR_HITPOINTS) < npc->GetAttribute(NPC_ATR_HITPOINTSMAX) / 2.0f)
			tenacity -= base / 2.0f;

		if (COA(npc, anictrl, model) && npc->anictrl->model->GetActiveAni(npc->anictrl->_t_hitfrun))
			tenacity += base / 2.0f;

		return tenacity;
	}

	bool allowStumbling = false;

	void __fastcall Hook_oCNpc_OnDamage_Anim(oCNpc*, void*, oCNpc::oSDamageDescriptor&);
	Hook<void(__thiscall*)(oCNpc*, oCNpc::oSDamageDescriptor&)> Ivk_oCNpc_OnDamage_Anim(ZENFOR(0x00741990, 0x00781E70, 0x0078C8F0, 0x00675BD0), &Hook_oCNpc_OnDamage_Anim, HookMode::Patch);
	void __fastcall Hook_oCNpc_OnDamage_Anim(oCNpc* _this, void* vtable, oCNpc::oSDamageDescriptor& desc)
	{
#define CHECK(cond) { if (!(cond)) return Ivk_oCNpc_OnDamage_Anim(_this, desc); }

		CHECK(desc.pVobAttacker);

		const oEDamageIndex index = static_cast<oEDamageIndex>(std::max_element(desc.aryDamage, desc.aryDamage + 8) - desc.aryDamage);
		//const float damage = desc.aryDamage[static_cast<int>(index)];
		const float damage = desc.fDamageReal;

		if (Options::Debug)
		{
			cmd << endl;
			LOG(damage);
		}

		const float tenacity = GetTenacity(_this, index);

		if (Options::Debug)
			LOG(tenacity);

		allowStumbling = damage > GetTenacity(_this, index);
		
		if (Options::Debug)
			LOG(allowStumbling);

		CHECK(allowStumbling);

		oCAniCtrl_Human* const anictrl = _this->GetAnictrl();
		CHECK(COA(anictrl, model));
		CHECK(anictrl->hitAniID != Invalid);
		anictrl->model->StopAni(anictrl->hitAniID);

		CHECK(false);

#undef CHECK
	}

	int __fastcall Hook_oCAIHuman_FightMelee(oCAIHuman*, void*);
	Hook<int(__thiscall*)(oCAIHuman*)> Ivk_oCAIHuman_FightMelee(ZENFOR(0x00611070, 0x00633DD0, 0x0063A630, 0x00696EC0), &Hook_oCAIHuman_FightMelee, HookMode::Patch);
	int __fastcall Hook_oCAIHuman_FightMelee(oCAIHuman* _this, void* vtable)
	{
#define CHECK(cond) if (!(cond)) return Ivk_oCAIHuman_FightMelee(_this);

		CHECK(_this->model);

		const int aniID = _this->model->GetAniIDFromAniName("T_GOTHIT");
		CHECK(aniID != Invalid);
		CHECK(_this->model->GetActiveAni(aniID));

		return true;

#undef CHECK
	}

	int __fastcall Hook_oCNpc_IsAPlayer(oCNpc* _this, void* vtable)
	{
		return !allowStumbling;
	}

	Sub patch(ZSUB(GameEvent::Entry), []()
		{
			{
				// hook first IsAPlayer
				const int address = 0x00678032;
				Unlocked<std::array<byte, 6>> call = address;
				(*call)[0] = 0xE8;
				reinterpret_cast<int&>((*call)[1]) = reinterpret_cast<int>(&Hook_oCNpc_IsAPlayer) - (address + 5);
				(*call)[5] = 0x90;
			}

			{
				// set IsInPrehit and IsInCombo to false
				const int address = 0x0067803C;
				Unlocked<std::array<byte, 5>> jump = address;
				(*jump) = { 0xE9 };
				reinterpret_cast<int&>((*jump)[1]) = 0x00678BDA - (address + 5);
			}

			{
				// playerHitDuringAttack = false
				Unlocked<byte> jump = 0x00678378;
				jump = 0xEB;
			}

			{
				// monsterNoStumble (set IsAPlayer == true)
				Unlocked<std::array<byte, 2>> jump = 0x006783CB;
				std::fill_n(jump->data(), 2, 0x90);
			}

			{
				// OnDamage_Script no interrupt
				const int address = 0x0066E862;
				Unlocked<byte> inst = address;
				Unlocked<int> operand = address + 1;
				inst = 0xE9;
				operand = 0x0066E893 - (address + 5);
			}
		});
}
