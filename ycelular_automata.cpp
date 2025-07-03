#include "ycelular_automata.h"


YCellularAutomata::YCellularAutomata() {
    // Initialize with default configuration
    config = CellularConfig();
    
	rng.instantiate();

    // Initialize arrays to nullptr
    world_state = nullptr;
    next_state = nullptr;
    locked_cells = nullptr;
    
    // Initialize with default region (center of world)
    current_region = GenerationRegion(Vector2i(WORLD_SIZE_X/2 - 64, WORLD_SIZE_Y/2 - 64), Vector2i(128, 128));

}

YCellularAutomata::~YCellularAutomata() {
    // Cleanup allocated arrays
    if (world_state) {
        delete[] world_state;
        world_state = nullptr;
    }
    if (next_state) {
        delete[] next_state;
        next_state = nullptr;
    }
    if (locked_cells) {
        delete[] locked_cells;
        locked_cells = nullptr;
    }
    rng = nullptr;
}

void YCellularAutomata::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_iterations", "iterations"), &YCellularAutomata::set_iterations);
    ClassDB::bind_method(D_METHOD("get_iterations"), &YCellularAutomata::get_iterations);
    
    ClassDB::bind_method(D_METHOD("set_neighborhood_size", "size"), &YCellularAutomata::set_neighborhood_size);
    ClassDB::bind_method(D_METHOD("get_neighborhood_size"), &YCellularAutomata::get_neighborhood_size);
    
    ClassDB::bind_method(D_METHOD("set_wrap_edges", "wrap"), &YCellularAutomata::set_wrap_edges);
    ClassDB::bind_method(D_METHOD("get_wrap_edges"), &YCellularAutomata::get_wrap_edges);
    
    ClassDB::bind_method(D_METHOD("set_random_seed", "seed"), &YCellularAutomata::set_random_seed);
    ClassDB::bind_method(D_METHOD("get_random_seed"), &YCellularAutomata::get_random_seed);
    
    // Note: add_rule method is not exposed to GDScript since it requires a struct
    // Use the specific rule addition methods instead
    ClassDB::bind_method(D_METHOD("remove_rule", "index"), &YCellularAutomata::remove_rule);
    ClassDB::bind_method(D_METHOD("clear_rules"), &YCellularAutomata::clear_rules);
    ClassDB::bind_method(D_METHOD("get_rule", "index"), &YCellularAutomata::get_rule_dict);
    ClassDB::bind_method(D_METHOD("set_rule", "index", "rule"), &YCellularAutomata::set_rule_dict);
    ClassDB::bind_method(D_METHOD("get_rule_count"), &YCellularAutomata::get_rule_count);
    
    // GDScript-friendly rule addition methods  
    ClassDB::bind_method(D_METHOD("add_surrounding_count_rule", "threshold", "target_type", "source_type", "override_range", "probability"), &  YCellularAutomata::add_surrounding_count_rule, DEFVAL(0), DEFVAL(0), DEFVAL(0), DEFVAL(1.0f));
    ClassDB::bind_method(D_METHOD("add_surrounding_count_less_rule", "threshold", "target_type", "source_type", "override_range", "probability"), &YCellularAutomata::add_surrounding_count_less_rule, DEFVAL(0), DEFVAL(0), DEFVAL(0), DEFVAL(1.0f));
    ClassDB::bind_method(D_METHOD("add_surrounding_empty_rule", "threshold", "target_type", "source_type", "override_range", "probability"), &YCellularAutomata::add_surrounding_empty_rule, DEFVAL(0), DEFVAL(0), DEFVAL(0), DEFVAL(1.0f));
    ClassDB::bind_method(D_METHOD("add_surrounding_type_rule", "threshold", "target_type", "source_type", "neighbor_type", "override_range", "override_forbidden_range", "probability"), &YCellularAutomata::add_surrounding_type_rule, DEFVAL(0), DEFVAL(0), DEFVAL(0), DEFVAL(0), DEFVAL(1.0f));
    ClassDB::bind_method(D_METHOD("add_surrounding_type_less_rule", "threshold", "target_type", "source_type", "neighbor_type", "override_range", "override_forbidden_range", "probability"), &YCellularAutomata::add_surrounding_type_less_rule, DEFVAL(0), DEFVAL(0), DEFVAL(0), DEFVAL(0), DEFVAL(1.0f));
    ClassDB::bind_method(D_METHOD("add_no_specific_type_rule", "target_type", "source_type", "forbidden_type", "override_range", "probability"), &YCellularAutomata::add_no_specific_type_rule, DEFVAL(0), DEFVAL(0), DEFVAL(0), DEFVAL(1.0f));
    ClassDB::bind_method(D_METHOD("add_surrounding_with_exception_rule", "threshold", "target_type", "source_type", "neighbor_type", "forbidden_type", "override_range", "override_forbidden_range", "probability"), &YCellularAutomata::add_surrounding_with_exception_rule, DEFVAL(0), DEFVAL(0), DEFVAL(0), DEFVAL(0), DEFVAL(0), DEFVAL(0), DEFVAL(1.0f));
    ClassDB::bind_method(D_METHOD("add_random_chance_rule", "probability", "target_type", "source_type"), &YCellularAutomata::add_random_chance_rule, DEFVAL(0), DEFVAL(0));
    ClassDB::bind_method(D_METHOD("add_avoid_opposite_cross_rule", "target_type", "source_type", "look_for_type", "override_range"), &YCellularAutomata::add_avoid_opposite_cross_rule, DEFVAL(0), DEFVAL(0), DEFVAL(0), DEFVAL(2));
    ClassDB::bind_method(D_METHOD("add_avoid_opposite_diagonal_rule", "target_type", "source_type", "look_for_type", "override_range"), &YCellularAutomata::add_avoid_opposite_diagonal_rule, DEFVAL(1), DEFVAL(0), DEFVAL(1), DEFVAL(2)); 
    // // Region-based methods
    ClassDB::bind_method(D_METHOD("initialize_world"), &YCellularAutomata::initialize_world);
    ClassDB::bind_method(D_METHOD("set_working_region", "starting_pos", "size"), &YCellularAutomata::set_working_region_gdscript, DEFVAL(Vector2i(0, 0)), DEFVAL(Vector2i(0, 0)));
    ClassDB::bind_method(D_METHOD("get_working_region"), &YCellularAutomata::get_working_region_dict);
    // ClassDB::bind_method(D_METHOD("set_initial_region_state", "initial_data"), &YCellularAutomata::set_initial_region_state);
    ClassDB::bind_method(D_METHOD("get_world_state"), &YCellularAutomata::get_world_state_gdscript);
    
    ClassDB::bind_method(D_METHOD("reset_world"), &YCellularAutomata::reset_world);
    ClassDB::bind_method(D_METHOD("is_initialized"), &YCellularAutomata::is_initialized);
    ClassDB::bind_method(D_METHOD("get_world_size"), &YCellularAutomata::get_world_size);
        
    // // Phase-based processing for current region
    ClassDB::bind_method(D_METHOD("fill_region_with_random"), &YCellularAutomata::fill_region_with_random, DEFVAL(0.45f));
    ClassDB::bind_method(D_METHOD("fill_region_edges", "type", "width"), &YCellularAutomata::fill_region_edges_with, DEFVAL(1), DEFVAL(1));
    ClassDB::bind_method(D_METHOD("connect_region_to_other_region", "starting_pos", "size", "other_starting_pos", "other_size", "connection_width"), &YCellularAutomata::connect_region_to_other_region, DEFVAL(3));
    ClassDB::bind_method(D_METHOD("connect_all_isolated_air_pockets", "connection_width", "prioritize_largest"), &YCellularAutomata::connect_all_isolated_air_pockets, DEFVAL(3), DEFVAL(true));

    ClassDB::bind_method(D_METHOD("process_rules", "iterations", "debug_printing"), &YCellularAutomata::process_rules, DEFVAL(1), DEFVAL(false));

    // // Locking system
    ClassDB::bind_method(D_METHOD("lock_cell", "pos"), &YCellularAutomata::lock_cell);
    ClassDB::bind_method(D_METHOD("unlock_cell", "pos"), &YCellularAutomata::unlock_cell);
    ClassDB::bind_method(D_METHOD("is_cell_locked", "pos"), &YCellularAutomata::is_cell_locked);
    ClassDB::bind_method(D_METHOD("lock_region_filled_cells"), &YCellularAutomata::lock_region_filled_cells);
    ClassDB::bind_method(D_METHOD("get_locked_cells"), &YCellularAutomata::get_locked_cells_gdscript);
    ClassDB::bind_method(D_METHOD("carve_path_between_cells", "start_cell", "end_cell", "path_width"), &YCellularAutomata::carve_path_between_cells, DEFVAL(3));
    ClassDB::bind_method(D_METHOD("set_area_to_type", "position", "type", "width", "circle"), &YCellularAutomata::set_area_to_type, DEFVAL(0), DEFVAL(3), DEFVAL(true));
    ClassDB::bind_method(D_METHOD("set_line_to_type", "start", "end", "type", "path_width", "circle"), &YCellularAutomata::set_line_to_type, DEFVAL(0), DEFVAL(3), DEFVAL(true));
    
    ClassDB::bind_method(D_METHOD("print_region_state"), &YCellularAutomata::print_region_state);
    ClassDB::bind_method(D_METHOD("get_region_state_as_string"), &YCellularAutomata::get_region_state_as_string);
    
    ClassDB::bind_method(D_METHOD("get_positions_matching_all_current_rules"), &YCellularAutomata::get_positions_matching_all_current_rules);
    ClassDB::bind_method(D_METHOD("get_positions_matching_any_current_rules"), &YCellularAutomata::get_positions_matching_any_current_rules);
    ClassDB::bind_method(D_METHOD("expand_air_area"), &YCellularAutomata::expand_air_area);

    ClassDB::bind_method(D_METHOD("copy_to_yarn_voxel", "yarn_voxel", "height", "voxel_scale", "density_blend_factor", "perlin_amplitude", "perlin_frequency", "noise_band", "use_3d_noise"), &YCellularAutomata::copy_to_yarn_voxel, DEFVAL(0), DEFVAL(1), DEFVAL(0.5f), DEFVAL(0.5f), DEFVAL(0.1f), DEFVAL(1.0f), DEFVAL(false));
    ClassDB::bind_method(D_METHOD("fill_region_in_yarn_voxel", "yarn_voxel", "fill_with_type", "height", "voxel_scale"), &YCellularAutomata::fill_region_in_yarn_voxel, DEFVAL(0), DEFVAL(1), DEFVAL(1));
    ClassDB::bind_method(D_METHOD("debug_sdf_region_to_image", "start", "size"), &YCellularAutomata::debug_sdf_region_to_image, DEFVAL(Vector2i(0, 0)), DEFVAL(Vector2i(WORLD_SIZE_X, WORLD_SIZE_Y)));
    ClassDB::bind_method(D_METHOD("compute_sdf_for_region", "start", "size"), &YCellularAutomata::compute_sdf_for_region_gdscript, DEFVAL(Vector2i(0, 0)), DEFVAL(Vector2i(WORLD_SIZE_X, WORLD_SIZE_Y)));

    BIND_ENUM_CONSTANT(CellularRule::RuleType::RULE_SURROUNDING_COUNT);
    BIND_ENUM_CONSTANT(CellularRule::RuleType::RULE_SURROUNDING_COUNT_LESS);
    BIND_ENUM_CONSTANT(CellularRule::RuleType::RULE_SURROUNDING_TYPE_LESS);
    BIND_ENUM_CONSTANT(CellularRule::RuleType::RULE_SURROUNDING_TYPE_WITH_EXCEPTION);
    BIND_ENUM_CONSTANT(CellularRule::RuleType::RULE_RANDOM_CHANCE);
    BIND_ENUM_CONSTANT(CellularRule::RuleType::RULE_AVOID_OPPOSITE_DIAGONAL);
    BIND_ENUM_CONSTANT(CellularRule::RuleType::RULE_AVOID_OPPOSITE_CROSS);
    BIND_ENUM_CONSTANT(CellularRule::RuleType::RULE_SURROUNDING_EMPTY);
    BIND_ENUM_CONSTANT(CellularRule::RuleType::RULE_SURROUNDING_TYPE);
    BIND_ENUM_CONSTANT(CellularRule::RuleType::RULE_NO_SPECIFIC_TYPE);
}

