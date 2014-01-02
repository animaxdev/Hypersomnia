npc_class = { 
	entity = 0,
	weapon_animation_sets = {},
	current_weapon = bare_hands
	--last_impact = vec2(0, 0),
	--health_info = {
	--	hp = 100,
	--	
	--	corpse_entity = {}
	--}
}

function set_max_speed(entity, max_speed_val)
	entity.movement.max_speed = max_speed_val
	entity.steering.max_resultant_force = max_speed_val
end

function get_scripted(entity)
    --print(debug.traceback())
	return entity.scriptable.script_data
end

function init_npc(this_entity, optional_write)
	this_entity.scriptable.script_data = this_entity.scriptable.script_data:create()
	this_entity.scriptable.script_data.entity = this_entity
	
	if optional_write ~= nil then recursive_write(this_entity.scriptable.script_data, optional_write) end
	
	this_entity.scriptable.script_data:init()
	this_entity.scriptable.script_data.head_entity = create_entity (archetyped(this_entity.scriptable.script_data.head_archetype, {
		chase = {
			target = this_entity,
			chase_type = chase_component.ORBIT,
			rotation_orbit_offset = vec2(4, 0),
			chase_rotation = false
		}
	}))
		
end

function npc_class:create(o)  
	 local inst = o or {}
     setmetatable(inst, { __index = npc_class } )
     return inst
end

function npc_class:init()
	local o = self
	
	o.steering_behaviours = {
		target_seeking = behaviour_state(target_seek_steering),
		forward_seeking = behaviour_state(forward_seek_steering),
		
		sensor_avoidance = behaviour_state(sensor_avoidance_steering),
		wandering = behaviour_state(wander_steering),
		obstacle_avoidance = behaviour_state(obstacle_avoidance_steering),
		separating = behaviour_state(separation_steering),
		pursuit = behaviour_state(pursuit_steering)
	}
	
	o.target_entities = {
		navigation = create_entity(target_entity_archetype),
		forward = create_entity(target_entity_archetype),
		last_seen = create_entity(target_entity_archetype)
	}
	
	o.was_seen = false
	o.is_seen = false
	o.is_alert = false
	o.last_seen_velocity = vec2(0, 0)
		
	o.steering_behaviours.forward_seeking.target_from:set(o.target_entities.forward)
	o.steering_behaviours.target_seeking.target_from:set(o.target_entities.navigation)
	o.steering_behaviours.sensor_avoidance.target_from:set(o.target_entities.navigation)
	
	o.steering_behaviours.pursuit.enabled = false
	
	o:take_weapon_item(bare_hands)
end

function npc_class:refresh_behaviours() 
	self.entity.steering:clear_behaviours()
	
	for k, v in pairs(self.steering_behaviours) do
		self.entity.steering:add_behaviour(v)
	end
end

function npc_class:take_weapon_item(item_data)
	self.entity.animate:set_current_animation_set(self.weapon_animation_sets[item_data.animation_index], self.entity)
	
	self.entity.gun.trigger = false
	self.entity.gun.is_swinging = false
	
	if self.entity.gun.current_rounds > 0 then
		self.entity.gun.is_melee = false
	end
	
	if self.wielded_entity ~= nil then
		local msg = destroy_message()
		self.wielded_entity.name = "wielded_entity"
		msg.subject = self.wielded_entity
		world:post_destroy_message(msg)
		self.wielded_entity = nil
	end

	--if item_data.wielded_entity ~= nil and item_data ~= bare_hands then 
		self.wielded_entity = create_entity(archetyped(item_data.wielded_entity, {
			chase = {
				target = self.entity
			}
		}))
	--end
	
	self.entity.gun = gun_component(item_data.weapon_info)
	
	self.current_weapon = item_data
	self.entity.gun:set_bullet_filter(create(b2Filter, self.weapon_bullet_filter))
	print("taking weapon...\n" .. item_data.animation_index)
