#ifndef YCELULAR_AUTOMATA_H
#define YCELULAR_AUTOMATA_H

#include "core/object/ref_counted.h"
#include "core/variant/variant.h"
#include "core/templates/vector.h"
#include "core/math/vector2i.h"
#include "core/object/object.h"
#include "core/math/random_number_generator.h"
#include "core/math/math_funcs.h"
#include "core/variant/typed_array.h"
#include "yarnvoxel.h"
#include "yvoxelchunk.h"
#include "core/io/image.h"

// Forward declarations
class YCellularAutomata;
class YarnVoxel;
class YVoxelChunk;

// Region specification for progressive generation
struct GenerationRegion {
    Vector2i start_pos;           // Starting position (e.g., 512, 512)
    Vector2i size;                // Size of the region (e.g., 128, 128)
    
    GenerationRegion() : start_pos(0, 0), size(0, 0) {}
    GenerationRegion(Vector2i start, Vector2i region_size) 
        : start_pos(start), size(region_size) {}
};

// Rule structure for special cases
struct CellularRule {
    enum RuleType {
        RULE_SURROUNDING_COUNT,                 // If X filled cells around, do Y
        RULE_SURROUNDING_COUNT_LESS,                 // If less than X filled cells around, do Y
        RULE_SURROUNDING_EMPTY,                 // If X empty cells around, do Y
        RULE_SURROUNDING_TYPE,                  // If X cells of specific type around, do Y
        RULE_SURROUNDING_TYPE_LESS,                  // If less than X cells of specific type around, do Y
        RULE_NO_SPECIFIC_TYPE,                  // If no cells of specific type around, do Y
        RULE_SURROUNDING_TYPE_WITH_EXCEPTION,    // If X cells of specific type around but not others of other specific type, do Y
        RULE_RANDOM_CHANCE,                     // Random chance to apply rule
        RULE_AVOID_OPPOSITE_DIAGONAL,
        RULE_AVOID_OPPOSITE_CROSS
    };

    RuleType type;
    int threshold;                   // Number of cells required
    uint8_t target_type;             // Type to look for or convert to
    uint8_t source_type;             // Source type to convert from (0 = any)
    uint8_t neighbor_type;           // Type to look for in neighbors (for RULE_SURROUNDING_TYPE)
    uint8_t forbidden_type;          // Type that should not be present (for RULE_NO_SPECIFIC_TYPE, RULE_SURROUNDING_TYPE_WITH_EXCEPTION)
    uint8_t override_range;              // Override the range for the rule (for RULE_SURROUNDING_COUNT, RULE_SURROUNDING_EMPTY, RULE_SURROUNDING_TYPE)
    uint8_t override_forbidden_range;   // Override the range for the forbidden type (for RULE_SURROUNDING_TYPE_WITH_EXCEPTION)
    bool also_count_modified_state;  // Whether to count on next state (for RULE_SURROUNDING_COUNT, RULE_SURROUNDING_EMPTY, RULE_SURROUNDING_TYPE)
    float probability;               // Probability for random rules
    CellularRule() : type(RULE_SURROUNDING_COUNT), threshold(0), target_type(0),
                     source_type(0), neighbor_type(0), forbidden_type(0), override_range(0),
                     override_forbidden_range(0), also_count_modified_state(false), probability(1.0f)
                     {}
};

// Cellular automata configuration
struct CellularConfig {
    int iterations;                  // Number of iterations to run
    int neighborhood_size;           // Size of neighborhood (1 = 3x3, 2 = 5x5, etc.)
    bool wrap_edges;                 // Whether to wrap around chunk edges
    float random_seed;               // Random seed for consistent generation
    Vector<CellularRule> rules;      // List of rules to apply
    
    CellularConfig() : iterations(5), neighborhood_size(1),
                       wrap_edges(false), random_seed(0.0f) {}
};