// Configuration methods
void YCellularAutomata::set_config(const CellularConfig& new_config) {
    config = new_config;
}

CellularConfig YCellularAutomata::get_config() const {
    return config;
}

void YCellularAutomata::set_iterations(int iterations) {
    config.iterations = MAX(1, iterations);
}

int YCellularAutomata::get_iterations() const {
    return config.iterations;
}

void YCellularAutomata::set_neighborhood_size(int size) {
    config.neighborhood_size = MAX(1, size);
}

int YCellularAutomata::get_neighborhood_size() const {
    return config.neighborhood_size;
}

void YCellularAutomata::set_wrap_edges(bool wrap) {
    config.wrap_edges = wrap;
}

bool YCellularAutomata::get_wrap_edges() const {
    return config.wrap_edges;
}

void YCellularAutomata::set_random_seed(float seed) {
    config.random_seed = seed;
    rng->set_seed(seed);
}

float YCellularAutomata::get_random_seed() const {
    return config.random_seed;
}

// Rule management
void YCellularAutomata::add_rule(const CellularRule& rule) {
    config.rules.push_back(rule);
}

void YCellularAutomata::remove_rule(int index) {
    if (index >= 0 && index < config.rules.size()) {
        config.rules.remove_at(index);
    }
}

void YCellularAutomata::clear_rules() {
    config.rules.clear();
}

CellularRule YCellularAutomata::get_rule(int index) const {
    if (index >= 0 && index < config.rules.size()) {
        return config.rules[index];
    }
    return CellularRule();
}

void YCellularAutomata::set_rule(int index, const CellularRule& rule) {
    if (index >= 0 && index < config.rules.size()) {
        config.rules.write[index] = rule;
    }
}

Dictionary YCellularAutomata::get_rule_dict(int index) const {
    if (index >= 0 && index < config.rules.size()) {
        Dictionary return_dict;
        return_dict["type"] = config.rules[index].type;
        return_dict["threshold"] = config.rules[index].threshold;
        return_dict["target_type"] = config.rules[index].target_type;
        return_dict["source_type"] = config.rules[index].source_type;
        return_dict["neighbor_type"] = config.rules[index].neighbor_type;
        return_dict["override_range"] = config.rules[index].override_range;
        return_dict["override_forbidden_range"] = config.rules[index].override_forbidden_range;
        return_dict["probability"] = config.rules[index].probability;
        return return_dict;
    }
    return Dictionary();
}


void YCellularAutomata::set_rule_dict(int index, const Dictionary& rule) {
    if (index >= 0 && index < config.rules.size()) {
        Dictionary original_rule = get_rule_dict(index);
        for (int i = 0; i < rule.size(); i++) {
            original_rule[rule.get_key_at_index(i)] = rule.get_value_at_index(i);
        }
        CellularRule new_rule;
        new_rule.type = (CellularRule::RuleType)original_rule["type"];
        new_rule.threshold = original_rule["threshold"];
        new_rule.target_type = original_rule["target_type"];
        new_rule.source_type = original_rule["source_type"];
        new_rule.neighbor_type = original_rule["neighbor_type"];
        new_rule.override_range = original_rule["override_range"];
        new_rule.override_forbidden_range = original_rule["override_forbidden_range"];
        new_rule.probability = original_rule["probability"];
        config.rules.write[index] = new_rule;
    }
}

int YCellularAutomata::get_rule_count() const {
    return config.rules.size();
}

// Convenience methods for common rules
void YCellularAutomata::add_surrounding_count_rule(int threshold, uint8_t target_type, uint8_t source_type, uint8_t override_range, float probability) {
    CellularRule rule;
    rule.type = CellularRule::RULE_SURROUNDING_COUNT;
    rule.threshold = threshold;
    rule.target_type = target_type;
    rule.source_type = source_type;
    rule.override_range = override_range;
    rule.probability = probability;
    
    rule.also_count_modified_state = false;
    add_rule(rule);
}

void YCellularAutomata::add_surrounding_count_less_rule(int threshold, uint8_t target_type, uint8_t source_type, uint8_t override_range, float probability) {
    CellularRule rule;
    rule.type = CellularRule::RULE_SURROUNDING_COUNT_LESS;
    rule.threshold = threshold;
    rule.target_type = target_type;
    rule.source_type = source_type;
    rule.override_range = override_range;
    rule.probability = probability;
    
    add_rule(rule);
}

void YCellularAutomata::add_surrounding_empty_rule(int threshold, uint8_t target_type, uint8_t source_type, uint8_t override_range, float probability) {
    CellularRule rule;
    rule.type = CellularRule::RULE_SURROUNDING_EMPTY;
    rule.threshold = threshold;
    rule.target_type = target_type;
    rule.source_type = source_type;
    rule.override_range = override_range;
    rule.probability = probability;
    
    add_rule(rule);
}

void YCellularAutomata::add_surrounding_type_rule(int threshold, uint8_t target_type, uint8_t source_type, uint8_t neighbor_type, uint8_t override_range, uint8_t override_forbidden_range, float probability) {
    CellularRule rule;
    rule.type = CellularRule::RULE_SURROUNDING_TYPE;
    rule.threshold = threshold;
    rule.target_type = target_type;
    rule.source_type = source_type;
    rule.neighbor_type = neighbor_type;
    rule.override_range = override_range;
    rule.override_forbidden_range = override_forbidden_range;
    rule.probability = probability;
    add_rule(rule);
}

void YCellularAutomata::add_surrounding_type_less_rule(int threshold, uint8_t target_type, uint8_t source_type, uint8_t neighbor_type, uint8_t override_range, uint8_t override_forbidden_range, float probability) {
    CellularRule rule;
    rule.type = CellularRule::RULE_SURROUNDING_TYPE_LESS;
    rule.threshold = threshold;
    rule.target_type = target_type;
    rule.source_type = source_type;
    rule.neighbor_type = neighbor_type;
    rule.override_range = override_range;
    rule.override_forbidden_range = override_forbidden_range;
    rule.probability = probability; 
    
    add_rule(rule);
}

void YCellularAutomata::add_avoid_opposite_diagonal_rule(uint8_t target_type, uint8_t source_type, uint8_t look_for_type, uint8_t override_range) {
    CellularRule rule;
    rule.type = CellularRule::RULE_AVOID_OPPOSITE_DIAGONAL;
    rule.target_type = target_type;
    rule.source_type = source_type;
    rule.neighbor_type = look_for_type;
    rule.override_range = override_range;
    rule.also_count_modified_state = true;
    add_rule(rule);
}