end

global_item_table = {}

function npc_class:take_weapon(weapon_entity)
	weapon_entity.gun:transfer_barrel_smoke(self.entity, false)
	
	self:take_weapon_item(get_scripted(weapon_entity).item_data)
	
	for k, v in ipairs(global_item_table) do
		if v == weapon_entity then
			table.remove(global_item_table, k)
			break
		end
	end
	
	local msg = destroy_message()
	msg.subject = weapon_entity
	world:post_destroy_message(msg)
end

function spawn_weapon(position, item_archetype, instance_data)
	local my_spawned_weapon = create_entity (archetyped(item_archetype.item_entity, {
		transform = {
			pos = position
		},
		
		scriptable = {
			script_data = {
				item_data = {}
			}
		},
		
		-- adding gun component only to generate barrel smoke
		gun = {}
	}))
	
	local item_data_table = my_spawned_weapon.scriptable.script_data.item_data
	
	recursive_write(item_data_table, item_archetype, { weapon_info = true })
	if instance_data ~= nil then item_data_table.weapon_info = gun_component(instance_data) end
	
	table.insert(global_item_table, my_spawned_weapon)
		
	return my_spawned_weapon
end

function npc_class:drop_weapon(force_multiplier)
	force_multiplier = force_multiplier or 1

	if self.current_weapon ~= bare_hands then
		print("dropping weapon..." .. self.current_weapon.animation_index)
		self.entity.gun.trigger = false
		local my_thrown_weapon = spawn_weapon(self.entity.transform.current.pos, self.current_weapon, self.entity.gun)
		
		self.entity.gun:transfer_barrel_smoke(my_thrown_weapon, true)
		my_thrown_weapon.gun:get_barrel_smoke():get().chase.rotation_orbit_offset = self.current_weapon.world_orbit_offset
		
		local throw_force = vec2.from_degrees(self.entity.transform.current.rotation) * 15 * force_multiplier
		
		local body = my_thrown_weapon.physics.body
		
		body:ApplyLinearImpulse(b2Vec2(throw_force.x, throw_force.y), body:GetWorldCenter())
		body:ApplyAngularImpulse(10 * force_multiplier)
		
		self:take_weapon_item(bare_hands)
	end
end

function npc_class:pick_up_weapon(try_to_pick_weapon)
	local items_in_range = physics_system:query_body(self.entity, create(b2Filter, filter_pick_up_items), nil)
	
	local was_something_on_ground = false
	
	for candidate in items_in_range.bodies do
		if try_to_pick_weapon == nil or (try_to_pick_weapon ~= nil and body_to_entity(candidate) == try_to_pick_weapon) then 
			print("picking up.. ")
			
			self:drop_weapon()
			self:take_weapon(body_to_entity(candidate))
			was_something_on_ground = true
			break
		end
	end
	
	if try_to_pick_weapon == nil and not was_something_on_ground then
		self:drop_weapon()
	end
	
	return was_something_on_ground
end

function npc_class:pursue_target(target_entity)			
	self.steering_behaviours.pursuit.target_from:set(target_entity)
	self.steering_behaviours.pursuit.enabled = true
	self.steering_behaviours.obstacle_avoidance.enabled = false
	--print(debug.traceback())
	self.steering_behaviours.sensor_avoidance.target_from:set(target_entity)
end

function npc_class:stop_pursuit()	
	self.steering_behaviours.pursuit.enabled = false
	self.steering_behaviours.obstacle_avoidance.enabled = true
	--print(debug.traceback())
	self.steering_behaviours.sensor_avoidance.target_from:set(self.target_entities.navigation)
end

function npc_class:good_health()
	return self.health_info.hp > 0
end

