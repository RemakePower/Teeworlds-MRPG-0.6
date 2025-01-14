/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_COMPONENT_SKILL_DATA_INFO_H
#define GAME_SERVER_COMPONENT_SKILL_DATA_INFO_H

enum SkillType
{
	SKILL_TYPE_IMPROVEMENTS,
	SKILL_TYPE_HEALER,
	SKILL_TYPE_DPS,
	SKILL_TYPE_TANK,
	NUM_SKILL_TYPES,
};

using SkillIdentifier = int;

class CSkillDescription : public MultiworldIdentifiableStaticData< std::map < SkillIdentifier, CSkillDescription > >
{
	friend class CSkillsCore;

	SkillIdentifier m_ID{};
	char m_aName[32]{};
	char m_aDescription[64]{};
	char m_aBoostName[64]{};
	int m_BoostDefault{};
	SkillType m_Type{};
	int m_PercentageCost{};
	int m_PriceSP{};
	int m_MaxLevel{};
	bool m_Passive{};

public:
	CSkillDescription() = default;
	CSkillDescription(SkillIdentifier ID) : m_ID(ID) {}

	void Init(const std::string& Name, const std::string& Description, const std::string& BonusName, int BonusDefault, SkillType Type, int PercentageCost, int PriceSP, int MaxLevel, bool Passive)
	{
		str_copy(m_aName, Name.c_str(), sizeof(m_aName));
		str_copy(m_aDescription, Description.c_str(), sizeof(m_aDescription));
		str_copy(m_aBoostName, BonusName.c_str(), sizeof(m_aBoostName));
		m_BoostDefault = BonusDefault;
		m_Type = Type;
		m_PercentageCost = PercentageCost;
		m_PriceSP = PriceSP;
		m_MaxLevel = MaxLevel;
		m_Passive = Passive;
		CSkillDescription::m_pData[m_ID] = *this;
	}

	SkillIdentifier GetID() const { return m_ID; }

	const char* GetName() const { return m_aName; }
	const char* GetDescription() const { return m_aDescription; }
	const char* GetBoostName() const { return m_aBoostName; }
	int GetBoostDefault() const { return m_BoostDefault; }
	SkillType GetType() const { return m_Type; }
	int GetPercentageCost() const { return m_PercentageCost; }
	int GetPriceSP() const { return m_PriceSP; }
	int GetMaxLevel() const { return m_MaxLevel; }
	bool IsPassive() const { return m_Passive; }

	static const char* GetEmoticonName(int EmoticionID);
};

#endif