void YCellularAutomata::add_avoid_opposite_cross_rule(uint8_t target_type, uint8_t source_type, uint8_t look_for_type, uint8_t override_range) {
    CellularRule rule;
    rule.type = CellularRule::RULE_AVOID_OPPOSITE_CROSS;
    rule.target_type = target_type;
    rule.source_type = source_type;
    rule.neighbor_type = look_for_type;
    rule.override_range = override_range;
    rule.also_count_modified_state = true;
    add_rule(rule);
}

void YCellularAutomata::add_no_specific_type_rule(uint8_t target_type, uint8_t source_type, uint8_t forbidden_type, uint8_t override_range, float probability) {
    CellularRule rule;
    rule.type = CellularRule::RULE_NO_SPECIFIC_TYPE;
    rule.target_type = target_type;
    rule.source_type = source_type;
    rule.forbidden_type = forbidden_type;
    rule.override_range = override_range;
    rule.probability = probability;
    
    add_rule(rule);
}

void YCellularAutomata::add_surrounding_with_exception_rule(int threshold, uint8_t target_type, uint8_t source_type, uint8_t neighbor_type, uint8_t forbidden_type, uint8_t override_range, uint8_t override_forbidden_range, float probability) {
    CellularRule rule;
    rule.type = CellularRule::RULE_SURROUNDING_TYPE_WITH_EXCEPTION;
    rule.threshold = threshold;
    rule.target_type = target_type;
    rule.source_type = source_type;
    rule.forbidden_type = forbidden_type;
    rule.override_range = override_range;
    rule.override_forbidden_range = override_forbidden_range;
    rule.probability = probability;
    
    add_rule(rule);
}

void YCellularAutomata::add_random_chance_rule(float probability, uint8_t target_type, uint8_t source_type) {
    CellularRule rule;
    rule.type = CellularRule::RULE_RANDOM_CHANCE;
    rule.probability = CLAMP(probability, 0.0f, 1.0f);
    rule.target_type = target_type;
    rule.source_type = source_type;
    
    add_rule(rule);
}

// Region-based processing methods
void YCellularAutomata::initialize_world() {
    // Cleanup existing arrays
    if (world_state) {
        delete[] world_state;
        world_state = nullptr;
    }
    if (next_state) {
        delete[] next_state;
        next_state = nullptr;
    }
    if (locked_cells) {
        delete[] locked_cells;
        locked_cells = nullptr;
    }
    
    // Allocate new 1D arrays for the entire world
    int total_size = WORLD_SIZE_X * WORLD_SIZE_Y;
    world_state = new uint8_t[total_size];
    std::fill_n(world_state, total_size, 1); // Initialize to 1
    next_state = new uint8_t[total_size]();
    locked_cells = new bool[total_size](); // () initializes to false
}

void YCellularAutomata::set_working_region(const GenerationRegion& region) {
    current_region = region;
}

void YCellularAutomata::set_working_region_gdscript(Vector2i starting_pos, Vector2i size) {
    current_region = GenerationRegion(starting_pos, size);
}

void YCellularAutomata::fill_region_with_random(float initial_chance_for_wall) {
    if (!world_state) {
        print_error("YCellularAutomata: Cannot fill region with random - world not initialized. Call initialize_world() first.");
        return;
    }
    
    for (int x = current_region.start_pos.x; x < current_region.start_pos.x + current_region.size.x; x++) {
        for (int y = current_region.start_pos.y; y < current_region.start_pos.y + current_region.size.y; y++) {
            int index = get_index(x, y);
            if (locked_cells[index]) {
                continue;
            }
            world_state[index] = get_random_value(Vector2i(x, y)) < initial_chance_for_wall ? 1 : 0;
        }
    }
}

Dictionary YCellularAutomata::get_working_region_dict() const {
    Dictionary return_dict;
    return_dict["start_pos"] = current_region.start_pos;
    return_dict["size"] = current_region.size;
    return return_dict;
}

GenerationRegion YCellularAutomata::get_working_region() const {
    return current_region;
}

void YCellularAutomata::set_initial_region_state(const uint8_t* initial_data) {
    if (!world_state || !initial_data) {
        return;
    }
    
    // Copy data only for the current region
    for (int x = current_region.start_pos.x; x < current_region.start_pos.x + current_region.size.x; x++) {
        for (int y = current_region.start_pos.y; y < current_region.start_pos.y + current_region.size.y; y++) {
            if (is_position_valid(Vector2i(x, y))) {
                int index = get_index(x, y);
                world_state[index] = initial_data[index];
            }
        }
    }
}

TypedArray<Vector2i> YCellularAutomata::get_positions_matching_all_current_rules() {
    TypedArray<Vector2i> return_array;
    for (int x = current_region.start_pos.x; x < current_region.start_pos.x + current_region.size.x; x++) {
        for (int y = current_region.start_pos.y; y < current_region.start_pos.y + current_region.size.y; y++) {
            bool failed = false;
            for (const CellularRule& rule : config.rules) {
                if (!should_apply_rule(rule, Vector2i(x, y))) {
                    failed = true;
                    break;
                }
            }
            if (!failed) {
                return_array.append(Vector2i(x, y));
            }
        }
    }
    return return_array;
}

TypedArray<Vector2i> YCellularAutomata::get_positions_matching_any_current_rules() {
    TypedArray<Vector2i> return_array;
    for (int x = current_region.start_pos.x; x < current_region.start_pos.x + current_region.size.x; x++) {
        for (int y = current_region.start_pos.y; y < current_region.start_pos.y + current_region.size.y; y++) {
            for (const CellularRule& rule : config.rules) {
                if (should_apply_rule(rule, Vector2i(x, y))) {
                    return_array.append(Vector2i(x, y));
                    break;
                }
            }
        }
    }
    return return_array;
}

uint8_t* YCellularAutomata::get_world_state() {
    return world_state;
}

Array YCellularAutomata::get_world_state_gdscript() {
    Array return_array;
    if (!world_state) {
        print_error("YCellularAutomata: Cannot get world state - world not initialized. Call initialize_world() first.");
        return return_array;
    }
    
    return_array.resize(WORLD_SIZE_X);
    for (int x = 0; x < WORLD_SIZE_X; x++) {
        Array return_array_x;
        return_array_x.resize(WORLD_SIZE_Y);
        for (int y = 0; y < WORLD_SIZE_Y; y++) {
            int index = get_index(x, y);
            return_array_x[y] = world_state[index];
        }
        return_array[x] = return_array_x;
    }
    return return_array;
}

// Utility methods
void YCellularAutomata::reset_world() {
    if (world_state) {
        delete[] world_state;
        world_state = nullptr;
    }
    if (next_state) {
        delete[] next_state;
        next_state = nullptr;
    }
    if (locked_cells) {
        delete[] locked_cells;
        locked_cells = nullptr;
    }
}

bool YCellularAutomata::is_initialized() const {
    return world_state != nullptr;
}

Vector2i YCellularAutomata::get_world_size() const {
    return Vector2i(WORLD_SIZE_X, WORLD_SIZE_Y);
}

// Helper methods
int YCellularAutomata::count_neighbors(const Vector2i& pos, uint8_t target_type, int range, bool also_count_on_next_state) {
    int count = 0;
    int next_state_count = 0;
    if (range <= 0) {
        range = config.neighborhood_size;
    }
    
    for (int dx = -range; dx <= range; dx++) {
        for (int dy = -range; dy <= range; dy++) {
            if (dx == 0 && dy == 0) {
                continue; // Skip the center cell
            }
            
            Vector2i neighbor_pos = pos + Vector2i(dx, dy);
            
            // Handle edge wrapping
            if (config.wrap_edges) {
                neighbor_pos = wrap_position(neighbor_pos);
            } else if (!is_position_valid(neighbor_pos)) {
                if (target_type == 1 || target_type == 255) { // Count if we're looking for wall cells or any type of filled cell
                    count++;
                    next_state_count++;
                }
                continue;
            }
            
            int iterations = (also_count_on_next_state == true ? 2 : 1);
            for (int i = 0; i < iterations; i++) {
                int index = get_index(neighbor_pos);
                const uint8_t& neighbor = i == 0 ? world_state[index] : next_state[index];
                if (target_type == 0) { // Count empty cells
                    if (neighbor == 0) {
                        if (i == 0) {
                            count++;
                        } else {
                            next_state_count++;
                        }
                    }
                } else if (target_type == 255) { // Count any type of filled cell
                    if (neighbor != 0) {
                        if (i == 0) {
                            count++;
                        } else {
                            next_state_count++;
                        }
                    }
                } else {
                    if (neighbor == target_type) { // Count specific type of filled cell
                        if (i == 0) {
                            count++;
                        } else {
                            next_state_count++;
                        }
                    }
                }
            }
        }
    }
    
    return std::max(count, next_state_count);
}

bool YCellularAutomata::has_neighbor_of_type(const Vector2i& pos, uint8_t type, int range, bool also_count_on_next_state) {
    if (range <= 0) {
        range = config.neighborhood_size;
    }
    
    for (int dx = -range; dx < range + 1; dx++) {
        for (int dy = -range; dy < range + 1; dy++) {
            if (dx == 0 && dy == 0) {
                continue;
            }
            Vector2i neighbor_pos = pos + Vector2i(dx, dy);
            
            if (config.wrap_edges) {
                neighbor_pos = wrap_position(neighbor_pos);
            } else if (!is_position_valid(neighbor_pos)) {
                continue;
            }
            int iterations = (also_count_on_next_state == true ? 2 : 1);
            for (int i = 0; i < iterations; i++) {
                int index = get_index(neighbor_pos);
                const uint8_t& neighbor = i == 0 ? world_state[index] : next_state[index];
                if (neighbor == type) {
                    return true;
                }
            }
        }
    }
    
    return false;
}

