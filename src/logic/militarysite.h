/*
 * Copyright (C) 2002-2004, 2007-2009 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_LOGIC_MILITARYSITE_H
#define WL_LOGIC_MILITARYSITE_H

#include <memory>

#include "base/macros.h"
#include "logic/attackable.h"
#include "logic/productionsite.h"
#include "logic/requirements.h"
#include "logic/soldiercontrol.h"

namespace Widelands {

class Soldier;
class World;

struct MilitarySiteDescr : public ProductionSiteDescr {
	MilitarySiteDescr
		(char const * name, char const * descname,
		 const std::string & directory, Profile &,  Section & global_s,
		 const Tribe_Descr & tribe, const World& world);
	~MilitarySiteDescr() override {}

	Building & create_object() const override;

	uint32_t get_conquers() const override {return m_conquer_radius;}
	uint32_t get_max_number_of_soldiers () const {
		return m_num_soldiers;
	}
	uint32_t get_heal_per_second        () const {
		return m_heal_per_second;
	}

	bool     m_prefers_heroes_at_start;
	std::string m_occupied_str;
	std::string m_aggressor_str;
	std::string m_attack_str;
	std::string m_defeated_enemy_str;
	std::string m_defeated_you_str;


private:
	uint32_t m_conquer_radius;
	uint32_t m_num_soldiers;
	uint32_t m_heal_per_second;
	DISALLOW_COPY_AND_ASSIGN(MilitarySiteDescr);
};

class MilitarySite :
	public ProductionSite, public SoldierControl, public Attackable
{
	friend class Map_Buildingdata_Data_Packet;
	MO_DESCR(MilitarySiteDescr)

public:
	// I assume elsewhere, that enum SoldierPreference fits to uint8_t.
	enum SoldierPreference  : uint8_t {
		kNoPreference,
		kPrefersRookies,
		kPrefersHeroes,
	};

	MilitarySite(const MilitarySiteDescr &);
	virtual ~MilitarySite();

	const std::string& update_statistics_string() override;

	void init(Editor_Game_Base &) override;
	void cleanup(Editor_Game_Base &) override;
	void act(Game &, uint32_t data) override;
	void remove_worker(Worker &) override;

	void set_economy(Economy *) override;
	bool get_building_work(Game &, Worker &, bool success) override;

	// Begin implementation of SoldierControl
	std::vector<Soldier *> presentSoldiers() const override;
	std::vector<Soldier *> stationedSoldiers() const override;
	uint32_t minSoldierCapacity() const override;
	uint32_t maxSoldierCapacity() const override;
	uint32_t soldierCapacity() const override;
	void setSoldierCapacity(uint32_t capacity) override;
	void dropSoldier(Soldier &) override;
	int incorporateSoldier(Editor_Game_Base & game, Soldier & s) override;
	// End implementation of SoldierControl

	// Begin implementation of Attackable
	Player & owner() const override {return Building::owner();}
	bool canAttack() override;
	void aggressor(Soldier &) override;
	bool attack   (Soldier &) override;
	// End implementation of Attackable

	/// Launch the given soldier on an attack towards the given
	/// target building.
	void sendAttacker(Soldier &, Building &);

	/// This methods are helper for use at configure this site.
	void set_requirements  (const Requirements &);
	void clear_requirements();
	const Requirements & get_requirements () const {
		return m_soldier_requirements;
	}

	void reinit_after_conqueration(Game &);

	void update_soldier_request(bool i = false);

	void set_soldier_preference(SoldierPreference);
	SoldierPreference get_soldier_preference() const {
			return m_soldier_preference;
	}

protected:
	void conquer_area(Editor_Game_Base &);

	virtual void create_options_window
		(Interactive_GameBase &, UI::Window * & registry) override;

private:
	bool isPresent(Soldier &) const;
	static void request_soldier_callback
		(Game &, Request &, Ware_Index, Worker *, PlayerImmovable &);

	MapObject * popSoldierJob
		(Soldier *, bool * stayhome = nullptr);
	bool haveSoldierJob(Soldier &);
	bool military_presence_kept(Game &);
	void notify_player(Game &, bool discovered = false);
	bool update_upgrade_requirements();
	void update_normal_soldier_request();
	void update_upgrade_soldier_request();
	bool incorporateUpgradedSoldier(Editor_Game_Base & game, Soldier & s);
	Soldier * find_least_suited_soldier();
	bool drop_least_suited_soldier(bool new_has_arrived, Soldier * s);


private:
	Requirements m_soldier_requirements; // This is used to grab a bunch of soldiers: Anything goes
	RequireAttribute m_soldier_upgrade_requirements; // This is used when exchanging soldiers.
	std::unique_ptr<Request> m_normal_soldier_request;  // filling the site
	std::unique_ptr<Request> m_upgrade_soldier_request; // seeking for better soldiers
	bool m_didconquer;
	uint32_t m_capacity;

	/**
	 * Next gametime where we should heal something.
	 */
	int32_t m_nexthealtime;

	struct SoldierJob {
		Soldier    * soldier;
		Object_Ptr  enemy;
		bool        stayhome;
	};
	std::vector<SoldierJob> m_soldierjobs;
	SoldierPreference m_soldier_preference;
	int32_t m_next_swap_soldiers_time;
	bool m_soldier_upgrade_try; // optimization -- if everybody is zero-level, do not downgrade
	bool m_doing_upgrade_request;
	std::string m_statistics_string;
};

}

#endif  // end of include guard: WL_LOGIC_MILITARYSITE_H
