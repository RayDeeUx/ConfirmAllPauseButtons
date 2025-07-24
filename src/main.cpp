#include <Geode/modify/CCMenuItemSpriteExtra.hpp>

using namespace geode::prelude;

long long epoch = std::chrono::system_clock::now().time_since_epoch().count();

#define TITLE "Woah! Hold on there!"
#define BODY "Are you sure you want to press this?"
#define TITLE_NODE_ID "title"
#define TEXTAREA_NDID "content-text-area"
#define TITLE_NDCHILD mainLayer->getChildByID(TITLE_NODE_ID)
#define TEXTAREA_CHLD mainLayer->getChildByID(TEXTAREA_NDID)

bool isButtonFromOwnMod(CCNode *node) {
	// shut the fuck up CLion and clang-tidy, this is NOT recursive [anymore!]
	if (!node || !node->getParent()) return false;
	if (!utils::string::startsWith(node->getParent()->getID(), "confirmation-popup-"_spr)) return isButtonFromOwnMod(node->getParent());
	if (typeinfo_cast<CCScene*>(node->getParent())) return false;
	CCLayer* mainLayer = static_cast<FLAlertLayer*>(node->getParent())->m_mainLayer;
	const TextArea* textArea = TEXTAREA_CHLD ? static_cast<TextArea*>(TEXTAREA_CHLD) : mainLayer->getChildByType<TextArea>(0);
	if (!textArea || !textArea->m_label || !textArea->m_label->m_lines || textArea->m_label->m_lines->count() < 1 || !textArea->m_label->m_lines->objectAtIndex(0)) return false;
	CCLabelBMFont* titleLabel = TITLE_NDCHILD ? static_cast<CCLabelBMFont*>(TITLE_NDCHILD) : mainLayer->getChildByType<CCLabelBMFont>(0);
	const auto firstLine = static_cast<CCLabelBMFont*>(textArea->m_label->m_lines->objectAtIndex(0));
	const std::string& titleString = titleLabel->getString();
	if (const std::string& firstString = firstLine->getString(); titleString == TITLE && firstString == BODY) return true;
	return false;
}

class $modify(MyCCMenuItemSpriteExtra, CCMenuItemSpriteExtra) {
	void activate() {
		if (!this->m_bEnabled) return;
		if (isButtonFromOwnMod(this)) return MyCCMenuItemSpriteExtra::runAsUsual();
		epoch = std::chrono::system_clock::now().time_since_epoch().count();
		FLAlertLayer* popup = geode::createQuickPopup(TITLE, BODY, "No", "Yes", 420.f, [this](auto, const bool isButtonTwo) {
			if (isButtonTwo) MyCCMenuItemSpriteExtra::runAsUsual();
		}, false, false);
		popup->setID(fmt::format("confirmation-popup-{}"_spr, epoch));
		popup->show();
	}
	void runAsUsual() {
		if (m_pListener && m_pfnSelector) (m_pListener->*m_pfnSelector)(this);
		if (kScriptTypeNone != m_eScriptType) CCScriptEngineManager::sharedManager()->getScriptEngine()->executeMenuItemEvent(this);
	}
};