bool YCellularAutomata::is_position_valid(const Vector2i& pos) const {
    return pos.x >= 0 && pos.x < WORLD_SIZE_X &&
           pos.y >= 0 && pos.y < WORLD_SIZE_Y;
}

bool YCellularAutomata::is_position_in_region(const Vector2i& pos) const {
    return pos.x >= current_region.start_pos.x && pos.x < current_region.start_pos.x + current_region.size.x &&
           pos.y >= current_region.start_pos.y && pos.y < current_region.start_pos.y + current_region.size.y;
}

Vector2i YCellularAutomata::wrap_position(const Vector2i& pos) {
    Vector2i wrapped = pos;
    
    if (wrapped.x < 0) wrapped.x = WORLD_SIZE_X - 1;
    else if (wrapped.x >= WORLD_SIZE_X) wrapped.x = 0;
    
    if (wrapped.y < 0) wrapped.y = WORLD_SIZE_Y - 1;
    else if (wrapped.y >= WORLD_SIZE_Y) wrapped.y = 0;
    
    return wrapped;
}

void YCellularAutomata::fill_region_edges_with(uint8_t type, int width) {
    // Fill all 4 edges with type, and width.
    for (int x = current_region.start_pos.x; x < current_region.start_pos.x + current_region.size.x; x++) {
        for (int extra_width = 0; extra_width < width; extra_width++) {
            int index = get_index(x, current_region.start_pos.y + extra_width);
            if (locked_cells[index]) {
                continue;
            }
            world_state[index] = type;
            index = get_index(x, current_region.start_pos.y + current_region.size.y - extra_width - 1);
            if (locked_cells[index]) {
                continue;
            }
            world_state[index] = type;
        }
    }
    for (int y = current_region.start_pos.y; y < current_region.start_pos.y + current_region.size.y; y++) {
        for (int extra_width = 0; extra_width < width; extra_width++) {
            int index = get_index(current_region.start_pos.x + extra_width, y);
            if (locked_cells[index]) {
                continue;
            }
            world_state[index] = type;
            index = get_index(current_region.start_pos.x + current_region.size.x - extra_width - 1, y);
            if (locked_cells[index]) {
                continue;
            }
            world_state[index] = type;
        }
    }
}

float YCellularAutomata::get_random_value(const Vector2i& pos) {
    // Use position and seed to generate consistent random values
    return rng->randf();
}

bool YCellularAutomata::should_apply_rule(const CellularRule& rule, const Vector2i& pos) {
    int index = get_index(pos);
    const uint8_t& current_cell = world_state[index];
    
    
    // Check source type filter
    if (current_cell != rule.source_type) {
        return false;
    }
    
    switch (rule.type) {
        case CellularRule::RULE_SURROUNDING_COUNT: {
            int neighbor_count = count_neighbors(pos, 255, rule.override_range, rule.also_count_modified_state); // Count filled cells
            // print_line(vformat("Checking rule: %s threshold: %d target_type: %d source_type: %d override_range: %d probability: %f neighbor_count: %d", rule.type, rule.threshold, rule.target_type, rule.source_type, rule.override_range, rule.probability, neighbor_count));
            return neighbor_count >= rule.threshold && (rule.probability >= 0.999f || get_random_value(pos) <= rule.probability);
        }
        case CellularRule::RULE_SURROUNDING_COUNT_LESS: {
            int neighbor_count = count_neighbors(pos, 255, rule.override_range, rule.also_count_modified_state); // Count filled cells
            // print_line(vformat("Checking rule: %s threshold: %d target_type: %d source_type: %d override_range: %d probability: %f neighbor_count: %d", rule.type, rule.threshold, rule.target_type, rule.source_type, rule.override_range, rule.probability, neighbor_count));
            return neighbor_count < rule.threshold && (rule.probability >= 0.999f || get_random_value(pos) <= rule.probability);
        }
        case CellularRule::RULE_SURROUNDING_EMPTY: {
            int empty_count = count_neighbors(pos, 0, rule.override_range, rule.also_count_modified_state); // Count empty cells
            return empty_count >= rule.threshold && (rule.probability >= 0.999f || get_random_value(pos) <= rule.probability);
        }
        
        case CellularRule::RULE_SURROUNDING_TYPE: {
            int type_count = count_neighbors(pos, rule.neighbor_type != 0 ? rule.neighbor_type : rule.target_type, rule.override_range, rule.also_count_modified_state);
            return type_count >= rule.threshold && (rule.probability >= 0.999f || get_random_value(pos) <= rule.probability);
        }

        case CellularRule::RULE_SURROUNDING_TYPE_LESS: {
            int type_count = count_neighbors(pos, rule.neighbor_type != 0 ? rule.neighbor_type : rule.target_type, rule.override_range, rule.also_count_modified_state);
            return type_count < rule.threshold && (rule.probability >= 0.999f || get_random_value(pos) <= rule.probability);
        }

        case CellularRule::RULE_NO_SPECIFIC_TYPE: {
            return !has_neighbor_of_type(pos, rule.neighbor_type != 0 ? rule.neighbor_type : rule.target_type, rule.override_range, rule.also_count_modified_state) && (rule.probability >= 0.999f || get_random_value(pos) <= rule.probability);
        }
        
        case CellularRule::RULE_SURROUNDING_TYPE_WITH_EXCEPTION: {
            // Special case: if there are X filled cells around but no treasure cells
            int filled_count = count_neighbors(pos, rule.neighbor_type != 0 ? rule.neighbor_type : rule.target_type, rule.override_range, rule.also_count_modified_state);
            bool has_forbidden_type = has_neighbor_of_type(pos, rule.forbidden_type, rule.override_forbidden_range, rule.also_count_modified_state);
            return filled_count >= rule.threshold && !has_forbidden_type && (rule.probability >= 0.999f || get_random_value(pos) <= rule.probability);
        }
        
        case CellularRule::RULE_RANDOM_CHANCE: {
            float random_val = get_random_value(pos);
            return random_val <= rule.probability;
        }
        
        case CellularRule::RULE_AVOID_OPPOSITE_DIAGONAL: {
            // This rule exists to check for the following case:
            // Being that wall is # and air is . and _ being either wall or air or anything else
            // ___      .#_
            // _.#   or #._   or any permutation where there's a diagonal wall and opposite diagonal air.
            // _#.      ___

            // Check if current cell is source
            if (current_cell != rule.source_type) {
                return false; // Not source, so rule doesn't apply
            }
            
            // Check 1: Are there 2 or more walls around me?
            int wall_count = count_neighbors(pos, rule.neighbor_type, 1, rule.also_count_modified_state); // Count filled cells in immediate neighborhood
            if (wall_count < 2) {
                return false; // Not enough walls around
            }
            
            // Check 2 & 3: Look for diagonal air pockets with walls on both sides
            Vector2i diagonal_offsets[4] = {
                Vector2i(-1, -1), Vector2i(1, -1),  // Top-left, Top-right
                Vector2i(-1, 1),  Vector2i(1, 1)    // Bottom-left, Bottom-right
            };
            
            for (int i = 0; i < 4; i++) {
                Vector2i diagonal_pos = pos + diagonal_offsets[i];
                
                // Check if diagonal position is valid and contains air
                if (!is_position_valid(diagonal_pos) || !is_position_in_region(diagonal_pos)) {
                    continue;
                }
                
                int diagonal_index = get_index(diagonal_pos);
                if (world_state[diagonal_index] != rule.source_type) {
                    continue; // Not source type, so rule doesn't apply
                }
                
                // Check if there are walls on both sides towards the diagonal air
                Vector2i side1, side2;
                
                switch (i) {
                    case 0: // Top-left diagonal
                        side1 = pos + Vector2i(-1, 0); // Left
                        side2 = pos + Vector2i(0, -1); // Top
                        break;
                    case 1: // Top-right diagonal
                        side1 = pos + Vector2i(1, 0);  // Right
                        side2 = pos + Vector2i(0, -1); // Top
                        break;
                    case 2: // Bottom-left diagonal
                        side1 = pos + Vector2i(-1, 0); // Left
                        side2 = pos + Vector2i(0, 1);  // Bottom
                        break;
                    case 3: // Bottom-right diagonal
                        side1 = pos + Vector2i(1, 0);  // Right
                        side2 = pos + Vector2i(0, 1);  // Bottom
                        break;
                }
                
                // Check if both sides have walls
                bool side1_wall = false, side2_wall = false;
                
                if (is_position_valid(side1) && is_position_in_region(side1)) {
                    int side1_index = get_index(side1);
                    side1_wall = (world_state[side1_index] == rule.neighbor_type);
                }
                
                if (is_position_valid(side2) && is_position_in_region(side2)) {
                    int side2_index = get_index(side2);
                    side2_wall = (world_state[side2_index] == rule.neighbor_type);
                }
                
                // If we found a diagonal air pocket with walls on both sides, this rule should apply
                if (side1_wall && side2_wall) {
                    return true;
                }
            }
            return false; // No problematic diagonal air pattern found
        }

        case CellularRule::RULE_AVOID_OPPOSITE_CROSS: {
            // This rule exists to check for the following case:
            // Being that wall is # and air is . and _ being either wall or air or anything else
            // _#_      _._
            // ...   or ###   or any permutation where there's a cross of either wall or air, with the opposite thing on the opposite cross.
            // _#_      _._
            
            // Check if current cell is the target type we're looking for
            if (current_cell != rule.source_type) {
                return false;
            }

            uint8_t opposite_type = rule.neighbor_type;
            // Define the cross pattern positions (horizontal and vertical)
            Vector2i cross_positions[4] = {
                Vector2i(-1, 0),  // Left
                Vector2i(1, 0),   // Right
                Vector2i(0, -1),  // Top
                Vector2i(0, 1)    // Bottom
            };
            
            // Check for cross pattern
            bool has_horizontal_cross = false;
            bool has_vertical_cross = false;
            bool has_opposite_horizontal = false;
            bool has_opposite_vertical = false;
            
            // Check horizontal cross (left and right)
            if (is_position_valid(pos + cross_positions[0]) && is_position_in_region(pos + cross_positions[0]) &&
                is_position_valid(pos + cross_positions[1]) && is_position_in_region(pos + cross_positions[1])) {
                
                int left_index = get_index(pos + cross_positions[0]);
                int right_index = get_index(pos + cross_positions[1]);
                
                uint8_t left_cell = world_state[left_index];
                uint8_t right_cell = world_state[right_index];
                
                // Check if both sides have the same type as current cell (forming a horizontal cross)
                if (left_cell == current_cell && right_cell == current_cell) {
                    has_horizontal_cross = true;
                }
                
                // Check if both sides have the opposite type (0 for air, anything else for filled)
                if (left_cell == opposite_type && right_cell == opposite_type) {
                    has_opposite_horizontal = true;
                }
            }

            // Check vertical cross (top and bottom)
            if (is_position_valid(pos + cross_positions[2]) && is_position_in_region(pos + cross_positions[2]) &&
                is_position_valid(pos + cross_positions[3]) && is_position_in_region(pos + cross_positions[3])) {
                
                int top_index = get_index(pos + cross_positions[2]);
                int bottom_index = get_index(pos + cross_positions[3]);
                
                uint8_t top_cell = world_state[top_index];
                uint8_t bottom_cell = world_state[bottom_index];
                
                // Check if both sides have the same type as current cell (forming a vertical cross)
                if (top_cell == current_cell && bottom_cell == current_cell) {
                    has_vertical_cross = true;
                }
                
                // Check if both sides have the opposite type
                if (top_cell == opposite_type && bottom_cell == opposite_type) {
                    has_opposite_vertical = true;
                }
            }
            
            // Rule applies if we have a cross pattern AND the opposite cross pattern exists
            bool has_cross = has_horizontal_cross || has_vertical_cross;
            bool has_opposite_cross = has_opposite_horizontal || has_opposite_vertical;
            
            return (has_cross && has_opposite_cross);
        }
                
        default:
            return false;
    }
}

