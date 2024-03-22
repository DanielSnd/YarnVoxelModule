//
// Created by Daniel on 2024-03-17.
//

#include "island_gen_node.h"

void IslandGeneratorHelper::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_island_gen", "island_gen"), &IslandGeneratorHelper::set_island_gen);
    ClassDB::bind_method(D_METHOD("get_island_gen"), &IslandGeneratorHelper::get_island_gen);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "island_generator", PROPERTY_HINT_RESOURCE_TYPE, "IslandGenerator"), "set_island_gen", "get_island_gen");
}
