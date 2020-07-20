/////////////////////////////////////////////////////////////////////
//
// client tech bits
//
// -Yorvik
//
/////////////////////////////////////////////////////////////////////

#ifndef CTF_TECH_H_DEFINED
#define CTF_TECH_H_DEFINED

namespace CTF{
	void SetTechIconShader(int id, const char* iconshadername);
	void UpdateTechs();
	void DrawTechIcons();

	const char* GetTechIcon(int id);
} //namespace CTF

extern char* ctf_techicon;

#endif //CTF_TECH_H_DEFINED