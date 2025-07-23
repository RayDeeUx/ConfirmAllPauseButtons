#include <Geode/modify/CCMenuItemSpriteExtra.hpp>

using namespace geode::prelude;

#define TITLE "Woah! Hold on there!"
#define BODY "Are you sure you want to press this?"

class QuickPopup : FLAlertLayer {}; // dummy class because i hate demangling class names manually

bool isButtonFromOwnMod(CCNode *node) {
	// shut the fuck up CLion and clang-tidy, this is NOT recursive
	if (!node || !node->getParent()) return false;
	if (typeinfo_cast<CCScene*>(node->getParent())) return false;
	if (const auto popup = typeinfo_cast<QuickPopup*>(node->getParent())) {
		CCLayer* mainLayer = typeinfo_cast<FLAlertLayer*>(popup)->m_mainLayer;
		const std::string& title = mainLayer->getChildByType<CCLabelBMFont>(0)->getString();
		if (const std::string& body = typeinfo_cast<CCLabelBMFont*>(mainLayer->getChildByType<TextArea>(0)->m_label->m_lines->objectAtIndex(0))->getString(); title == TITLE && body == BODY) return true;
		return false;
	}
	return isButtonFromOwnMod(node->getParent());
}

class $modify(MyCCMenuItemSpriteExtra, CCMenuItemSpriteExtra) {
	void activate() {
		if (!this->m_bEnabled) return;
		if (isButtonFromOwnMod(this)) return MyCCMenuItemSpriteExtra::runAsUsual();
		geode::createQuickPopup(TITLE, BODY, "No", "Yes", [this](auto, const bool isButtonTwo) {
			if (isButtonTwo) MyCCMenuItemSpriteExtra::runAsUsual();
		});
	}
	void runAsUsual() {
		if (m_pListener && m_pfnSelector) (m_pListener->*m_pfnSelector)(this);
		if (kScriptTypeNone != m_eScriptType) CCScriptEngineManager::sharedManager()->getScriptEngine()->executeMenuItemEvent(this);
	}
};