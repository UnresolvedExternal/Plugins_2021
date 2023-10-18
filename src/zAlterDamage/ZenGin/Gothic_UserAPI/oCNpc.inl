// Supported with union (c) 2020 Union team

// User API for oCNpc
// Add your methods here

int EV_DamagePerFrame_Union(oCMsgDamage* message);
void OnDamage_Union(oSDamageDescriptor& desc);
void OnDamage_Hit_Union(oSDamageDescriptor& desc);
void OnDamage_Condition_Union(oSDamageDescriptor& desc);
void OnDamage_Anim_Union(oSDamageDescriptor& desc);
void OnDamage_CreateBlood(oSDamageDescriptor& desc);
void OnDamage_Effects_Start_Union(oSDamageDescriptor& desc);
void OnDamage_Effects_End_Union(oSDamageDescriptor& desc);
void OnDamage_Script_Union(oSDamageDescriptor& desc);
void OnDamage_State_Union(oSDamageDescriptor& desc);
void OnDamage_Events_Union(oSDamageDescriptor& desc);
void OnDamage_Sound_Union(oSDamageDescriptor& desc);