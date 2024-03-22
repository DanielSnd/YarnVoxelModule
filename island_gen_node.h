//
// Created by Daniel on 2024-03-17.
//

#ifndef ISLAND_GEN_NODE_H
#define ISLAND_GEN_NODE_H

#include "island_generator.h"
#include "scene/3d/node_3d.h"

class IslandGeneratorHelper : public Node3D {
    GDCLASS(IslandGeneratorHelper, Node3D);

private:
    static void _bind_methods();
public:
    Ref<IslandGenerator> island_gen;
    void set_island_gen(Ref<IslandGenerator> val) {island_gen = val;}
    Ref<IslandGenerator> get_island_gen() {return island_gen;}
};


#endif //ISLAND_GEN_NODE_H
