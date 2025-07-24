#include <Geode/modify/CCMenuItemSpriteExtra.hpp>

using namespace geode::prelude;

long long epoch = std::chrono::system_clock::now().time_since_epoch().count();
bool applyEverywhere = false;
bool hasNodeIDs = false;

#define NODE_IDS "geode.node-ids"
#define TITLE "Woah! Hold on there!"
#define BODY "Are you sure you want to press this?"
#define TITLE_NODE_ID "title"
#define TEXTAREA_NDID "content-text-area"
#define TITLE_NDCHILD mainLayer->getChildByID(TITLE_NODE_ID)
#define TEXTAREA_CHLD mainLayer->getChildByID(TEXTAREA_NDID)

bool isButtonFromOwnMod(CCMenuItemSpriteExtra* button) {
	// button -> parent CCMenu* -> parent CCLayer* -> parent QuickPopup* / FLAlertLayer*
	if (!button) return false;
	CCNode* parent =  button->getParent();
	if (!parent) return false;
	CCNode* grandparent = parent->getParent();
	if (!grandparent) return false;
	if (CCNode* greatgrandparent = grandparent->getParent()) {
		const std::string& nodeID = greatgrandparent->getID();
		if (nodeID.empty()) return false;
		return utils::string::startsWith(nodeID, "confirmation-popup-"_spr);
	}
	return false;
}

bool isFromPauseMenu(CCMenuItemSpriteExtra* button) {
	// button -> parent CCMenu* -> parent PauseLayer*
	if (hasNodeIDs) {
		if (!CCScene::get()->getChildByID("PauseLayer")) return false;
	} else {
		if (!CCScene::get()->getChildByType<PauseLayer*>(0)) return false;
	}
	if (!button) return false;
	CCNode* parent = button->getParent();
	if (!parent) return false;
	if (CCNode* grandparent = parent->getParent()) {
		if (grandparent->getID() == "PauseLayer") return true;
		if (hasNodeIDs) return false;
		return typeinfo_cast<PauseLayer*>(grandparent);
	}
	return false;
}

class $modify(MyCCMenuItemSpriteExtra, CCMenuItemSpriteExtra) {
	void activate() {
		if (!this->m_bEnabled) return;
		if (!applyEverywhere && !isFromPauseMenu(this)) return MyCCMenuItemSpriteExtra::runAsUsual();
		if (isButtonFromOwnMod(this)) return MyCCMenuItemSpriteExtra::runAsUsual();
		// this is definitely overkill but idc this aint going onto the index anyway
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

$on_mod(Loaded) {
	hasNodeIDs = Loader::get()->isModLoaded(NODE_IDS);
	applyEverywhere = Mod::get()->getSettingValue<bool>("applyEverywhere");
	listenForSettingChanges("applyEverywhere", [](const bool newApplyEverywhere) {
		applyEverywhere = newApplyEverywhere;
	});
}