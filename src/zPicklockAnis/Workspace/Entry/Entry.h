#include <array>

namespace NAMESPACE
{
	bool ShouldPlayTryAni(oCNpc* npc, oCMobInter* mob)
	{
		if (npc->GetAnictrl()->walkmode == ANI_WALKMODE_SNEAK)
			return false;

		if (mob->IsOwnedByNpc(npc->GetInstance()))
			return true;

		if (mob->IsOwnedByGuild(npc->GetGuild()))
			return true;
		
		oCPortalRoomManager* const roomManager = ogame->GetPortalRoomManager();

		if (!roomManager)
			return true;

		const zSTRING* roomName = mob->GetSectorNameVobIsIn();

		if (!roomName)
			return true;

		const int roomIndex = roomManager->GetPortalRoomIndex(roomName);

		if (roomIndex == Invalid)
			return true;

		oCPortalRoom* const room = roomManager->portals[roomIndex];

		if (!room)
			return true;

		if (!room->ownerNpc.IsEmpty() && room->ownerNpc == npc->GetObjectName())
			return true;

		if (room->GetOwnerGuild() == Invalid)
			return true;

		if (room->GetOwnerGuild() == npc->GetGuild())
			return true;

		oCGuilds* const guilds = ogame->GetGuilds();

		if (!guilds)
			return true;

		if (guilds->GetAttitude(room->GetOwnerGuild(), npc->GetGuild()) == NPC_ATT_FRIENDLY)
			return true;

		return false;
	}

#define ASSERTRETURN(cond) { if (!(cond)) return; }

	void __fastcall Hook_oCMobInter_StopInteraction(oCMobInter* mob, void* vtable, oCNpc* npc)
	{
		if (ShouldPlayTryAni(npc, mob))
		{
			npc->GetEM()->OnMessage(new oCMsgConversation(oCMsgConversation::EV_PLAYANI_NOOVERLAY, "T_" + mob->GetScemeName() + "_S0_TRY"), npc);
			mob->SetSleeping(false);
			mob->GetModel()->StartAni(mob->GetModel()->GetAniIDFromAniName("T_S0_TRY"), zCModel::zMDL_STARTANI_DEFAULT);
		}
		else
		{
			mob->StopInteraction(npc);

			if (zCModelAniActive* ani = npc->GetModel()->GetActiveAni(npc->GetModel()->GetAniIDFromAniName(Z"T_" + mob->GetScemeName() + Z"_S0_2_STAND")))
				ani->nextAniOverride = npc->GetModel()->GetAniFromAniID(npc->GetModel()->GetAniIDFromAniName("T_RUN_2_SNEAK"));

			//npc->GetEM()->OnMessage(new oCMsgMovement(oCMsgMovement::EV_SETWALKMODE, npc->GetAnictrl()->walkmode), npc);
		}
	}

	Sub hookStopInter(ZSUB(GameEvent::Entry), []()
		{
			Unlocked<std::array<byte, 6>> ff = ZENDEF(0x006824C4, 0x006B1C06, 0x006C6166, 0x007241F6);
			ff = { 0x90, 0xE8 };
			*reinterpret_cast<int*>(ff.GetAddress() + 2) = reinterpret_cast<int>(&Hook_oCMobInter_StopInteraction) - ff.GetNextAddress();
		});

	class oCMobLockMsg : public oCMobMsg
	{
	public:
		zCLASS_UNION_DECLARATION(oCMobLockMsg);

	private:
		bool isDeleted = false;
		int npcAniId;

	public:
		enum TMobLockMsgSubType
		{
			EV_LOCK,
			EV_UNLOCK,
			EV_ENDINTERACTION
		};

		oCMobLockMsg()
		{
			from = 0;
		}

		oCMobLockMsg(TMobLockMsgSubType subType, oCNpc* npc) :
			oCMobMsg{ static_cast<TMobMsgSubType>(subType), npc }
		{
			from = 0;
			to = Invalid;
		}

		virtual int IsDeleted() override
		{
			return isDeleted;
		}

		virtual void Delete() override
		{
			isDeleted = true;
		}

		virtual int IsDeleteable() override
		{
			return true;
		}

		virtual int IsJob() override
		{
			return true;
		}

		virtual int IsOverlay() override
		{
			return false;
		}

		virtual int	MD_GetNumOfSubTypes() override
		{
			return 3;
		}

