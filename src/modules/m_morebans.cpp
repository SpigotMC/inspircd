/*
 * InspIRCd -- Internet Relay Chat Daemon
 *
 *   Copyright (C) 2015 NickG365 <nick+inspircd@gameon365.net>
 *
 * This file is part of InspIRCd.  InspIRCd is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* $ModAuthor: NickG365 */
/* $ModAuthorMail: nick+inspircd@gameon365.net */
/* $ModDesc: Allows increasing the ban list limit for specific channels with channel mode +E (max) */

#include <stdlib.h>
#include "inspircd.h"

class LargeBanListMode : public ModeHandler
{
 public:
	LargeBanListMode(Module* Creator) : ModeHandler(Creator, "largebanlist", 'E', PARAM_SETONLY, MODETYPE_CHANNEL)
	{
		oper = true;
	}

	ModeAction OnModeChange(User* source, User* dest, Channel* channel, std::string &parameter, bool adding)
	{
		// Removing mode that wasn't set
		if (channel->IsModeSet(this) == false && !adding)
			return MODEACTION_DENY;
		// Removing mode
		if (channel->IsModeSet(this) && !adding)
		{
			ServerInstance->Config->maxbans[channel->name] = ServerInstance->Config->maxbans.count("*") ? ServerInstance->Config->maxbans["*"] : 64;
			channel->ResetMaxBans();
			return MODEACTION_ALLOW;
		}
		// Adding or updating mode
		long newmax = strtol(parameter.c_str(), NULL, 10);
		if (newmax == channel->GetMaxBans() || newmax == (ServerInstance->Config->maxbans.count("*") ? ServerInstance->Config->maxbans["*"] : 64))
			return MODEACTION_DENY;
		ServerInstance->Config->maxbans[channel->name] = newmax;
		channel->ResetMaxBans();
		channel->SetModeParam(this, parameter);
		return MODEACTION_ALLOW;
	}
};

class ModuleLargeBanList : public Module
{
 private:
	LargeBanListMode mode;
 public:
	ModuleLargeBanList() : mode(this)
	{
	}

	void init()
	{
		ServerInstance->Modules->AddService(mode);

		Implementation eventlist[] = { I_OnRehash };
		ServerInstance->Modules->Attach(eventlist, this, sizeof(eventlist)/sizeof(Implementation));

		OnRehash(NULL);
	}

	void OnRehash(User* user)
	{
		for (chan_hash::iterator i = ServerInstance->chanlist->begin(); i != ServerInstance->chanlist->end(); )
		{
			Channel* channel = i->second;
			++i;
			if (channel->IsModeSet('E'))
			{
				long newmax = strtol(channel->GetModeParameter('E').c_str(), NULL, 10);
				ServerInstance->Config->maxbans[channel->name] = newmax;
				channel->ResetMaxBans();
			}
		}
	}

	Version GetVersion()
	{
		return Version("Allows increasing the ban list limit for specific channels with channel mode +E (max)");
	}
};

MODULE_INIT(ModuleLargeBanList)
