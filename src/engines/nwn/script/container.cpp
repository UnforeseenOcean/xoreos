/* xoreos - A reimplementation of BioWare's Aurora engine
 *
 * xoreos is the legal property of its developers, whose names
 * can be found in the AUTHORS file distributed with this source
 * distribution.
 *
 * xoreos is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * xoreos is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xoreos. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file
 *  An object containing scripts.
 */

#include <cassert>

#include "src/common/util.h"
#include "src/common/error.h"

#include "src/aurora/gff3file.h"

#include "src/aurora/nwscript/types.h"
#include "src/aurora/nwscript/variable.h"
#include "src/aurora/nwscript/ncsfile.h"

#include "src/engines/nwn/script/container.h"

namespace Engines {

namespace NWN {

struct ScriptName {
	Script script;
	const char *name;
};

static const ScriptName kScriptNames[] = {
	{kScriptAcquireItem      , "Mod_OnAcquirItem"},
	{kScriptUnacquireItem    , "Mod_OnUnAqreItem"},
	{kScriptActivateItem     , "Mod_OnActvtItem" },
	{kScriptEnter            , "Mod_OnClientEntr"},
	{kScriptEnter            , "OnEnter"         },
	{kScriptEnter            , "ScriptOnEnter"   },
	{kScriptExit             , "Mod_OnClientLeav"},
	{kScriptExit             , "OnExit"          },
	{kScriptExit             , "ScriptOnExit"    },
	{kScriptCutsceneAbort    , "Mod_OnCutsnAbort"},
	{kScriptHeartbeat        , "Mod_OnHeartbeat" },
	{kScriptHeartbeat        , "OnHeartbeat"     },
	{kScriptHeartbeat        , "ScriptHeartbeat" },
	{kScriptModuleLoad       , "Mod_OnModLoad"   },
	{kScriptModuleStart      , "Mod_OnModStart"  },
	{kScriptPlayerChat       , "Mod_OnPlrChat"   },
	{kScriptPlayerDeath      , "Mod_OnPlrDeath"  },
	{kScriptPlayerDying      , "Mod_OnPlrDying"  },
	{kScriptPlayerEquipItem  , "Mod_OnPlrEqItm"  },
	{kScriptPlayerUnequipItem, "Mod_OnPlrUnEqItm"},
	{kScriptPlayerLevelUp    , "Mod_OnPlrLvlUp"  },
	{kScriptPlayerRest       , "Mod_OnPlrRest"   },
	{kScriptPlayerRespawn    , "Mod_OnSpawnBtnDn"},
	{kScriptUserdefined      , "Mod_OnUsrDefined"},
	{kScriptUserdefined      , "OnUserDefined"   },
	{kScriptUserdefined      , "ScriptUserDefine"},
	{kScriptUsed             , "OnUsed"          },
	{kScriptClick            , "OnClick"         },
	{kScriptOpen             , "OnOpen"          },
	{kScriptClosed           , "OnClosed"        },
	{kScriptDamaged          , "OnDamaged"       },
	{kScriptDamaged          , "ScriptDamaged"   },
	{kScriptDeath            , "OnDeath"         },
	{kScriptDeath            , "ScriptDeath"     },
	{kScriptDisarm           , "OnDisarm"        },
	{kScriptLock             , "OnLock"          },
	{kScriptUnlock           , "OnUnlock"        },
	{kScriptAttacked         , "OnMeleeAttacked" },
	{kScriptAttacked         , "ScriptAttacked"  },
	{kScriptSpellCastAt      , "OnSpellCastAt"   },
	{kScriptSpellCastAt      , "ScriptSpellAt"   },
	{kScriptTrapTriggered    , "OnTrapTriggered" },
	{kScriptDialogue         , "ScriptDialogue"  },
	{kScriptDisturbed        , "ScriptDisturbed" },
	{kScriptEndRound         , "ScriptEndRound"  },
	{kScriptBlocked          , "ScriptOnBlocked" },
	{kScriptNotice           , "ScriptOnNotice"  },
	{kScriptRested           , "ScriptRested"    },
	{kScriptSpawn            , "ScriptSpawn"     },
	{kScriptFailToOpen       , "OnFailToOpen"    }
};

ScriptContainer::ScriptContainer() {
}

ScriptContainer::~ScriptContainer() {
}

const Common::UString &ScriptContainer::getScript(Script script) const {
	assert((script >= 0) && (script < kScriptMAX));

	return _scripts[script];
}

bool ScriptContainer::hasScript(Script script) const {
	return !getScript(script).empty();
}

void ScriptContainer::clearScripts() {
	for (size_t i = 0; i < kScriptMAX; i++)
		_scripts[i].clear();
}

void ScriptContainer::readScripts(const Aurora::GFF3Struct &gff) {
	clearScripts();

	for (size_t i = 0; i < ARRAYSIZE(kScriptNames); i++) {
		const Script script = kScriptNames[i].script;
		const char  *name   = kScriptNames[i].name;

		_scripts[script] = gff.getString(name, _scripts[script]);
	}
}

void ScriptContainer::readScripts(const ScriptContainer &container) {
	for (size_t i = 0; i < kScriptMAX; i++)
		_scripts[i] = container._scripts[i];
}

bool ScriptContainer::runScript(Script script, Aurora::NWScript::Object *owner,
                                Aurora::NWScript::Object *triggerer) {
	return runScript(getScript(script), owner, triggerer);
}

bool ScriptContainer::runScript(const Common::UString &script,
                                Aurora::NWScript::Object *owner,
                                Aurora::NWScript::Object *triggerer) {

	return runScript(script, Aurora::NWScript::NCSFile::getEmptyState(), owner, triggerer);
}

bool ScriptContainer::runScript(const Common::UString &script,
                                const Aurora::NWScript::ScriptState &state,
                                Aurora::NWScript::Object *owner,
                                Aurora::NWScript::Object *triggerer) {
	if (script.empty())
		return true;

	try {
		Aurora::NWScript::NCSFile ncs(script);

		const Aurora::NWScript::Variable &retVal = ncs.run(state, owner, triggerer);
		if (retVal.getType() == Aurora::NWScript::kTypeInt)
			return retVal.getInt() != 0;
		if (retVal.getType() == Aurora::NWScript::kTypeFloat)
			return retVal.getFloat() != 0.0f;

		return true;

	} catch (...) {
		Common::exceptionDispatcherWarning("Failed running script \"%s\"", script.c_str());
		return false;
	}

	return true;
}

} // End of namespace NWN

} // End of namespace Engines