void YCellularAutomata::apply_rule(const CellularRule& rule, const Vector2i& pos) {
    uint8_t& next_cell = next_state[get_index(pos)];

    if ((rule.type == CellularRule::RULE_AVOID_OPPOSITE_CROSS || rule.type == CellularRule::RULE_AVOID_OPPOSITE_DIAGONAL) && rule.override_range > 1) {
        set_area_to_type(pos, rule.target_type, rule.override_range, true);
        return;
    }
    next_cell = rule.target_type;
}

void YCellularAutomata::copy_region_state() {
    for (int x = current_region.start_pos.x; x < current_region.start_pos.x + current_region.size.x; x++) {
        for (int y = current_region.start_pos.y; y < current_region.start_pos.y + current_region.size.y; y++) {
            int index = get_index(x, y);
            next_state[index] = world_state[index];
        }
    }
}

String YCellularAutomata::get_region_state_as_string() {
    String result;
    for (int y = current_region.start_pos.y; y < current_region.start_pos.y + current_region.size.y; y++) {
        String line;
        for (int x = current_region.start_pos.x; x < current_region.start_pos.x + current_region.size.x; x++) {
            int index = get_index(x, y);
            uint8_t value = world_state[index];
            switch(value) {
                case 0:
                    line += ".";
                    break;
                case 1:
                    line += "#";
                    break;
                default:
                    line += String::chr('A' + (value - 2)); // A,B,C etc for values > 1
                    break;
            }
        }
        result += line + "\n";
    }
    return result;
}

// Debug methods
void YCellularAutomata::print_region_state() const {
    print_line("Cellular Automata Region State:");
    print_line("Region: ", current_region.start_pos, " to ", 
               current_region.start_pos + current_region.size);
    
    for (int y = current_region.start_pos.y; y < current_region.start_pos.y + current_region.size.y; y++) {
        String line;
        for (int x = current_region.start_pos.x; x < current_region.start_pos.x + current_region.size.x; x++) {
            int index = get_index(x, y);
            uint8_t value = world_state[index];
            switch(value) {
                case 0:
                    line += ".";
                    break;
                case 1:
                    line += "#";
                    break;
                default:
                    line += String::chr('A' + (value - 2)); // A,B,C etc for values > 1
                    break;
            }
        }
        print_line(line);
    }
}

void YCellularAutomata::print_rules() const {
    print_line("Cellular Automata Rules:");
    for (int i = 0; i < config.rules.size(); i++) {
        const CellularRule& rule = config.rules[i];
        print_line("Rule ", i, ": Type=", rule.type, 
                   " Threshold=", rule.threshold,
                   " Target=", rule.target_type,
                   " Source=", rule.source_type);
    }
}



void YCellularAutomata::expand_air_area() {
     if (!world_state || !next_state || !locked_cells) {
        print_error("YCellularAutomata: Cannot process generation phase - world not properly initialized.");
        return;
    }
    
    copy_region_state();
    for (int x = current_region.start_pos.x; x < current_region.start_pos.x + current_region.size.x; x++) {
        for (int y = current_region.start_pos.y; y < current_region.start_pos.y + current_region.size.y; y++) {
            int index = get_index(x, y);
            Vector2i pos(x, y);

            if (x < 2 || x > WORLD_SIZE_X - 3 || y < 2 || y > WORLD_SIZE_Y - 3) {
                continue;
            }
            
            // Skip if cell is locked
            if (locked_cells[index]) {
                continue;
            }

            // Check if cell is wall
            if (world_state[index] == 1) {
                // Check 4 neighbors (von Neumann neighborhood)
                Vector2i neighbors[4] = {
                    pos + Vector2i(1, 0), pos + Vector2i(-1, 0),
                    pos + Vector2i(0, 1), pos + Vector2i(0, -1)
                };

                for (int i = 0; i < 4; i++) {
                    Vector2i neighbor = neighbors[i];
                    int neighbor_index = get_index(neighbor);
                    if (!locked_cells[neighbor_index] && world_state[neighbor_index] == 0) {
                        next_state[index] = 0;
                        break;
                    }
                }
            }
        }
    }

    // Swap states
    uint8_t* temp = world_state;
    world_state = next_state;
    next_state = temp;
}

void YCellularAutomata::process_rules(int iterations, bool debug_printing) {
    if (!world_state || !next_state || !locked_cells) {
        print_error("YCellularAutomata: Cannot process generation phase - world not properly initialized.");
        return;
    }
    
    for (int i = 0; i < iterations; i++) {
        copy_region_state();
        
        for (int x = current_region.start_pos.x; x < current_region.start_pos.x + current_region.size.x; x++) {
            for (int y = current_region.start_pos.y; y < current_region.start_pos.y + current_region.size.y; y++) {
                int index = get_index(x, y);
                Vector2i pos(x, y);
                
                // Skip if cell is locked
                if (locked_cells[index]) {
                    continue;
                }
                
                // Only apply generation phase rules
                for (const CellularRule& rule : config.rules) {
                    if (should_apply_rule(rule, pos)) {
                        apply_rule(rule, pos);
                        break;
                    }
                }
            }
        }
        
        // Swap states
        uint8_t* temp = world_state;
        world_state = next_state;
        next_state = temp;

        if (debug_printing) {
            print_region_state();
        }
    }
}

Vector<Vector<Vector2i>> YCellularAutomata::get_all_isolated_air_pockets(Vector2i start_pos, Vector2i size) {
    Vector<Vector<Vector2i>> return_array;
    
    // Allocate visited array
    bool* visited = new bool[WORLD_SIZE_X * WORLD_SIZE_Y];
    
    // Initialize visited array
    for (int x = 0; x < WORLD_SIZE_X; x++) {
        for (int y = 0; y < WORLD_SIZE_Y; y++) {
            visited[get_index(x, y)] = false;
        }
    }

    // Find all air cells in the region
    Vector<Vector2i> all_air_cells;
    for (int x = start_pos.x; x < start_pos.x + size.x; x++) {
        for (int y = start_pos.y; y < start_pos.y + size.y; y++) {
            int index = get_index(x, y);
            if (world_state[index] == 0) { // Air
                all_air_cells.push_back(Vector2i(x, y));
            }
        }
    }
    
    // Find all connected components (air pockets)
    for (const Vector2i& start_pos : all_air_cells) {
        int start_index = get_index(start_pos);
        if (visited[start_index]) {
            continue; // Already visited this component
        }
        
        Vector<Vector2i> component;
        Vector<Vector2i> component_queue;
        component_queue.push_back(start_pos);
        visited[start_index] = true;
        
        // Flood fill this component
        while (!component_queue.is_empty()) {
            Vector2i pos = component_queue[0];
            component_queue.remove_at(0);
            component.push_back(pos);
            
            // Check 4 neighbors (von Neumann neighborhood)
            Vector2i neighbors[4] = {
                pos + Vector2i(1, 0), pos + Vector2i(-1, 0),
                pos + Vector2i(0, 1), pos + Vector2i(0, -1)
            };
            
            for (int i = 0; i < 4; i++) {
                Vector2i neighbor = neighbors[i];
                int neighbor_index = get_index(neighbor);
                if (is_position_valid(neighbor) && is_position_in_region(neighbor) && 
                    !visited[neighbor_index] &&
                    world_state[neighbor_index] == 0) {
                    visited[neighbor_index] = true;
                    component_queue.push_back(neighbor);
                }
            }
        }
        
        // Add this air pocket to the return array
        if (!component.is_empty()) {
            return_array.push_back(component);
        }
    }
    
    delete[] visited;
    return return_array;
}

