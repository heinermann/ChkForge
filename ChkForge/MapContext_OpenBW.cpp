#include "MapContext.h"

using namespace ChkForge;

void MapContext::chkdraft_to_openbw(bool is_editor_mode)
{
  openbw_ui.reset();

  bwgame::game_load_functions game_load_funcs(openbw_ui.st);
  game_load_funcs.use_map_settings = true;

  // Sync dimensions
  size_t tile_width = chk.layers.getTileWidth(), tile_height = chk.layers.getTileHeight();
  game_load_funcs.st.game->map_tile_width = tile_width;
  game_load_funcs.st.game->map_tile_height = tile_height;
  game_load_funcs.st.game->map_width = tile_width * 32;
  game_load_funcs.st.game->map_height = tile_height * 32;
  game_load_funcs.st.game->map_walk_width = tile_width * 4;
  game_load_funcs.st.game->map_walk_height = tile_height * 4;

  // Sync tileset
  game_load_funcs.st.game->tileset_index = chk.layers.getTileset() % 8;

  // Sync player data
  for (size_t i = 0; i != Sc::Player::Total; ++i) {
    game_load_funcs.st.players[i].controller = chk.players.getSlotType(i);

    Chk::Race srcRace = chk.players.getPlayerRace(i);
    if (srcRace == Chk::Race::UserSelectable || srcRace == Chk::Race::Random) {
      srcRace = static_cast<Chk::Race>(random() % 3);
    }
    switch (srcRace) {
    case Chk::Race::Zerg:
      game_load_funcs.st.players[i].race = bwgame::race_t::zerg;
      break;
    case Chk::Race::Protoss:
      game_load_funcs.st.players[i].race = bwgame::race_t::protoss;
      break;
    default:
      game_load_funcs.st.players[i].race = bwgame::race_t::terran;
      break;
    }

    if (i < Sc::Player::TotalSlots) {
      game_load_funcs.st.players[i].force = chk.players.getPlayerForce(i);
      int color = chk.players.getPlayerColor(i);
      if (color < 0 || color >= 16) color = i;
      game_load_funcs.st.players[i].color = color;
    }
  }

  // Sync string data
  game_load_funcs.st.game->map_strings.clear();
  size_t num_strings = chk.strings.getCapacity(Chk::Scope::Game);
  game_load_funcs.st.game->map_strings.reserve(num_strings);
  for (size_t i = 1; i < num_strings; ++i) {
    auto str = chk.strings.getString<RawString>(i, Chk::Scope::Game);
    if (str) {
      game_load_funcs.st.game->map_strings.emplace_back(*str.get());
    }
    else {
      game_load_funcs.st.game->map_strings.emplace_back("");
    }
  }

  auto scenario_name = chk.strings.getScenarioName<RawString>(Chk::Scope::GameOverEditor);
  if (scenario_name) {
    game_load_funcs.st.game->scenario_name = *scenario_name.get();
  }
  auto scenario_desc = chk.strings.getScenarioDescription<RawString>(Chk::Scope::GameOverEditor);
  if (scenario_desc) {
    game_load_funcs.st.game->scenario_description = *scenario_desc.get();
  }

  //--
  game_load_funcs.reset();
  //--

  // Sync MTXM (Tiles) and MASK (Fog)
  bool mtxm_out_of_range = false;
  bool mask_out_of_range = false;
  game_load_funcs.st.game->gfx_tiles.resize(tile_width * tile_height);
  for (size_t i = 0; i < tile_width * tile_height; ++i) {
    bwgame::tile_id tile{ 0 };
    if (!mtxm_out_of_range) {
      try {
        tile.raw_value = chk.layers.mtxm->getTile(i);
      }
      catch (std::out_of_range e) {
        mtxm_out_of_range = true;
      }
    }
    game_load_funcs.st.game->gfx_tiles.at(i) = tile;

    uint8_t mask = 0;
    if (!mask_out_of_range) {
      try {
        mask = chk.layers.mask->getFog(i);
      }
      catch (std::out_of_range e) {
        mask_out_of_range = true;
      }
    }
    game_load_funcs.st.tiles[i].visible = game_load_funcs.st.tiles[i].explored = mask;
  }

  for (size_t i = 0; i != game_load_funcs.st.game->gfx_tiles.size(); ++i) {
    bwgame::tile_id tile_id = game_load_funcs.st.game->gfx_tiles[i];
    if (tile_id.group_index() >= game_load_funcs.cv5().size()) tile_id = {};
    size_t megatile_index = game_load_funcs.cv5().at(tile_id.group_index()).mega_tile_index[tile_id.subtile_index()];

    int cv5_flags = game_load_funcs.cv5().at(tile_id.group_index()).flags & ~(
      bwgame::tile_t::flag_walkable | bwgame::tile_t::flag_unwalkable | bwgame::tile_t::flag_very_high |
      bwgame::tile_t::flag_middle | bwgame::tile_t::flag_high | bwgame::tile_t::flag_partially_walkable
      );

    game_load_funcs.st.tiles_mega_tile_index[i] = (uint16_t)megatile_index;
    game_load_funcs.st.tiles[i].flags = game_load_funcs.mega_tile_flags().at(megatile_index) | cv5_flags;
    if (tile_id.has_creep()) {
      game_load_funcs.st.tiles_mega_tile_index[i] |= 0x8000;
      game_load_funcs.st.tiles[i].flags |= bwgame::tile_t::flag_has_creep;
    }
  }

  game_load_funcs.tiles_flags_and(0, game_load_funcs.st.game->map_tile_height - 2, 5, 1, ~(bwgame::tile_t::flag_walkable | bwgame::tile_t::flag_has_creep | bwgame::tile_t::flag_partially_walkable));
  game_load_funcs.tiles_flags_or(0, game_load_funcs.st.game->map_tile_height - 2, 5, 1, bwgame::tile_t::flag_unbuildable);
  game_load_funcs.tiles_flags_and(game_load_funcs.st.game->map_tile_width - 5, game_load_funcs.st.game->map_tile_height - 2, 5, 1, ~(bwgame::tile_t::flag_walkable | bwgame::tile_t::flag_has_creep | bwgame::tile_t::flag_partially_walkable));
  game_load_funcs.tiles_flags_or(game_load_funcs.st.game->map_tile_width - 5, game_load_funcs.st.game->map_tile_height - 2, 5, 1, bwgame::tile_t::flag_unbuildable);

  game_load_funcs.tiles_flags_and(0, game_load_funcs.st.game->map_tile_height - 1, game_load_funcs.st.game->map_tile_width, 1, ~(bwgame::tile_t::flag_walkable | bwgame::tile_t::flag_has_creep | bwgame::tile_t::flag_partially_walkable));
  game_load_funcs.tiles_flags_or(0, game_load_funcs.st.game->map_tile_height - 1, game_load_funcs.st.game->map_tile_width, 1, bwgame::tile_t::flag_unbuildable);

  game_load_funcs.regions_create();

  // Sync THG2 (Sprites)
  for (size_t i = 0; i < chk.layers.thg2->numSprites(); ++i) {
    Chk::SpritePtr sprite = chk.layers.thg2->getSprite(i);
    bwgame::xy position{ sprite->xc, sprite->yc };
    if (sprite->isDrawnAsSprite()) {
      if (sprite->type >= Sc::Sprite::TotalSprites) continue;
      auto* obw_sprite_type = game_load_funcs.get_sprite_type(bwgame::SpriteTypes(sprite->type));
      game_load_funcs.create_thingy(obw_sprite_type, position, sprite->owner);
    }
    else {
      if (sprite->type >= Sc::Unit::TotalTypes) continue;
      int owner = sprite->owner;
      auto* obw_unit_type = game_load_funcs.get_unit_type(bwgame::UnitTypes(sprite->type));
      if (game_load_funcs.unit_is_door(obw_unit_type)) owner = 11;
      bwgame::unit_t* unit = game_load_funcs.create_initial_unit(obw_unit_type, position, owner);
      if (sprite->flags & 0x80) game_load_funcs.disable_thg2_unit(unit);
    }
  }

  // Sync Unit and Weapon settings
  for (size_t i = 0; i < Sc::Unit::TotalTypes; ++i) {
    auto chkd_type = Sc::Unit::Type(i);
    if (chk.properties.unitUsesDefaultSettings(chkd_type)) continue;

    bwgame::unit_type_t* unit_type = game_load_funcs.get_unit_type(bwgame::UnitTypes(i));

    unit_type->hitpoints = bwgame::fp8::from_raw(chk.properties.getUnitHitpoints(chkd_type));
    unit_type->shield_points = chk.properties.getUnitShieldPoints(chkd_type);
    unit_type->armor = chk.properties.getUnitArmorLevel(chkd_type);
    unit_type->build_time = chk.properties.getUnitBuildTime(chkd_type);
    unit_type->mineral_cost = chk.properties.getUnitMineralCost(chkd_type);
    unit_type->gas_cost = chk.properties.getUnitGasCost(chkd_type);
    unit_type->unit_map_string_index = chk.properties.getUnitNameStringId(chkd_type);

    bwgame::unit_type_t* attacking_type = unit_type->turret_unit_type ? (bwgame::unit_type_t*)unit_type->turret_unit_type : unit_type;
    bwgame::weapon_type_t* grnd_wpn = (bwgame::weapon_type_t*)(const bwgame::weapon_type_t*)attacking_type->ground_weapon;
    bwgame::weapon_type_t* air_wpn = (bwgame::weapon_type_t*)(const bwgame::weapon_type_t*)attacking_type->air_weapon;

    if (grnd_wpn) {
      grnd_wpn->damage_amount = chk.properties.getWeaponBaseDamage(Sc::Weapon::Type(grnd_wpn->id));
      grnd_wpn->damage_bonus = chk.properties.getWeaponUpgradeDamage(Sc::Weapon::Type(grnd_wpn->id));
    }
    if (air_wpn) {
      air_wpn->damage_amount = chk.properties.getWeaponBaseDamage(Sc::Weapon::Type(air_wpn->id));
      air_wpn->damage_bonus = chk.properties.getWeaponUpgradeDamage(Sc::Weapon::Type(air_wpn->id));
    }
  }

  // Sync Upgrade settings
  size_t num_upgrades = chk.versions.isHybridOrAbove() ? Sc::Upgrade::TotalTypes : Sc::Upgrade::TotalOriginalTypes;
  for (size_t i = 0; i < num_upgrades; ++i) {
    auto chkd_type = Sc::Upgrade::Type(i);
    if (chk.properties.upgradeUsesDefaultCosts(chkd_type)) continue;

    bwgame::upgrade_type_t* upg_type = game_load_funcs.get_upgrade_type(bwgame::UpgradeTypes(i));
    upg_type->mineral_cost_base = chk.properties.getUpgradeBaseMineralCost(chkd_type);
    upg_type->mineral_cost_factor = chk.properties.getUpgradeMineralCostFactor(chkd_type);
    upg_type->gas_cost_base = chk.properties.getUpgradeBaseGasCost(chkd_type);
    upg_type->gas_cost_factor = chk.properties.getUpgradeGasCostFactor(chkd_type);
    upg_type->time_cost_base = chk.properties.getUpgradeBaseResearchTime(chkd_type);
    upg_type->time_cost_factor = chk.properties.getUpgradeResearchTimeFactor(chkd_type);
  }

  // Sync Tech settings
  size_t num_techs = chk.versions.isHybridOrAbove() ? Sc::Tech::TotalTypes : Sc::Tech::TotalOriginalTypes;
  for (size_t i = 0; i < num_techs; ++i) {
    auto chkd_type = Sc::Tech::Type(i);
    if (chk.properties.techUsesDefaultSettings(chkd_type)) continue;

    bwgame::tech_type_t* tech_type = game_load_funcs.get_tech_type(bwgame::TechTypes(i));
    tech_type->mineral_cost = chk.properties.getTechMineralCost(chkd_type);
    tech_type->gas_cost = chk.properties.getTechGasCost(chkd_type);
    tech_type->research_time = chk.properties.getTechResearchTime(chkd_type);
    tech_type->energy_cost = chk.properties.getTechEnergyCost(chkd_type);
  }

  // Sync Upgrade restrictions
  for (size_t player = 0; player < Sc::Player::Total; ++player) {
    for (size_t i = 0; i < num_upgrades; ++i) {
      auto chkd_type = Sc::Upgrade::Type(i);
      bool use_default = chk.properties.playerUsesDefaultUpgradeLeveling(chkd_type, player);
      game_load_funcs.game_st.max_upgrade_levels[player][bwgame::UpgradeTypes(i)]
        = use_default ? chk.properties.getDefaultMaxUpgradeLevel(chkd_type) : chk.properties.getMaxUpgradeLevel(chkd_type, player);

      game_load_funcs.st.upgrade_levels[player][bwgame::UpgradeTypes(i)]
        = use_default ? chk.properties.getDefaultStartUpgradeLevel(chkd_type) : chk.properties.getStartUpgradeLevel(chkd_type, player);
    }
  }

  // Sync Tech restrictions
  for (size_t player = 0; player < Sc::Player::Total; ++player) {
    for (size_t i = 0; i < num_techs; ++i) {
      auto chkd_type = Sc::Tech::Type(i);
      bool use_default = chk.properties.playerUsesDefaultTechSettings(chkd_type, player);

      game_load_funcs.game_st.tech_available[player][bwgame::TechTypes(i)]
        = use_default ? chk.properties.techDefaultAvailable(chkd_type) : chk.properties.techAvailable(chkd_type, player);

      game_load_funcs.st.tech_researched[player][bwgame::TechTypes(i)]
        = use_default ? chk.properties.techDefaultResearched(chkd_type) : chk.properties.techResearched(chkd_type, player);
    }
  }

  // Sync Unit restrictions
  for (size_t player = 0; player < Sc::Player::Total; ++player) {
    for (size_t i = 0; i < Sc::Unit::TotalTypes; ++i) {
      auto chkd_type = Sc::Unit::Type(i);
      bool use_default = chk.properties.playerUsesDefaultUnitBuildability(chkd_type, player);

      game_load_funcs.game_st.unit_type_allowed[player][bwgame::UnitTypes(i)]
        = use_default ? chk.properties.isUnitDefaultBuildable(chkd_type) : chk.properties.isUnitBuildable(chkd_type, player);
    }
  }

  // Sync Unit placement
  for (size_t i = 0; i < chk.layers.numUnits(); ++i) {
    auto unit = chk.layers.getUnit(i);

    int type = unit->type;
    int owner = unit->owner;

    if (type >= Sc::Unit::TotalTypes) continue;
    if (owner >= Sc::Player::Total) owner = 0;

    if (type == Sc::Unit::Type::StartLocation) {
      game_load_funcs.game_st.start_locations[owner] = { unit->xc, unit->yc };
    }

    const bwgame::unit_type_t* obw_unit_type = game_load_funcs.get_unit_type(bwgame::UnitTypes(type));
    bwgame::unit_t* new_unit = game_load_funcs.create_completed_unit(obw_unit_type, { unit->xc, unit->yc }, owner);
    if (!new_unit) continue;
    if (game_load_funcs.unit_type_spreads_creep(obw_unit_type, true) || game_load_funcs.ut_requires_creep(obw_unit_type)) {
      game_load_funcs.spread_creep_completely(new_unit, new_unit->sprite->position);
    }


    if (unit->validFieldFlags & Chk::Unit::ValidField::Hitpoints) {
      using tmp_t = bwgame::fixed_point<32, 8, true>;
      tmp_t tmp = tmp_t::extend(new_unit->unit_type->hitpoints);
      tmp = std::max(tmp * int(unit->hitpointPercent) / 100, tmp_t::from_raw(1));
      game_load_funcs.set_unit_hp(new_unit, bwgame::fp8::truncate(tmp));
    }
    if (unit->validFieldFlags & Chk::Unit::ValidField::Shields) {
      game_load_funcs.set_unit_shield_points(new_unit, bwgame::fp8::integer(new_unit->unit_type->shield_points * unit->shieldPercent / 100));
    }
    if (unit->validFieldFlags & Chk::Unit::ValidField::Energy) {
      game_load_funcs.set_unit_energy(new_unit, game_load_funcs.unit_max_energy(new_unit) * int(unit->energyPercent) / 100);
    }
    if (unit->validFieldFlags & Chk::Unit::ValidField::Resources) {
      game_load_funcs.set_unit_resources(new_unit, unit->resourceAmount);
    }
    if (unit->validFieldFlags & Chk::Unit::ValidField::Hanger) {
      // TODO
    }

    if (unit->validStateFlags & unit->stateFlags & Chk::Unit::State::Cloak) {
      // TODO 
      //game_load_funcs.cloak_unit(new_unit);
    }

    if (unit->validStateFlags & unit->stateFlags & Chk::Unit::State::Burrow) {
      if (game_load_funcs.unit_can_use_tech(new_unit, game_load_funcs.get_tech_type(bwgame::TechTypes::Burrowing))) {
        for (auto img = new_unit->sprite->images.begin(); img != new_unit->sprite->images.end(); ++img) {
          game_load_funcs.iscript_run_anim(&*img, 14);
        }
        game_load_funcs.set_unit_burrowed(new_unit);
        game_load_funcs.cloak_unit(new_unit);
        game_load_funcs.u_set_status_flag(new_unit, bwgame::unit_t::status_flags_t::status_flag_no_collide);
        // TODO: Finish (missing a function)
      }
    }
    if (unit->validStateFlags & unit->stateFlags & Chk::Unit::State::InTransit) {
      if (game_load_funcs.unit_can_receive_order(new_unit, game_load_funcs.get_order_type(bwgame::Orders::BuildingLiftoff), new_unit->owner)) {
        // TODO (unknown)
      }
    }
    if (unit->validStateFlags & unit->stateFlags & Chk::Unit::State::Hallucinated) {
      game_load_funcs.make_unit_hallucination(new_unit);
    }
    if (unit->validStateFlags & unit->stateFlags & Chk::Unit::State::Invincible) {
      game_load_funcs.u_set_status_flag(new_unit, bwgame::unit_t::status_flags_t::status_flag_invincible);
    }

    if (type == Sc::Unit::Type::ZergBroodling) {
      game_load_funcs.set_remove_timer(new_unit);
    }
  }

  for (bwgame::unit_t* u : bwgame::ptr(game_load_funcs.st.visible_units)) {
    game_load_funcs.update_unit_sprite(u);
  }
  game_load_funcs.st.update_psionic_matrix = true;

  // Sync unit properties (for triggers)
  // TODO

  // Sync locations (for triggers)
  for (size_t i = 0; i < chk.layers.numLocations(); ++i) {
    auto chk_loc = chk.layers.getLocation(i);
    auto& openbw_loc = game_load_funcs.st.locations.emplace_back();

    if (!chk_loc) continue;
    openbw_loc.area = bwgame::rect{ {int(chk_loc->left), int(chk_loc->top)}, {int(chk_loc->right), int(chk_loc->bottom)} };
    openbw_loc.elevation_flags = chk_loc->elevationFlags;
  }

  // Sync Triggers
  for (size_t i = 0; i < chk.triggers.numTriggers(); ++i) {
    auto chk_trig = chk.triggers.getTrigger(i);
    auto& openbw_trig = game_load_funcs.game_st.triggers.emplace_back();
    // TODO
  }
  // TODO
}
