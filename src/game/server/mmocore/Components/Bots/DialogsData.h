/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_COMPONENT_DIALOGS_DATA_H
#define GAME_SERVER_COMPONENT_DIALOGS_DATA_H

/************************************************************************/
/*  Dialog struct (reworked 01.03.2023)                                 */
/************************************************************************/
class CPlayerDialog
{
	friend class CDialogElem;
	class CGS* GS() const;
	class CPlayer* m_pPlayer {};

	int m_BotCID{};
	int m_MobID{};
	int m_BotType{};
	int m_Step{};
	char m_aFormatedText[1024] {};

public:
	CPlayerDialog() { Clear(); }
	bool IsActive() const { return m_BotCID > 0; }
	void Start(class CPlayer* pPlayer, int BotCID);
	void Next();
	void TickUpdate();

private:
	void FormatText(class CDialogElem* pDialog, const char* pLeftNickname, const char* pRightNickname);
	const char* GetCurrentText() const { return m_aFormatedText; }
	void ClearText();

	class CDialogElem* GetCurrent() const;
	void DialogEvents() const;
	void EndDialogEvents() const;
	void PostNext();
	void Clear();
};

class CDialogElem
{
	int m_LeftSide{};
	int m_RightSide{};

	std::string m_Text {};

	bool m_Request{};
	int64 m_Flags{};

	int GetLeftSide() const { return m_LeftSide; }
	int GetRightSide() const { return m_RightSide; }

	int GetClientIDByBotID(class CGS* pGS, int CheckVisibleForCID, int BotID) const;

public:
	void Init(int BotID, std::string Text, bool Request);
	void Show(class CGS* pGS, int ClientID);

	const char* GetText() const { return m_Text.c_str(); }
	bool IsEmptyDialog() const { return m_Text.empty(); }
	bool IsRequestAction() const { return m_Request; }
	int64 GetFlag() const { return m_Flags; }
};

#endif
