/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "MailBoxCore.h"

#include <game/server/gamecontext.h>

using namespace sqlstr;

bool CMailBoxCore::OnHandleVoteCommands(CPlayer* pPlayer, const char* CMD, int VoteID, int VoteID2, int Get, const char* GetText)
{
	const int ClientID = pPlayer->GetCID();
	if(PPSTR(CMD, "MAIL") == 0)
	{
		AcceptMailLetter(pPlayer, VoteID);
		GS()->StrongUpdateVotes(ClientID, MenuList::MENU_INBOX);
		return true;
	}

	if(PPSTR(CMD, "DELETE_MAIL") == 0)
	{
		DeleteMailLetter(VoteID);
		GS()->StrongUpdateVotes(ClientID, MenuList::MENU_INBOX);
		return true;
	}

	return false;
}

bool CMailBoxCore::OnHandleMenulist(CPlayer* pPlayer, int Menulist, bool ReplaceMenu)
{
	const int ClientID = pPlayer->GetCID();
	if(ReplaceMenu)
		return false;

	if(Menulist == MenuList::MENU_INBOX)
	{
		pPlayer->m_LastVoteMenu = MenuList::MENU_MAIN;

		GS()->AddVotesBackpage(ClientID);
		GS()->AV(ClientID, "null");
		GetInformationInbox(pPlayer);
		return true;
	}

	return false;
}

// check whether messages are available
int CMailBoxCore::GetMailLettersSize(int AccountID)
{
	ResultPtr pRes = Database->Execute<DB::SELECT>("ID", "tw_accounts_mailbox", "WHERE UserID = '%d'", AccountID);
	const int MailValue = pRes->rowsCount();
	return MailValue;
}

// show a list of mails
void CMailBoxCore::GetInformationInbox(CPlayer *pPlayer)
{
	int ShowLetterID = 0;
	bool EmptyMailBox = true;
	const int ClientID = pPlayer->GetCID();
	int HideID = (int)(NUM_TAB_MENU + CItemDescription::Data().size() + 200);
	ResultPtr pRes = Database->Execute<DB::SELECT>("*", "tw_accounts_mailbox", "WHERE UserID = '%d' LIMIT %d", pPlayer->Acc().m_UserID, MAILLETTER_MAX_CAPACITY);
	while(pRes->next())
	{
		// get the information to create an object
		const int MailLetterID = pRes->getInt("ID");
		const int ItemID = pRes->getInt("ItemID");
		const int ItemValue = pRes->getInt("ItemValue");
		const int Enchant = pRes->getInt("Enchant");
		EmptyMailBox = false;
		ShowLetterID++;
		HideID++;

		// add vote menu
		GS()->AVH(ClientID, HideID, "✉ Letter({INT}) {STR}", ShowLetterID, pRes->getString("Name").c_str());
		GS()->AVM(ClientID, "null", NOPE, HideID, "{STR}", pRes->getString("Description").c_str());
		if(ItemID <= 0 || ItemValue <= 0)
			GS()->AVM(ClientID, "MAIL", MailLetterID, HideID, "Accept (L{INT})", ShowLetterID);
		else 
		{
			CItemDescription* pItemAttach = GS()->GetItemInfo(ItemID);
			if(pItemAttach->IsEnchantable())
			{
				GS()->AVM(ClientID, "MAIL", MailLetterID, HideID, "Receive {STR} {STR} (L{INT})",
					pItemAttach->GetName(), pItemAttach->StringEnchantLevel(Enchant).c_str(), ShowLetterID);
			}
			else
				GS()->AVM(ClientID, "MAIL", MailLetterID, HideID, "Receive {STR}x{VAL} (L{INT})", pItemAttach->GetName(), ItemValue, ShowLetterID);
		}

		GS()->AVM(ClientID, "DELETE_MAIL", MailLetterID, HideID, "Delete (L{INT})", ShowLetterID);
	}

	if(EmptyMailBox)
		GS()->AVL(ClientID, "null", "Your mailbox is empty");
}

