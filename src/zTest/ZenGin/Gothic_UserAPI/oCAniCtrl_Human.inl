// Supported with union (c) 2020 Union team

// User API for oCAniCtrl_Human
// Add your methods here

void _Stand_Union();
inline void SetState(int state) { this->state = (zCAIPlayer::zTMovementState)state; }