		virtual zSTRING	MD_GetSubTypeString(int subType)
		{
			switch (subType)
			{
			case EV_LOCK: return "EV_LOCK";
			case EV_UNLOCK: return "EV_UNLOCK";
			case EV_ENDINTERACTION: return "EV_ENDINTERACTION";
			default: return "";
			}
		}

		virtual void Pack(zCBuffer& buffer, zCEventManager* evManContext) override
		{
			oCMobMsg::Pack(buffer, evManContext);
			buffer.Write(&isDeleted, sizeof(isDeleted));
			buffer.Write(&npcAniId, sizeof(npcAniId));
		}

		virtual void Unpack(zCBuffer& buffer, zCEventManager* evManContext) override
		{
			oCMobMsg::Unpack(buffer, evManContext);
			buffer.Read(&isDeleted, sizeof(isDeleted));
			buffer.Read(&npcAniId, sizeof(npcAniId));
		}

		void SetNpcAniId(int id)
		{
			npcAniId = id;
		}

		int GetNpcAniId()
		{
			return npcAniId;
		}
	};

	zCLASS_UNION_DEFINITION(oCMobLockMsg, oCMobMsg, 0, 0);

	ActiveValue<bool> hookOnMessage;

	void __fastcall Hook_zCEventManager_OnMessage(zCEventManager*, void*, zCEventMessage*, zCVob*);
	Hook<void(__thiscall*)(zCEventManager*, zCEventMessage*, zCVob*), ActiveValue<bool>> Ivk_zCEventManager_OnMessage(ZENFOR(0x006DD090, 0x00715250, 0x00726940, 0x00786380), &Hook_zCEventManager_OnMessage, HookMode::Patch, hookOnMessage);
	void __fastcall Hook_zCEventManager_OnMessage(zCEventManager* eventMan, void* vtable, zCEventMessage* message, zCVob* sourceVob)
	{
		oCMobContainer* const chest = dynamic_cast<oCMobContainer*>(eventMan->hostVob);
		oCMobMsg* const mobMessage = dynamic_cast<oCMobMsg*>(message);
		oCNpc* const sourceNpc = dynamic_cast<oCNpc*>(sourceVob);

		if (chest && mobMessage && sourceNpc && mobMessage->GetSubType() == oCMobMsg::EV_STARTSTATECHANGE && mobMessage->from == 0 && mobMessage->to == 1)
			Ivk_zCEventManager_OnMessage(eventMan, new oCMobLockMsg{ oCMobLockMsg::EV_UNLOCK, sourceNpc }, sourceNpc);

		Ivk_zCEventManager_OnMessage(eventMan, message, sourceVob);
		hookOnMessage = false;
	}

	void __fastcall Hook_oCMobLockable_Interact(oCMobLockable*, void*, oCNpc*, int, int, int, int, int);
	Hook<void(__thiscall*)(oCMobLockable*, oCNpc*, int, int, int, int, int)> Ivk_oCMobLockable_Interact(ZENFOR(0x00681FF0, 0x006B16F0, 0x006C5C60, 0x00723CF0), &Hook_oCMobLockable_Interact, HookMode::Patch);
	void __fastcall Hook_oCMobLockable_Interact(oCMobLockable* chest, void* vtable, oCNpc* npc, int a1, int a2, int a3, int a4, int a5)
	{
		if (!chest->GetEM()->IsEmpty(false))
			return;

		hookOnMessage = chest->IsInState(npc, 0) && chest->locked;
		Ivk_oCMobLockable_Interact(chest, npc, a1, a2, a3, a4, a5);
		hookOnMessage = false;
	}