Vector<Vector2i> YCellularAutomata::flood_fill_isolated_air(Vector2i start_pos, Vector2i size, bool fill_with_walls) {
    Vector<Vector2i> return_array;
    
    // Temporarily set the working region to the specified region
    GenerationRegion original_region = current_region;
    current_region = GenerationRegion(start_pos, size);
    
    // Get all air pockets using the new method
    Vector<Vector<Vector2i>> air_pockets = get_all_isolated_air_pockets(start_pos, size);
    
    // Find the largest air pocket (assumed to be the main connected region)
    Vector<Vector2i> largest_component;
    if (!air_pockets.is_empty()) {
        int largest_component_size = 0;
        for (const Vector<Vector2i>& pocket : air_pockets) {
            if (pocket.size() > largest_component_size) {
                largest_component_size = pocket.size();
                largest_component = pocket;
            }
        }
    }
    
    // Create a set of positions in the largest component for fast lookup
    HashSet<Vector2i> largest_component_set;
    for (const Vector2i& pos : largest_component) {
        largest_component_set.insert(pos);
    }
    
    // Process all air cells in the region
    for (int x = start_pos.x; x < start_pos.x + size.x; x++) {
        for (int y = start_pos.y; y < start_pos.y + size.y; y++) {
            int index = get_index(x, y);
            if (world_state[index] == 0) { // Air cell
                Vector2i pos(x, y);
                if (largest_component_set.has(pos)) {
                    // This is part of the main connected air region
                    return_array.append(pos);
                } else {
                    // This is isolated air
                    if (fill_with_walls && !locked_cells[index]) {
                        world_state[index] = 1; // Fill with wall
                    }
                }
            }
        }
    }
    
    // Restore original working region
    current_region = original_region;
    
    return return_array;
}

void YCellularAutomata::connect_all_isolated_air_pockets(int connection_width, bool prioritize_largest) {
    Vector<Vector<Vector2i>> air_pockets = get_all_isolated_air_pockets(current_region.start_pos, current_region.size);
    if (air_pockets.size() <= 1) {
        return;
    }
    int attempts = 100;
    while (air_pockets.size() > 1 && attempts > 0) {
        int connect_index_from = 0;
        int connect_index_to = 1;
        if (prioritize_largest) {
            int largest_pocket_index = 0;
            int largest_pocket_size = air_pockets[0].size();
            for (int i = 1; i < air_pockets.size(); i++) {
                if (air_pockets[i].size() > largest_pocket_size) {
                    largest_pocket_index = i;
                    largest_pocket_size = air_pockets[i].size();
                }
            }
            connect_index_from = largest_pocket_index;
            if (connect_index_from == connect_index_to) {
                connect_index_to = (connect_index_to + 1) % air_pockets.size();
            }
        }
        connect_air_pocket_to_other_air_pocket(air_pockets[connect_index_from], air_pockets[connect_index_to], connection_width);
        air_pockets = get_all_isolated_air_pockets(current_region.start_pos, current_region.size);
        attempts--;
    }
}

void YCellularAutomata::connect_region_to_other_region(Vector2i starting_pos, Vector2i size, Vector2i other_starting_pos, Vector2i other_size, int connection_width) {
    // Use flood fill to identify which air areas belong to each region
    Vector<Vector2i> region1_air_cells = get_air_cells_belonging_to_region(starting_pos, size);
    Vector<Vector2i> region2_air_cells = get_air_cells_belonging_to_region(other_starting_pos, other_size);
    connect_air_pocket_to_other_air_pocket(region1_air_cells, region2_air_cells, connection_width);
}

void YCellularAutomata::connect_air_pocket_to_other_air_pocket(Vector<Vector2i> air_pocket, Vector<Vector2i> other_air_pocket, int connection_width) {
    // print_line("Region 1 air cells: ", air_pocket.size(), " Region 2 air cells: ", other_air_pocket.size());
    if (air_pocket.is_empty() || other_air_pocket.is_empty()) {
        // print_line("No air cells to connect");
        return; // No air cells to connect
    }
    if (air_pocket == other_air_pocket) {
        // print_line("Air pockets are the same");
        return;
    }
    
    // Find the closest pair of air cells between the two regions
    Vector2i closest_cell1, closest_cell2;
    float min_distance = FLT_MAX;
    
    for (const Vector2i& cell1 : air_pocket) {
        for (const Vector2i& cell2 : other_air_pocket) {
            float distance = cell1.distance_to(cell2);
            if (distance < min_distance) {
                min_distance = distance;
                closest_cell1 = cell1;
                closest_cell2 = cell2;
            }
        }
    }

    // print_line("Connecting regions with closest air cells: ", closest_cell1, " and ", closest_cell2);
    // Carve a path between the two closest air cells
    carve_path_between_cells(closest_cell1, closest_cell2, connection_width);
}

Vector<Vector2i> YCellularAutomata::get_air_cells_belonging_to_region(Vector2i region_start, Vector2i region_size) {
    return flood_fill_isolated_air(region_start, region_size, false);
}

void YCellularAutomata::carve_path_between_cells(Vector2i start_cell, Vector2i end_cell, int path_width) {
    set_line_to_type(start_cell, end_cell, 0, path_width);
}

void YCellularAutomata::set_area_to_type(Vector2i position, uint8_t type, int width, bool circle) {
    int width_half = width / 2;
    int width_squared = width_half * width_half;
    for (int dx = -width_half; dx <= width_half; dx++) {
        int dx_squared = dx * dx;
        for (int dy = -width_half; dy <= width_half; dy++) {
            int dy_squared = dy * dy;
            if (!circle || dx_squared + dy_squared <= width_squared) {
                Vector2i carve_pos = position + Vector2i(dx, dy);
                if (is_position_valid(carve_pos)) {
                    int index = get_index(carve_pos);
                    if (!locked_cells[index]) {
                        world_state[index] = type; // Carve type
                    }
                }
            }
        }
    }
}

void YCellularAutomata::set_line_to_type(Vector2i start, Vector2i end, uint8_t type, int path_width, bool circle) {
    Vector<Vector2i> path = get_line_path(start, end);
    for (const Vector2i& path_cell : path) {
        if (path_width == 1) {
            int index = get_index(path_cell);
            if (is_position_valid(path_cell) && !locked_cells[index]) {
                world_state[index] = type;
            }
        } else {
            int width_half = path_width / 2;
            int width_squared = width_half * width_half;
            if (circle) {
                // Carve path_width around each path cell
                for (int dx = -width_half; dx <= width_half; dx++) {
                    int dx_squared = dx * dx;
                    for (int dy = -width_half; dy <= width_half; dy++) {
                        int dy_squared = dy * dy;
                        if (dx_squared + dy_squared <= width_squared) {
                            Vector2i carve_pos = path_cell + Vector2i(dx, dy);
                            if (is_position_valid(carve_pos)) {
                                int index = get_index(carve_pos);
                                if (!locked_cells[index]) {
                                    world_state[index] = type; // Carve type
                                }
                            }
                        }
                    }
                }
            } else {
                // Carve path_width around each path cell
                for (int dx = -width_half; dx <= width_half; dx++) {
                    for (int dy = -width_half; dy <= width_half; dy++) {
                        Vector2i carve_pos = path_cell + Vector2i(dx, dy);
                        if (is_position_valid(carve_pos)) {
                            int index = get_index(carve_pos);
                            if (!locked_cells[index]) {
                                world_state[index] = type; // Carve type
                            }
                        }
                    }
                }
            }
        }
    }
}

