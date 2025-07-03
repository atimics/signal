#ifndef SHIP_ASSEMBLY_H
#define SHIP_ASSEMBLY_H

#include "core.h"
#include <ode/ode.h>

// Ship part categories
typedef enum {
    PART_HULL,              // Primary structure/shell
    PART_THRUSTER,          // Propulsion units
    PART_TANK,              // Fuel/oxidizer tanks
    PART_PIPE,              // Fluid transfer systems
    PART_POWER,             // Power generation/storage
    PART_CONTROL,           // Control surfaces/gyros
    PART_SENSOR,            // Sensors/cameras
    PART_DOCKING,           // Docking ports/clamps
    PART_STRUCTURAL,        // Beams/struts/reinforcement
    PART_PAYLOAD            // Cargo/equipment bays
} ShipPartCategory;

// Attachment point types
typedef enum {
    ATTACH_SURFACE,         // Can attach to any surface
    ATTACH_NODE,            // Specific connection nodes
    ATTACH_RADIAL,          // Radial symmetry attachment
    ATTACH_STACK,           // Stack attachment (top/bottom)
    ATTACH_PIPE             // Pipe/fluid connections
} AttachmentType;

// Connection node on a part
typedef struct {
    Vector3 position;       // Local position on part
    Vector3 direction;      // Normal direction
    AttachmentType type;
    float size;            // Connection size (for compatibility)
    bool occupied;         // Is this node connected?
    char* connection_id;   // ID of connected part
} AttachmentNode;

// Ship part definition
typedef struct {
    char* part_id;              // Unique part identifier
    char* display_name;         // Human-readable name
    ShipPartCategory category;
    
    // Physical properties
    float mass;                 // kg
    Vector3 center_of_mass;     // Local COM
    float drag_coefficient;
    float heat_capacity;
    
    // Visual representation
    char* mesh_name;            // Mesh asset name
    char* material_name;        // Material/texture
    Vector3 visual_offset;      // Offset from physics body
    
    // Attachment points
    int num_attachments;
    AttachmentNode* attachment_nodes;
    
    // Functional properties (part-specific)
    void* properties;           // Points to part-specific struct
} ShipPart;

// Assembled ship part instance
typedef struct {
    ShipPart* part_definition;
    
    // Transform relative to ship root
    Vector3 position;
    Quaternion orientation;
    
    // ODE physics
    dBodyID body;               // Part's rigid body
    dJointID* joints;           // Connections to other parts
    int num_joints;
    
    // Runtime state
    float temperature;
    float structural_integrity;
    bool active;
    
    // Connections to other parts
    struct ShipPartInstance** connected_parts;
    int num_connections;
} ShipPartInstance;

// Complete ship assembly
typedef struct {
    char* ship_name;
    EntityID entity_id;
    
    // Root part (usually main hull)
    ShipPartInstance* root_part;
    
    // All parts
    ShipPartInstance** parts;
    int num_parts;
    
    // ODE composite body
    dBodyID composite_body;     // Combined ship body
    dSpaceID collision_space;   // Ship's collision space
    
    // Ship-wide properties
    float total_mass;
    Vector3 center_of_mass;
    Matrix3 inertia_tensor;
    
    // Resource tracking
    float total_thrust_available;
    float power_generation;
    float power_consumption;
} ShipAssembly;

// Part library management
typedef struct {
    ShipPart** parts;
    int num_parts;
    int capacity;
    
    // Categorized access
    ShipPart*** parts_by_category;
    int* counts_by_category;
} ShipPartLibrary;

// Initialize ship part library
bool ship_parts_init(ShipPartLibrary* library);

// Load part definitions from file
bool ship_parts_load_directory(ShipPartLibrary* library, const char* directory);

// Get part by ID
ShipPart* ship_parts_get(ShipPartLibrary* library, const char* part_id);

// Get parts by category
ShipPart** ship_parts_get_category(ShipPartLibrary* library, ShipPartCategory category, int* count);

// Create new ship assembly
ShipAssembly* ship_assembly_create(const char* name);

// Add part to assembly
bool ship_assembly_add_part(ShipAssembly* assembly, 
                           ShipPart* part,
                           ShipPartInstance* attach_to,
                           int attachment_index,
                           const Vector3* position,
                           const Quaternion* orientation);

// Remove part from assembly
bool ship_assembly_remove_part(ShipAssembly* assembly, ShipPartInstance* part);

// Validate assembly (check connections, balance, etc.)
bool ship_assembly_validate(ShipAssembly* assembly);

// Create ODE physics representation
bool ship_assembly_create_physics(ShipAssembly* assembly, struct World* world);

// Update assembly physics
void ship_assembly_update(ShipAssembly* assembly, float delta_time);

// Save/load ship designs
bool ship_assembly_save(ShipAssembly* assembly, const char* filename);
ShipAssembly* ship_assembly_load(const char* filename);

// Cleanup
void ship_assembly_destroy(ShipAssembly* assembly);
void ship_parts_cleanup(ShipPartLibrary* library);

#endif // SHIP_ASSEMBLY_H