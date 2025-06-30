# Tag-Based Semantic Material System

The CGame engine now uses a sophisticated tag-based system for automatic material and color generation. This system allows artists and designers to control the visual appearance of assets through simple metadata tags without touching any code.

## How It Works

1. **Asset Metadata**: Each mesh has a `metadata.json` file with tags
2. **Material Definitions**: The `assets/material_definitions.json` file defines color palettes and material properties for each tag
3. **Priority System**: Tags have priority levels - higher priority tags override lower priority ones
4. **Automatic Generation**: During asset compilation, the system automatically generates:
   - Semantic color palettes for texture templates
   - MTL files with appropriate material properties
   - Multi-texture support with proper emission, roughness, etc.

## Example Usage

### Mesh Metadata (assets/meshes/props/sun/metadata.json)
```json
{
    "name": "Sun",
    "tags": ["environment", "star", "celestial"],
    "description": "A glowing sun mesh with bright texture for space environments"
}
```

### Generated Material Properties
- **Highest Priority Tag**: `star` (priority: 100)
- **Generated Colors**: Bright gold, orange, red-orange gradients
- **Material Properties**: High shininess, strong emission, bright ambient/diffuse
- **Result**: Glowing stellar object with realistic emission lighting

## Available Tags

### High Priority (Unique Appearance)
- `star` (100): Bright stellar objects with emission
- `sun` (100): Solar bodies with strong emission  
- `engine` (95): Engine components with heat emission
- `celestial` (90): Rocky planetary bodies
- `military` (85): Military and combat vehicles

### Medium Priority (Type-Based)
- `spacecraft` (80): Advanced spacecraft with metallic hull
- `ship` (75): General purpose vessels
- `vehicle` (70): General vehicles and transports
- `building` (60): Architectural structures
- `spaceport` (58): Spaceport facilities and infrastructure
- `structure` (55): Industrial and utility structures

### Low Priority (Environment/Modifiers)
- `environment` (40): Natural terrain and landscape
- `improved` (10): Enhanced/upgraded versions with accent colors

## Customization

To add new material types or modify existing ones:

1. Edit `assets/material_definitions.json`
2. Add your tag with priority, colors, and material properties
3. Recompile assets with `make clean && make`
4. The new materials will automatically be applied to any assets with matching tags

## Color System

Each tag definition includes:
- **Primary**: Main surface color
- **Secondary**: Accent/detail color  
- **Tertiary**: Highlight/special feature color
- **Accent**: Trim/edge color

## Material Properties

Each tag includes full PBR-style material definitions:
- **Ambient**: Base lighting color
- **Diffuse**: Primary surface color
- **Specular**: Reflection color and intensity
- **Shininess**: Surface roughness (higher = smoother)
- **Emission**: Self-illumination for glowing objects

## Benefits

- **Artist-Friendly**: No code changes needed for new materials
- **Consistent**: Automatic color coordination across related objects
- **Flexible**: Mix and match tags for complex material behaviors
- **Professional**: Supports modern PBR-style material properties
- **Data-Driven**: All definitions stored in easily-editable JSON files