// sending a mail to a player
void CMailBoxCore::SendInbox(const char* pFrom, int AccountID, const char* pName, const char* pDesc, int ItemID, int Value, int Enchant)
{
	// clear str and connection
	const CSqlString<64> cName = CSqlString<64>(pName);
	const CSqlString<64> cDesc = CSqlString<64>(pDesc);
	const CSqlString<64> cFrom = CSqlString<64>(pFrom);

	// send information about new message
	if(GS()->ChatAccount(AccountID, "[Mailbox] New letter ({STR})!", cName.cstr()))
	{
		const int MailLettersSize = GetMailLettersSize(AccountID);
		if(MailLettersSize >= (int)MAILLETTER_MAX_CAPACITY)
		{
			GS()->ChatAccount(AccountID, "[Mailbox] Your mailbox is full you can't get.");
			GS()->ChatAccount(AccountID, "[Mailbox] It will come after you clear your mailbox.");
		}
	}

	// send new message
	if (ItemID <= 0)
	{
		Database->Execute<DB::INSERT>("tw_accounts_mailbox", "(Name, Description, UserID, FromSend) VALUES ('%s', '%s', '%d', '%s');", cName.cstr(), cDesc.cstr(), AccountID, cFrom.cstr());
		return;
	}
	Database->Execute<DB::INSERT>("tw_accounts_mailbox", "(Name, Description, ItemID, ItemValue, Enchant, UserID, FromSend) VALUES ('%s', '%s', '%d', '%d', '%d', '%d', '%s');",
		 cName.cstr(), cDesc.cstr(), ItemID, Value, Enchant, AccountID, cFrom.cstr());
}

bool CMailBoxCore::SendInbox(const char* pFrom, const char* pNickname, const char* pName, const char* pDesc, int ItemID, int Value, int Enchant)
{
	const CSqlString<64> cName = CSqlString<64>(pNickname);
	ResultPtr pRes = Database->Execute<DB::SELECT>("ID, Nick", "tw_accounts_data", "WHERE Nick = '%s'", cName.cstr());
	if(pRes->next())
	{
		const int AccountID = pRes->getInt("ID");
		SendInbox(pFrom, AccountID, pName, pDesc, ItemID, Value, Enchant);
		return true;
	}
	return false;
}

void CMailBoxCore::AcceptMailLetter(CPlayer* pPlayer, int MailLetterID)
{
	ResultPtr pRes = Database->Execute<DB::SELECT>("ItemID, ItemValue, Enchant", "tw_accounts_mailbox", "WHERE ID = '%d'", MailLetterID);
	if(pRes->next())
	{
		// get informed about the mail
		const int ItemID = pRes->getInt("ItemID");
		const int ItemValue = pRes->getInt("ItemValue");
		if(ItemID <= 0 || ItemValue <= 0)
		{
			DeleteMailLetter(MailLetterID);
			return;
		}

		// recieve
		if(GS()->GetItemInfo(ItemID)->IsEnchantable() && pPlayer->GetItem(ItemID)->HasItem())
		{
			GS()->Chat(pPlayer->GetCID(), "Enchant item maximal count x1 in a backpack!");
			return;
		}

		const int Enchant = pRes->getInt("Enchant");
		pPlayer->GetItem(ItemID)->Add(ItemValue, 0, Enchant);
		GS()->Chat(pPlayer->GetCID(), "You received an attached item [{STR}].", GS()->GetItemInfo(ItemID)->GetName());
		DeleteMailLetter(MailLetterID);
	}
}

void CMailBoxCore::DeleteMailLetter(int MailLetterID)
{
	Database->Execute<DB::REMOVE>("tw_accounts_mailbox", "WHERE ID = '%d'", MailLetterID);
}

void CMailBoxCore::SetReadState(int MailLetterID, bool State)
{
	Database->Execute<DB::UPDATE>("tw_accounts_mailbox", "IsRead='%d' WHERE ID = '%d'", State, MailLetterID);
}