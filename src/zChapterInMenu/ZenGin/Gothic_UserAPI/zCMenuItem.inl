// Supported with union (c) 2020 Union team

// User API for zCMenuItem
// Add your methods here

#if ENGINE <= Engine_G1A

void Release()
{
	m_iRefCtr -= 1;

	if (m_iRefCtr <= 0 && !registeredCPP)
		delete this;
}

#endif