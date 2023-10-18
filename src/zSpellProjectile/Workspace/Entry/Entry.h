namespace NAMESPACE
{
	void __fastcall Hook_oCVisualFX_SetCollisionEnabled(oCVisualFX*, void*, int);
	Hook<void(__thiscall*)(oCVisualFX*, int)> Ivk_oCVisualFX_SetCollisionEnabled(ZENFOR(0x00485640, 0x004907F0, 0x0048B9C0, 0x0048D330), &Hook_oCVisualFX_SetCollisionEnabled, HookMode::Patch);
	void __fastcall Hook_oCVisualFX_SetCollisionEnabled(oCVisualFX* _this, void* vtable, int a0)
	{
		if (a0 && (*Options::Spells & _this->GetSpellType()))
			_this->SetIsProjectile(true);

		Ivk_oCVisualFX_SetCollisionEnabled(_this, a0);
	}
}