class YCellularAutomata : public RefCounted {
    GDCLASS(YCellularAutomata, RefCounted);

protected:
    static void _bind_methods();

    CellularConfig config;
    
    // Fixed size world array (1024x1024)
    static const int WORLD_SIZE_X = 512;
    static const int WORLD_SIZE_Y = 512;
    
    // Current working region
    GenerationRegion current_region;
    
    // Use 1D arrays for the entire world with index calculations
    uint8_t* world_state;
    uint8_t* next_state;
    bool* locked_cells;
    
    // Helper methods for 1D array indexing
    inline int get_index(int x, int y) const { return y * WORLD_SIZE_X + x; }
    inline int get_index(const Vector2i& pos) const { return pos.y * WORLD_SIZE_X + pos.x; }
    
    // Helper methods
    int count_neighbors(const Vector2i& pos, uint8_t target_type = 0, int range = 0, bool also_count_on_next_state = false);
    bool has_neighbor_of_type(const Vector2i& pos, uint8_t type, int range = 0, bool also_count_on_next_state = false);
    bool is_position_valid(const Vector2i& pos) const;
    bool is_position_in_region(const Vector2i& pos) const;
    Vector2i wrap_position(const Vector2i& pos);
    float get_random_value(const Vector2i& pos);
    bool should_apply_rule(const CellularRule& rule, const Vector2i& pos);
    void apply_rule(const CellularRule& rule, const Vector2i& pos);
    void copy_region_state();
    
    // Phase-specific methods
    void process_rules(int iterations = 1, bool debug_printing = false);

    Vector<Vector<Vector2i>> get_all_isolated_air_pockets(Vector2i start_pos, Vector2i size);
    Vector<Vector2i> flood_fill_isolated_air(Vector2i start_pos, Vector2i size, bool fill_with_walls = true);
    void connect_all_isolated_air_pockets(int connection_width = 3, bool prioritize_largest = true);
    void lock_filled_cells();
    void carve_path_between_cells(Vector2i start_cell, Vector2i end_cell, int path_width);
    void set_line_to_type(Vector2i start, Vector2i end, uint8_t type, int path_width, bool circle = true);
    void set_area_to_type(Vector2i position, uint8_t type, int width, bool circle = true);
    Vector<Vector2i> get_line_path(Vector2i start, Vector2i end);
    Vector<Vector2i> get_air_cells_belonging_to_region(Vector2i region_start, Vector2i region_size);

public:
    YCellularAutomata();
    ~YCellularAutomata();

	Ref<RandomNumberGenerator> rng;

    uint8_t* create_banded_grid_for_voxels();

    // Configuration methods
    void set_config(const CellularConfig& new_config);
    CellularConfig get_config() const;
    
    void set_iterations(int iterations);
    int get_iterations() const;
    
    void set_neighborhood_size(int size);
    int get_neighborhood_size() const;

    void set_wrap_edges(bool wrap);
    bool get_wrap_edges() const;
    
    void set_random_seed(float seed);
    float get_random_seed() const;

    // Rule management
    void add_rule(const CellularRule& rule);
    