function npc_class:throw_corpse()
	self.entity:remove_behaviour_tree()
	
	local msg = destroy_message()
	msg.subject = self.entity
	world:post_destroy_message(msg)
	--msg.subject = self.head_entity
	--world:post_destroy_message(msg)
	
	
	--world:delete_entity(self.entity, nil)
	--world:delete_entity(self.head_entity, nil)

	local thrown_corpse_entity = create_entity (archetyped(self.health_info.corpse_entity, {
			transform = {
				pos = self.entity.transform.current.pos,
				rotation = self.last_impact:get_degrees()
			}
		}
	))
	
	self.head_entity.chase:set_target(thrown_corpse_entity)
	
	self.head_entity.chase.chase_type = chase_component.ORBIT
	self.head_entity.chase.chase_rotation = true
	self.head_entity.chase.rotation_orbit_offset = vec2(55, 0)
	self.head_entity.chase.rotation_offset = 90
	local corpse_body = thrown_corpse_entity.physics.body
	corpse_body:ApplyLinearImpulse(b2Vec2(self.last_impact.x*2, self.last_impact.y*2), corpse_body:GetWorldCenter())
	
	return thrown_corpse_entity
end

function npc_class:loop()
	local entity = self.entity
	local behaviours = self.steering_behaviours
	local target_entities = self.target_entities
	
	
	local myvel = entity.physics.body:GetLinearVelocity()
	target_entities.forward.transform.current.pos = entity.transform.current.pos + vec2(myvel.x, myvel.y) * 50
	
	if entity.pathfinding:is_still_pathfinding() or entity.pathfinding:is_still_exploring() then
		target_entities.navigation.transform.current.pos = entity.pathfinding:get_current_navigation_target()
		
		behaviours.obstacle_avoidance.enabled = true
		if behaviours.sensor_avoidance.last_output_force:non_zero() then
			behaviours.target_seeking.enabled = false
			behaviours.forward_seeking.enabled = true
			behaviours.obstacle_avoidance.enabled = true
		else
			behaviours.target_seeking.enabled = true
			behaviours.forward_seeking.enabled = false
			--behaviours.obstacle_avoidance.enabled = false
		end
	else
		behaviours.target_seeking.enabled = false
		behaviours.forward_seeking.enabled = false
		--behaviours.obstacle_avoidance.enabled = false
	end
	
	behaviours.sensor_avoidance.max_intervention_length = (entity.transform.current.pos - target_entities.navigation.transform.current.pos):length() - 70
	
	--	behaviours.sensor_avoidance.enabled = true
	--	player_behaviours.obstacle_avoidance.enabled = true
	--player_behaviours.forward_seeking.enabled = true
	
	if behaviours.obstacle_avoidance.last_output_force:non_zero() then
		behaviours.wandering.current_wander_angle = behaviours.obstacle_avoidance.last_output_force:get_degrees()
	end
	
	if not self:good_health() then
		print "dying.."
		self:drop_weapon(0.5)
		self:throw_corpse()
		return true
	end
	
	-- we don't see anything if player is dead
	if not player.body:exists() then 
		self.is_seen = false
	else
		-- resolve player visibility no matter what we're doing 
		local p1 = entity.transform.current.pos
		local p2 = player.body:get().transform.current.pos
		
		ray_output = physics_system:ray_cast(p1, p2, create(b2Filter, filter_pathfinding_visibility), entity)
		
		if not ray_output.hit then
			self.target_entities.last_seen.transform.current.pos = player.body:get().transform.current.pos
			
			self.was_seen = true
			self.is_seen = true
			self.is_alert = true
			
			local player_vel = player.body:get().physics.body:GetLinearVelocity()
			self.last_seen_velocity = vec2(player_vel.x, player_vel.y)
		else
			self.is_seen = false
		end
	end
	
	return true
end

