namespace NAMESPACE
{
	enum class ScriptEvent
	{
		Dialog = 0,
		Equip,
		UseItem,
		NewGame
	};

	class KirScope
	{
	private:
		static KirScope* instance;

		const ScriptEvent event;
		ParserScope scope;

	public:
		static void RaiseEvent(ScriptEvent event, bool end)
		{
			if (!player)
				return;

			static Symbol func{ parser, "zKirTheSeeker_OnEvent" };

			if (func)
				CallParser<void>(func.GetParser(), func.GetIndex(), static_cast<int>(event), end);
		}

		static void TryCreate(std::optional<KirScope>& scope, ScriptEvent event)
		{
			if (!player || instance)
				return;

			scope.emplace(event);
		}
		
		KirScope(ScriptEvent event) :
			event{ event },
			scope{ parser }
		{
			instance = this;
			RaiseEvent(event, false);
		}

		~KirScope()
		{
			RaiseEvent(event, true);
			instance = nullptr;
		}
	};

	KirScope* KirScope::instance = nullptr;

	void __fastcall Hook_oCInformationManager_OnInfo(oCInformationManager*, oCInfo*);
	Hook<void(__fastcall*)(oCInformationManager*, oCInfo*)> Ivk_oCInformationManager_OnInfo(ZENFOR(0x0072D560, 0x0076BB30, 0x00779070, 0x00662290), &Hook_oCInformationManager_OnInfo, HookMode::Patch);
	void __fastcall Hook_oCInformationManager_OnInfo(oCInformationManager* _this, oCInfo* a0)
	{
		std::optional<KirScope> scope;
		KirScope::TryCreate(scope, ScriptEvent::Dialog);
		Ivk_oCInformationManager_OnInfo(_this, a0);
	}

	void __fastcall Hook_oCInformationManager_OnChoice(oCInformationManager*, oCInfoChoice*);
	Hook<void(__fastcall*)(oCInformationManager*, oCInfoChoice*)> Ivk_oCInformationManager_OnChoice(ZENFOR(0x0072DC70, 0x0076C2A0, 0x00779780, 0x006629A0), &Hook_oCInformationManager_OnChoice, HookMode::Patch);
	void __fastcall Hook_oCInformationManager_OnChoice(oCInformationManager* _this, oCInfoChoice* a0)
	{
		std::optional<KirScope> scope;
		KirScope::TryCreate(scope, ScriptEvent::Dialog);
		Ivk_oCInformationManager_OnChoice(_this, a0);
	}

	void __fastcall Hook_oCNpc_AddItemEffects(oCNpc*, void*, oCItem*);
	Hook<void(__thiscall*)(oCNpc*, oCItem*)> Ivk_oCNpc_AddItemEffects(ZENFOR(0x0068F640, 0x006C02E0, 0x006D3C40, 0x007320F0), &Hook_oCNpc_AddItemEffects, HookMode::Patch);
	void __fastcall Hook_oCNpc_AddItemEffects(oCNpc* _this, void* vtable, oCItem* a0)
	{
		std::optional<KirScope> scope;
		KirScope::TryCreate(scope, ScriptEvent::Equip);
		Ivk_oCNpc_AddItemEffects(_this, a0);
	}

	void __fastcall Hook_oCNpc_RemoveItemEffects(oCNpc*, void*, oCItem*);
	Hook<void(__thiscall*)(oCNpc*, oCItem*)> Ivk_oCNpc_RemoveItemEffects(ZENFOR(0x0068F7D0, 0x006C0470, 0x006D3DC0, 0x00732270), &Hook_oCNpc_RemoveItemEffects, HookMode::Patch);
	void __fastcall Hook_oCNpc_RemoveItemEffects(oCNpc* _this, void* vtable, oCItem* a0)
	{
		std::optional<KirScope> scope;
		KirScope::TryCreate(scope, ScriptEvent::Equip);
		Ivk_oCNpc_RemoveItemEffects(_this, a0);
	}

	int __fastcall Hook_oCNpc_EV_UseItemToState(oCNpc*, void*, oCMsgManipulate*);
	Hook<int(__thiscall*)(oCNpc*, oCMsgManipulate*)> Ivk_oCNpc_EV_UseItemToState(ZENFOR(0x006AFC70, 0x006E3AF0, 0x006F6AC0, 0x007558F0), &Hook_oCNpc_EV_UseItemToState, HookMode::Patch);
	int __fastcall Hook_oCNpc_EV_UseItemToState(oCNpc* _this, void* vtable, oCMsgManipulate* a0)
	{
		std::optional<KirScope> scope;

		if (_this == player)
			KirScope::TryCreate(scope, ScriptEvent::UseItem);

		return Ivk_oCNpc_EV_UseItemToState(_this, a0);
	}

	Sub onNewGame(ZSUB(GameEvent::LoadEnd_NewGame), []()
		{
			KirScope::RaiseEvent(ScriptEvent::NewGame, true);
		});
}