Vector<Vector2i> YCellularAutomata::get_line_path(Vector2i start, Vector2i end) {
    Vector<Vector2i> path;
    
    // Simple Bresenham's line algorithm
    int x0 = start.x, y0 = start.y;
    int x1 = end.x, y1 = end.y;
    
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    
    while (true) {
        path.push_back(Vector2i(x0, y0));
        
        if (x0 == x1 && y0 == y1) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
    
    return path;
}

void YCellularAutomata::lock_region_filled_cells() {
    for (int x = current_region.start_pos.x; x < current_region.start_pos.x + current_region.size.x; x++) {
        for (int y = current_region.start_pos.y; y < current_region.start_pos.y + current_region.size.y; y++) {
            int index = get_index(x, y);
            // Lock all air, non-wall cells
            if (world_state[index] == 0 && world_state[index] != 1) { // Air and not Wall.
                locked_cells[index] = true;
            }
        }
    }
}

// Locking system methods
void YCellularAutomata::lock_cell(const Vector2i& pos) {
    if (is_position_valid(pos)) {
        locked_cells[get_index(pos)] = true;
    }
}

void YCellularAutomata::unlock_cell(const Vector2i& pos) {
    if (is_position_valid(pos)) {
        locked_cells[get_index(pos)] = false;
    }
}

bool YCellularAutomata::is_cell_locked(const Vector2i& pos) const {
    if (is_position_valid(pos)) {
        return locked_cells[get_index(pos)];
    }
    return false;
}

bool* YCellularAutomata::get_locked_cells() {
    return locked_cells;
}

Array YCellularAutomata::get_locked_cells_gdscript() {
    Array return_array;
    return_array.resize(WORLD_SIZE_X);
    for (int x = 0; x < WORLD_SIZE_X; x++) {
        for (int y = 0; y < WORLD_SIZE_Y; y++) {
            int index = get_index(x, y);
            return_array[index] = locked_cells[index];
        }
    }   
    return return_array;
}

void YCellularAutomata::fill_region_in_yarn_voxel(YarnVoxel* yarn_voxel, uint8_t fill_with_type, int height, int voxel_scale) {
    if (!yarn_voxel) {
        print_error("YCellularAutomata: Cannot fill region in YarnVoxel - yarn_voxel is null");
        return;
    }
    voxel_scale = MAX(1, voxel_scale);

    Vector3i chunk_number = yarn_voxel->FindChunkNumberFromPosition(Vector3(current_region.start_pos.x, height, current_region.start_pos.y));
    YVoxelChunk* modify_chunk = yarn_voxel->get_chunk(chunk_number);
    if (!modify_chunk) return;

    int voxel_start_x = current_region.start_pos.x * voxel_scale;
    int voxel_start_z = current_region.start_pos.y * voxel_scale;
    int voxel_end_x = (current_region.start_pos.x + current_region.size.x) * voxel_scale;
    int voxel_end_z = (current_region.start_pos.y + current_region.size.y) * voxel_scale;

    for (int voxel_x = voxel_start_x; voxel_x < voxel_end_x; voxel_x++) {
        for (int voxel_z = voxel_start_z; voxel_z < voxel_end_z; voxel_z++) {
            Vector3i check_chunk_number = yarn_voxel->FindChunkNumberFromPosition(Vector3(voxel_x, height, voxel_z));
            if (check_chunk_number != chunk_number) {
                modify_chunk->deferred_set_dirty();
                chunk_number = check_chunk_number;
                modify_chunk = yarn_voxel->get_chunk(chunk_number);
                if (!modify_chunk) continue;
            }

            Vector3i point_position = yarn_voxel->FindPointNumberFromPosition(Vector3(voxel_x, height, voxel_z));
            if (!modify_chunk->is_point_position_in_range_without_neighbours(point_position.x, point_position.y, point_position.z)) {
                continue;
            }

            auto& point_value = modify_chunk->points[point_position.x][point_position.y][point_position.z];
            point_value.byteValue = fill_with_type;
            point_value.floatValue = INT16_MIN;

            if (yarn_voxel->IsPointNumberInBoundary(point_position)) {
                modify_chunk->AttemptSetDirtyNeighbour(point_position);
            }
        }
    }
    modify_chunk->deferred_set_dirty();
}

// 1. Create banded grid for voxels
uint8_t* YCellularAutomata::create_banded_grid_for_voxels() {
    uint8_t* banded_grid = new uint8_t[WORLD_SIZE_X * WORLD_SIZE_Y];

    // First pass: Copy CA state to banded grid.
    // 0 = air, 1 = solid (initially)
    for (int i = 0; i < WORLD_SIZE_X * WORLD_SIZE_Y; ++i) {
        banded_grid[i] = (world_state[i] == 0) ? 0 : 1;
    }

    // Second pass: Mark solid cells adjacent to air as band 2 (boundary).
    for (int y = 0; y < WORLD_SIZE_Y; ++y) {
        for (int x = 0; x < WORLD_SIZE_X; ++x) {
            int idx = y * WORLD_SIZE_X + x;
            if (banded_grid[idx] == 1) {
                for (int dy = -1; dy <= 1; ++dy) {
                    for (int dx = -1; dx <= 1; ++dx) {
                        if (dx == 0 && dy == 0) continue;
                        int nx = x + dx, ny = y + dy;
                        if (nx >= 0 && nx < WORLD_SIZE_X && ny >= 0 && ny < WORLD_SIZE_Y) {
                            if (banded_grid[ny * WORLD_SIZE_X + nx] == 0) {
                                banded_grid[idx] = 2; // Mark as boundary
                                goto next2;
                            }
                        }
                    }
                }
            }
            next2:;
        }
    }

    // Third pass: Mark solid cells adjacent to band 2 as band 3 (one step deeper).
    for (int y = 0; y < WORLD_SIZE_Y; ++y) {
        for (int x = 0; x < WORLD_SIZE_X; ++x) {
            int idx = y * WORLD_SIZE_X + x;
            if (banded_grid[idx] == 1) {
                for (int dy = -1; dy <= 1; ++dy) {
                    for (int dx = -1; dx <= 1; ++dx) {
                        if (dx == 0 && dy == 0) continue;
                        int nx = x + dx, ny = y + dy;
                        if (nx >= 0 && nx < WORLD_SIZE_X && ny >= 0 && ny < WORLD_SIZE_Y) {
                            if (banded_grid[ny * WORLD_SIZE_X + nx] == 2) {
                                banded_grid[idx] = 3;
                                goto next3;
                            }
                        }
                    }
                }
            }
            next3:;
        }
    }

    // Fourth pass: Mark solid cells adjacent to band 3 as band 4 (even deeper).
    for (int y = 0; y < WORLD_SIZE_Y; ++y) {
        for (int x = 0; x < WORLD_SIZE_X; ++x) {
            int idx = y * WORLD_SIZE_X + x;
            if (banded_grid[idx] == 1) {
                for (int dy = -1; dy <= 1; ++dy) {
                    for (int dx = -1; dx <= 1; ++dx) {
                        if (dx == 0 && dy == 0) continue;
                        int nx = x + dx, ny = y + dy;
                        if (nx >= 0 && nx < WORLD_SIZE_X && ny >= 0 && ny < WORLD_SIZE_Y) {
                            if (banded_grid[ny * WORLD_SIZE_X + nx] == 3) {
                                banded_grid[idx] = 4;
                                goto next4;
                            }
                        }
                    }
                }
            }
            next4:;
        }
    }

    // Fifth pass: Any remaining solid (not marked as 2,3,4) is deep solid (band 5).
    for (int i = 0; i < WORLD_SIZE_X * WORLD_SIZE_Y; ++i) {
        if (banded_grid[i] == 1) banded_grid[i] = 5;
    }
    return banded_grid;
}

TypedArray<float> YCellularAutomata::compute_sdf_for_region_gdscript(Vector2i start, Vector2i size) {
    Vector<float> sdf = compute_sdf_for_region(start, size, world_state);
    TypedArray<float> sdf_array;
    for (int i = 0; i < sdf.size(); ++i) {
        sdf_array.push_back(sdf[i]);
    }
    return sdf_array;
}
// Compute SDF for a region (does not affect global sdf_state)
Vector<float> YCellularAutomata::compute_sdf_for_region(Vector2i start, Vector2i size, const uint8_t* grid) {
    int sx = start.x, sy = start.y, w = size.x, h = size.y;
    Vector<float> sdf;
    sdf.resize(w * h);
    // First pass: boundary detection
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int gx = sx + x, gy = sy + y;
            int idx = y * w + x;
            uint8_t val = grid[gy * WORLD_SIZE_X + gx];
            bool is_boundary = false;
            for (int dy = -1; dy <= 1 && !is_boundary; ++dy) {
                for (int dx = -1; dx <= 1 && !is_boundary; ++dx) {
                    if (dx == 0 && dy == 0) continue;
                    int nx = gx + dx, ny = gy + dy;
                    bool neighbor_is_wall;
                    if (nx < sx || nx >= sx + w || ny < sy || ny >= sy + h) {
                        neighbor_is_wall = true; // Outside region: treat as wall
                    } else {
                        neighbor_is_wall = (grid[ny * WORLD_SIZE_X + nx] != 0);
                    }
                    if (neighbor_is_wall != (val != 0)) {
                        is_boundary = true;
                    }
                }
            }
            if (is_boundary) {
                sdf.write[idx] = 0.0f;
            } else {
                sdf.write[idx] = 1e6f;
            }
        }
    }
    // Forward pass
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx = y * w + x;
            if (sdf[idx] == 0.0f) continue;
            float min_dist = sdf[idx];
            if (x > 0) min_dist = MIN(min_dist, sdf[y * w + (x - 1)] + 1.0f);
            if (y > 0) min_dist = MIN(min_dist, sdf[(y - 1) * w + x] + 1.0f);
            if (x > 0 && y > 0) min_dist = MIN(min_dist, sdf[(y - 1) * w + (x - 1)] + 1.4142f);
            if (x < w - 1 && y > 0) min_dist = MIN(min_dist, sdf[(y - 1) * w + (x + 1)] + 1.4142f);
            sdf.write[idx] = min_dist;
        }
    }
    // Backward pass
    for (int y = h - 1; y >= 0; --y) {
        for (int x = w - 1; x >= 0; --x) {
            int idx = y * w + x;
            float min_dist = sdf[idx];
            if (x < w - 1) min_dist = MIN(min_dist, sdf[y * w + (x + 1)] + 1.0f);
            if (y < h - 1) min_dist = MIN(min_dist, sdf[(y + 1) * w + x] + 1.0f);
            if (x < w - 1 && y < h - 1) min_dist = MIN(min_dist, sdf[(y + 1) * w + (x + 1)] + 1.4142f);
            if (x > 0 && y < h - 1) min_dist = MIN(min_dist, sdf[(y + 1) * w + (x - 1)] + 1.4142f);
            sdf.write[idx] = min_dist;
        }
    }
    // Assign sign: negative inside solid, positive inside air
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int gx = sx + x, gy = sy + y;
            int idx = y * w + x;
            if (grid[gy * WORLD_SIZE_X + gx] != 0) {
                sdf.write[idx] = -sdf[idx];
            }
        }
    }
    return sdf;
}