	void HandleLock(oCMobContainer* chest, oCMobLockMsg* message)
	{
		if (message->from == 0)
		{
			zSTRING chestAni;
			zSTRING npcAni;

			switch (message->GetSubType())
			{
			case oCMobLockMsg::EV_LOCK:
				chestAni = "T_S0_LOCK";
				npcAni = "T_" + chest->GetScemeName() + "_S0_LOCK";
				break;

			case oCMobLockMsg::EV_UNLOCK:
				chestAni = "T_S0_UNLOCK";
				npcAni = "T_" + chest->GetScemeName() + "_S0_UNLOCK";
				break;

			default:
				ASSERT(false);
				break;
			}

			message->to = chest->GetModel()->GetAniIDFromAniName(chestAni);

			if (message->to == Invalid)
			{
				message->Delete();
				return;
			}

			chest->GetModel()->StartAni(message->to, zCModel::zMDL_STARTANI_DEFAULT);
			message->SetNpcAniId(message->npc->GetModel()->GetAniIDFromAniName(npcAni));

			if (message->GetNpcAniId() != Invalid)
				message->npc->GetModel()->StartAni(message->GetNpcAniId(), zCModel::zMDL_STARTANI_DEFAULT);

			message->from = 1;
			return;
		}

		std::array activeAnis = { chest->GetModel()->GetActiveAni(message->to), message->npc->GetModel()->GetActiveAni(message->GetNpcAniId()) };

		for (zCModelAniActive* activeAni : activeAnis)
			if (activeAni && std::abs(activeAni->GetProgressPercent() - 1.0f) > 0.001f)
				return;

		switch (message->GetSubType())
		{
		case oCMobLockMsg::EV_LOCK:
			chest->locked = true;
			break;

		case oCMobLockMsg::EV_UNLOCK:
			chest->locked = false;
			break;
		}

		message->Delete();
	}

	void __fastcall Hook_oCMobContainer_OnMessage(oCMobContainer*, void*, zCEventMessage*, zCVob*);
	Hook<void(__thiscall*)(oCMobContainer*, zCEventMessage*, zCVob*)> Ivk_oCMobContainer_OnMessage(ZENFOR(0x00683BC0, 0x006B35E0, 0x006C7B20, 0x00725BB0), &Hook_oCMobContainer_OnMessage, HookMode::Patch);
	void __fastcall Hook_oCMobContainer_OnMessage(oCMobContainer* chest, void* vtable, zCEventMessage* message, zCVob* sourceVob)
	{
		oCMobLockMsg* lockMessage = dynamic_cast<oCMobLockMsg*>(message);

		if (!lockMessage)
			return Ivk_oCMobContainer_OnMessage(chest, message, sourceVob);

		switch (lockMessage->GetSubType())
		{
		case oCMobLockMsg::EV_LOCK:
		case oCMobLockMsg::EV_UNLOCK:
			return HandleLock(chest, lockMessage);

		default:
			ASSERT(false);
			return;
		}
	}

	int __fastcall IsDeleted(oCMobMsg* message, void* vftable)
	{
		return message->from == std::numeric_limits<int>::max();
	}

	void __fastcall Delete(oCMobMsg* message, void* vftable)
	{
		message->from = std::numeric_limits<int>::max();
	}

	Sub makeDeleatable(ZSUB(GameEvent::Entry), []()
		{
			auto& table = vftable_oCMobMsg::GetTable();
			table.names.ZENDEF2(f10_IsDeleted, f11_IsDeleted) = &IsDeleted;
			table.names.ZENDEF2(f08_Delete, f09_Delete) = &Delete;
		});

	Sub loadUnlocked(ZSUB(GameEvent::LoadEnd), []()
		{
			VobTraverser traverser;

			traverser.handle = [](zCVob* vob)
			{
				oCMobContainer* const chest = vob->CastTo<oCMobContainer>();
				ASSERTRETURN(chest && !chest->locked);

				zCModel* const model = chest->GetModel();
				ASSERTRETURN(model);

				const int aniId = model->GetAniIDFromAniName("T_S0_UNLOCK");
				ASSERTRETURN(aniId != Invalid);

				model->StartAni(aniId, zCModel::zMDL_STARTANI_DEFAULT);
				zCModelAniActive* const activeAni = model->GetActiveAni(aniId);
				ASSERTRETURN(activeAni && activeAni->protoAni);

				if (activeAni->advanceDir > 0)
				{
					activeAni->actFrame = activeAni->protoAni->numFrames - 1;
					activeAni->actAniEvent = activeAni->protoAni->numFrames;
				}
				else
				{
					activeAni->actFrame = 0;
					activeAni->actAniEvent = -1;
				}

				auto scope = AssignTemp(ztimer->factorMotion, 0.0f);
				model->AdvanceAnis();
			};

			traverser.TraverseVobList();
		});

	Sub sub(ZSUB(GameEvent::Loop), []
		{
			oCMobLockable* mob = player->GetFocusVob()->CastTo<oCMobLockable>();

			if (!mob)
				return;

			int y = 1000;
			LOGS(mob->objectName);
			LOGS(mob->sleepingMode);

			for (zCModelAni* ani : mob->GetModel()->modelProtoList[0]->protoAnis)
				LOGS(ani->aniName);
		});
}
