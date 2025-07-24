#include <Geode/modify/CCMenuItemSpriteExtra.hpp>
#include <Geode/modify/CCMenuItemToggler.hpp>

using namespace geode::prelude;

bool applyEverywhere = false;
bool hasNodeIDs = false;

#define NODE_IDS "geode.node-ids"
#define TITLE "Woah! Hold on there!"
#define BODY "Are you sure you want to press this?"
#define TITLE_NODE_ID "title"
#define TEXTAREA_NDID "content-text-area"
#define TITLE_NDCHILD mainLayer->getChildByID(TITLE_NODE_ID)
#define TEXTAREA_CHLD mainLayer->getChildByID(TEXTAREA_NDID)
#define CONFIRMATION_POPUP_NODE_ID "confirmation-popup"_spr

bool isButtonFromOwnMod(CCMenuItem* button) {
	// button -> parent CCMenu* -> parent CCLayer* -> parent QuickPopup* / FLAlertLayer*
	if (!button) return false;
	CCNode* parent =  button->getParent();
	if (!parent) return false;
	CCNode* grandparent = parent->getParent();
	if (!grandparent) return false;
	if (CCNode* greatgrandparent = grandparent->getParent()) {
		const std::string& nodeID = greatgrandparent->getID();
		if (nodeID.empty()) return false;
		return nodeID == CONFIRMATION_POPUP_NODE_ID;
	}
	return false;
}

bool isFromPauseMenu(CCMenuItem* button) {
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

void createPopupWithCallback(CCMenuItem* item, const std::function<void()>& callback) {
	if (!item->m_bEnabled) return;
	if (!applyEverywhere && !isFromPauseMenu(item)) return callback();
	if (isButtonFromOwnMod(item)) return callback();
	FLAlertLayer* popup = geode::createQuickPopup(TITLE, BODY, "No", "Yes", 420.f, [callback](auto, const bool isButtonTwo) {
		if (isButtonTwo) callback();
	}, true, true);
	popup->setID(CONFIRMATION_POPUP_NODE_ID);
}

void cocosMenuItemRunAsUsual(CCMenuItem* item) {
	// based on IDA and Ghidra decomps :P
	if (item->m_pListener && item->m_pfnSelector) (item->m_pListener->*item->m_pfnSelector)(item);
	if (item->m_eScriptType != ccScriptType::kScriptTypeNone) CCScriptEngineManager::sharedManager()->getScriptEngine()->executeMenuItemEvent(item);
}

class $modify(MyCCMenuItemSpriteExtra, CCMenuItemSpriteExtra) {
	void activate() {
		// this is definitely overkill but idc this aint going onto the index anyway
		createPopupWithCallback(this, [this] {
			MyCCMenuItemSpriteExtra::runAsUsual();
		});
	}
	void runAsUsual() {
		// based on IDA and Ghidra decomps :P
		this->stopAllActions();
		if (this->m_animationType == MenuAnimationType::Scale) this->setScale(this->m_baseScale);
		if (!std::string(this->m_activateSound).empty() && this->m_volume > 0.f) {
			FMODAudioEngine::sharedEngine()->playEffect(m_activateSound, 1.f, 0.f, this->m_volume);
		}
		cocosMenuItemRunAsUsual(this);
	}
};

class $modify(MyCCMenuItemToggler, CCMenuItemToggler) {
	void activate() {
		// this is definitely overkill but idc this aint going onto the index anyway
		createPopupWithCallback(this, [this] {
			MyCCMenuItemToggler::runAsUsual();
		});
	}
	void runAsUsual() {
		cocosMenuItemRunAsUsual(this);
	}
};

$on_mod(Loaded) {
	hasNodeIDs = Loader::get()->isModLoaded(NODE_IDS);
	applyEverywhere = Mod::get()->getSettingValue<bool>("applyEverywhere");
	listenForSettingChanges("applyEverywhere", [](const bool newApplyEverywhere) {
		applyEverywhere = newApplyEverywhere;
	});
}