// Debug: create grayscale image of SDF for a region
Ref<Image> YCellularAutomata::debug_sdf_region_to_image(Vector2i start, Vector2i size) {
    Vector<float> sdf = compute_sdf_for_region(start, size, world_state);
    gaussian_blur_sdf(sdf, size.x, size.y, 1.0f, 2);
    int w = size.x, h = size.y;
    // Find max abs for normalization
    float max_abs = 1.0f;
    for (int i = 0; i < w * h; ++i) {
        float abs_val = Math::abs(sdf[i]);
        if (abs_val > max_abs) max_abs = abs_val;
    }
    PackedByteArray data;
    for (int i = 0; i < w * h; ++i) {
        float norm = (sdf[i] / (2.0f * max_abs)) + 0.5f; // [-max,max] -> [0,1]
        norm = CLAMP(norm, 0.0f, 1.0f);
        data.push_back(static_cast<uint8_t>(norm * 255.0f));
    }
    return Image::create_from_data(w, h, false, Image::FORMAT_L8, data);
}

// Helper: Apply Gaussian blur to a 2D SDF array (in-place, separable)
void YCellularAutomata::gaussian_blur_sdf(Vector<float>& sdf, int w, int h, float sigma, int radius) {
    // Create Gaussian kernel
    Vector<float> kernel;
    kernel.resize(2 * radius + 1);
    float sum = 0.0f;
    for (int i = -radius; i <= radius; ++i) {
        float v = Math::exp(-0.5f * (i * i) / (sigma * sigma));
        kernel.write[i + radius] = v;
        sum += v;
    }
    for (int i = 0; i < kernel.size(); ++i) kernel.write[i] /= sum;
    // Temp buffer
    Vector<float> temp;
    temp.resize(w * h);
    // Horizontal pass
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            float acc = 0.0f;
            for (int k = -radius; k <= radius; ++k) {
                int xx = CLAMP(x + k, 0, w - 1);
                acc += sdf[y * w + xx] * kernel[k + radius];
            }
            temp.write[y * w + x] = acc;
        }
    }
    // Vertical pass
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            float acc = 0.0f;
            for (int k = -radius; k <= radius; ++k) {
                int yy = CLAMP(y + k, 0, h - 1);
                acc += temp[yy * w + x] * kernel[k + radius];
            }
            sdf.write[y * w + x] = acc;
        }
    }
}

void YCellularAutomata::copy_to_yarn_voxel(YarnVoxel* yarn_voxel, int height, int voxel_scale, float density_blend_factor, float perlin_amplitude, float perlin_frequency, float noise_band, bool use_3d_noise) {
    if (!yarn_voxel) {
        print_error("YCellularAutomata: Cannot copy to YarnVoxel - yarn_voxel is null");
        return;
    }
    voxel_scale = MAX(1, voxel_scale);
    density_blend_factor = CLAMP(density_blend_factor, 0.0f, 1.0f);

    // Create banded grid and compute SDF for the region only
    uint8_t* banded_grid = create_banded_grid_for_voxels();
    Vector<float> region_sdf = compute_sdf_for_region(current_region.start_pos, current_region.size, banded_grid);
    // Apply Gaussian blur to SDF for softer boundaries
    gaussian_blur_sdf(region_sdf, current_region.size.x, current_region.size.y, 1.0f, 2);

    Vector3i chunk_number = yarn_voxel->FindChunkNumberFromPosition(Vector3(current_region.start_pos.x, height, current_region.start_pos.y));
    YVoxelChunk* modify_chunk = yarn_voxel->get_chunk(chunk_number);
    if (!modify_chunk) { delete[] banded_grid; return; }

    int voxel_start_x = current_region.start_pos.x * voxel_scale;
    int voxel_start_z = current_region.start_pos.y * voxel_scale;
    int voxel_end_x = (current_region.start_pos.x + current_region.size.x) * voxel_scale;
    int voxel_end_z = (current_region.start_pos.y + current_region.size.y) * voxel_scale;

    for (int voxel_x = voxel_start_x; voxel_x < voxel_end_x; voxel_x++) {
        for (int voxel_z = voxel_start_z; voxel_z < voxel_end_z; voxel_z++) {
            float ca_xf = float(voxel_x) / float(voxel_scale);
            float ca_zf = float(voxel_z) / float(voxel_scale);
            int ca_x0 = int(floor(ca_xf));
            int ca_z0 = int(floor(ca_zf));
            int ca_x1 = ca_x0 + 1;
            int ca_z1 = ca_z0 + 1;
            ca_x0 = CLAMP(ca_x0, 0, WORLD_SIZE_X - 1);
            ca_x1 = CLAMP(ca_x1, 0, WORLD_SIZE_X - 1);
            ca_z0 = CLAMP(ca_z0, 0, WORLD_SIZE_Y - 1);
            ca_z1 = CLAMP(ca_z1, 0, WORLD_SIZE_Y - 1);
            float fx = ca_xf - ca_x0;
            float fz = ca_zf - ca_z0;
            // Bilinear interpolation of blurred region SDF
            int region_x0 = ca_x0 - current_region.start_pos.x;
            int region_z0 = ca_z0 - current_region.start_pos.y;
            int region_x1 = ca_x1 - current_region.start_pos.x;
            int region_z1 = ca_z1 - current_region.start_pos.y;
            // Clamp to region bounds
            region_x0 = CLAMP(region_x0, 0, current_region.size.x - 1);
            region_x1 = CLAMP(region_x1, 0, current_region.size.x - 1);
            region_z0 = CLAMP(region_z0, 0, current_region.size.y - 1);
            region_z1 = CLAMP(region_z1, 0, current_region.size.y - 1);
            float sdf00 = region_sdf[region_z0 * current_region.size.x + region_x0];
            float sdf10 = region_sdf[region_z0 * current_region.size.x + region_x1];
            float sdf01 = region_sdf[region_z1 * current_region.size.x + region_x0];
            float sdf11 = region_sdf[region_z1 * current_region.size.x + region_x1];
            float sdf_interp = (1 - fx) * (1 - fz) * sdf00 + fx * (1 - fz) * sdf10 + (1 - fx) * fz * sdf01 + fx * fz * sdf11;
            
            float density_without_sdf = world_state[ca_z0 * WORLD_SIZE_X + ca_x0] == 0 ? 1.0f : -1.0f;

            // Apply Perlin noise in a slim band near the boundary
            if (perlin_amplitude > 0.0f && Math::abs(sdf_interp) < noise_band) {
                float blend = 1.0f - (Math::abs(sdf_interp) / noise_band);
                blend = CLAMP(blend, 0.0f, 1.0f);
                float noise = use_3d_noise ? YarnVoxel::static_perlin_noise_3d(voxel_x * perlin_frequency, height * perlin_frequency, voxel_z * perlin_frequency) : YarnVoxel::static_perlin_noise(voxel_x * perlin_frequency, voxel_z * perlin_frequency);
                sdf_interp += noise * perlin_amplitude * blend;
            }

            Vector3i check_chunk_number = yarn_voxel->FindChunkNumberFromPosition(Vector3(voxel_x, height, voxel_z));
            if (check_chunk_number != chunk_number) {
                modify_chunk->deferred_set_dirty();
                chunk_number = check_chunk_number;
                modify_chunk = yarn_voxel->get_chunk(chunk_number);
                if (!modify_chunk) continue;
            }
            Vector3i point_position = yarn_voxel->FindPointNumberFromPosition(Vector3(voxel_x, height, voxel_z));
            if (!modify_chunk->is_point_position_in_range_without_neighbours(point_position.x, point_position.y, point_position.z)) {
                continue;
            }
            auto& point_value = modify_chunk->points[point_position.x][point_position.y][point_position.z];

            float density = density_without_sdf;
            // Optionally blend with blurred SDF for extra smoothness at the boundary
            float max_sdf = 8.0f;
            float t = CLAMP((sdf_interp + max_sdf) / (2.0f * max_sdf), 0.0f, 1.0f);
            float sdf_density = Math::lerp(-1.0f, 1.0f, t);
            density = Math::lerp(density, sdf_density, density_blend_factor);
            point_value.byteValue = density <= 0.0f ? 1 : 0;
            point_value.floatValue = floatToInt16(density);
            if (yarn_voxel->IsPointNumberInBoundary(point_position)) {
                modify_chunk->AttemptSetDirtyNeighbour(point_position);
            }
        }
    }
    modify_chunk->deferred_set_dirty();
    delete[] banded_grid;
}