    // GDScript-friendly rule addition methods
    void add_surrounding_count_rule(int threshold, uint8_t target_type, uint8_t source_type = 0, uint8_t override_range = 0, float probability = 1.0f);
    void add_surrounding_count_less_rule(int threshold, uint8_t target_type, uint8_t source_type = 0, uint8_t override_range = 0, float probability = 1.0f);
    void add_surrounding_type_rule(int threshold, uint8_t target_type, uint8_t source_type, uint8_t neighbor_type, uint8_t override_range = 0, uint8_t override_forbidden_range = 0, float probability = 1.0f);
    void add_surrounding_type_less_rule(int threshold, uint8_t target_type, uint8_t source_type, uint8_t neighbor_type, uint8_t override_range = 0, uint8_t override_forbidden_range = 0, float probability = 1.0f);
    void add_no_specific_type_rule(uint8_t target_type, uint8_t source_type, uint8_t forbidden_type, uint8_t override_range = 0, float probability = 1.0f);
    void add_surrounding_empty_rule(int threshold, uint8_t target_type, uint8_t source_type = 0, uint8_t override_range = 0, float probability = 1.0f);
    void add_surrounding_with_exception_rule(int threshold, uint8_t target_type, uint8_t source_type, uint8_t neighbor_type, uint8_t forbidden_type, uint8_t override_range = 0, uint8_t override_forbidden_range = 0, float probability = 1.0f);
    void add_random_chance_rule(float probability, uint8_t target_type, uint8_t source_type = 0);
    void add_avoid_opposite_diagonal_rule(uint8_t target_type = 0, uint8_t source_type = 0, uint8_t look_for_type = 1, uint8_t override_range = 2);
    void add_avoid_opposite_cross_rule(uint8_t target_type, uint8_t source_type = 0, uint8_t look_for_type = 1, uint8_t override_range = 2);
    
    // Cpp only methods
    void remove_rule(int index);
    void clear_rules();
    CellularRule get_rule(int index) const;
    Dictionary get_rule_dict(int index) const;
    void set_rule(int index, const CellularRule& rule);
    void set_rule_dict(int index, const Dictionary& rule);
    int get_rule_count() const;

    // Region-based processing methods
    void initialize_world();
    void set_working_region(const GenerationRegion& region);
    void set_working_region_gdscript(Vector2i starting_pos, Vector2i size);
    GenerationRegion get_working_region() const;
    Dictionary get_working_region_dict() const;
    void set_initial_region_state(const uint8_t* initial_data);
    uint8_t* get_world_state();
    Array get_world_state_gdscript();
    void connect_region_to_other_region(Vector2i starting_pos, Vector2i size, Vector2i other_starting_pos, Vector2i other_size, int connection_width = 3);
    void connect_air_pocket_to_other_air_pocket(Vector<Vector2i> air_pocket, Vector<Vector2i> other_air_pocket, int connection_width = 3);
    
    void fill_region_with_random(float initial_chance_for_wall = 0.45f);
    void fill_region_edges_with(uint8_t type, int width);
        
    // Locking system
    void lock_cell(const Vector2i& pos);
    void unlock_cell(const Vector2i& pos);
    bool is_cell_locked(const Vector2i& pos) const;
    void lock_region_filled_cells();
    Array get_locked_cells_gdscript();
    bool* get_locked_cells();
    
    // Utility methods
    void reset_world();
    bool is_initialized() const;
    Vector2i get_world_size() const;
    
    TypedArray<Vector2i> get_positions_matching_all_current_rules();
    TypedArray<Vector2i> get_positions_matching_any_current_rules();

    void expand_air_area();
    void copy_to_yarn_voxel(YarnVoxel* yarn_voxel, int height, int voxel_scale = 1, float density_blend_factor = 0.5f, float perlin_amplitude = 0.5f, float perlin_frequency = 0.1f, float noise_band = 1.0f, bool use_3d_noise = false);
    void fill_region_in_yarn_voxel(YarnVoxel* yarn_voxel, uint8_t fill_with_type, int height, int voxel_scale = 1);
    
    // Debug methods
    void print_region_state() const;
    String get_region_state_as_string();
    void print_rules() const;

    void gaussian_blur_sdf(Vector<float>& sdf, int w, int h, float sigma, int radius);

    // Region-based SDF and debug image
    Vector<float> compute_sdf_for_region(Vector2i start, Vector2i size, const uint8_t* grid);
    TypedArray<float> compute_sdf_for_region_gdscript(Vector2i start, Vector2i size);
    Ref<Image> debug_sdf_region_to_image(Vector2i start, Vector2i size);

};

VARIANT_ENUM_CAST(CellularRule::RuleType);

#endif // YCELULAR_AUTOMATA_H