transform_positions = {
	vec2(474 , -480),
	vec2(941 , -279),
	vec2(1261, -136),
	vec2(1300, -700),
	vec2(1967, -333),
	-- trojka na niebieskim
	vec2(1255, -1048),
	vec2(1508, -1184),
	vec2(1657, -977),
	
	--dwojka z lewej
	vec2(540, -1263),
	vec2(871, -690),
	
	--  w lewym gornym
	vec2(314, -1651),
	
	-- w gornym
	vec2(1667, -1446),
	
	-- prawy dolny
	vec2(1890, -626),
	vec2(2457, -354),
	
	--prawy
	vec2(2125, -1088)
	
}

head_images = {
	images.head_1, 
	images.head_2, 
	images.head_3, 
	images.head_4, 
	images.head_5, 
	images.head_6, 
	images.head_7, 
	images.head_8, 
	images.head_9, 
	images.head_10,
	images.head_11,
	images.head_12,
	images.head_13,
	images.head_14,
	images.head_15
}

npc_weapons = {
	bare_hands,
	fireaxe,
	assault_rifle,
	assault_rifle,
	shotgun,
	shotgun,
	assault_rifle,
	fireaxe,
	assault_rifle,
	fireaxe,
	shotgun,
	fireaxe,
	shotgun,
	assault_rifle,
	assault_rifle
}

head_sprites = {}

for i=1, 15 do
	head_sprites[i] = create_sprite {
		image = head_images[i]
	}
end

npc_count = 3
my_npcs = {}

final_npc_archetype = (archetyped(character_archetype, {
		body = {
			transform = { pos = vec2(1000, (-4800)) },
			
			behaviour_tree = {
				trees = {
					npc_alertness.behave,
					npc_legs_behaviour_tree.legs,
					npc_hands_behaviour_tree.hands
				}
			},
		
			lookat = {
				target = "body",
				look_mode = lookat_component.VELOCITY,
				easing_mode = lookat_component.EXPONENTIAL,
				averages_per_sec = 25
			},
						
			scriptable = {
				script_data = npc_class
			},
			
			physics = {
				body_info = {
					filter = filter_enemies
				}
			}
		},
		
		legs = {
			lookat = {
				target = "body",
				look_mode = lookat_component.VELOCITY,
				easing_mode = lookat_component.EXPONENTIAL,
				averages_per_sec = 10
			},
			
			
			animate = {
				available_animations = enemy_animation_legs_set
			}
		}
}))



--spawn_weapon(vec2(1300, (-2800)), assddd
for i=1, npc_count do

	--if i == 1 then 
		--my_npcs[i] = ptr_create_entity_group(final_npc_archetype)
	--else
		my_npcs[i] = ptr_create_entity_group(archetyped(final_npc_archetype, {
			body = {
				transform = {
					pos = transform_positions[i]
				}
			}
		})) 
	--end
	
	
	init_npc(my_npcs[i].body:get(), { 
	weapon_animation_sets = {
		BARE_HANDS = enemy_animation_bare_hands_set,
		FIREAXE = enemy_animation_melee_set,
		ASSAULT_RIFLE = enemy_animation_firearm_set,
		SHOTGUN = enemy_animation_firearm_set
	},
	
		
		health_info = {
			hp = 100,
			
			corpse_entity = archetyped(corpse_archetype, {
				render = {
					model = corpse_sprite
				}
			})			
		},
			
		wield_offsets = npc_wield_offsets,
		
		head_archetype =  {
			transform = {},
			
			chase = {
				--rotation_orbit_offset = vec2(2, 0)
			},
			
			render = {
				layer = layers.HEADS,
				model = head_sprites[i]
			}
		},
		
		weapon_bullet_filter = filter_enemy_bullets
	})
	
	local script_data = get_scripted(my_npcs[i].body:get())
	script_data:refresh_behaviours()
	script_data:take_weapon_item(bare_hands)
	--my_npcs[i].body.pathfinding:start_exploring()
	
	my_npcs[i].body:get().gun.target_camera_to_shake:set(world_camera)
	get_scripted(my_npcs[i].body:get()):take_weapon_item(npc_weapons[i])
end

