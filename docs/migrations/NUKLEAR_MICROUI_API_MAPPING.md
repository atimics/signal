# Nuklear vs MicroUI: Technical Comparison

## API Mapping Reference

This document provides a quick reference for converting Nuklear code to MicroUI equivalents.

### Window Management

| Nuklear | MicroUI | Notes |
|---------|---------|-------|
| `nk_begin(ctx, "Title", bounds, flags)` | `mu_begin_window(ctx, "Title", rect)` | Simplified flag system |
| `nk_end(ctx)` | `mu_end_window(ctx)` | Direct equivalent |

### Layout System

| Nuklear | MicroUI | Notes |
|---------|---------|-------|
| `nk_layout_row_dynamic(ctx, height, cols)` | `mu_layout_row(ctx, cols, widths, height)` | MicroUI requires width array |
| `nk_layout_row_static(ctx, height, width, cols)` | `mu_layout_row(ctx, cols, (int[]){width, width}, height)` | Use width array |

### Widgets

| Nuklear | MicroUI | Notes |
|---------|---------|-------|
| `nk_button_label(ctx, "Text")` | `mu_button(ctx, "Text")` | Direct equivalent |
| `nk_label(ctx, "Text", align)` | `mu_label(ctx, "Text")` | No alignment parameter |
| `nk_checkbox_label(ctx, "Text", &value)` | `mu_checkbox(ctx, "Text", &value)` | Direct equivalent |

### Text Input

| Nuklear | MicroUI | Notes |
|---------|---------|-------|
| `nk_edit_string(ctx, flags, buffer, &len, max, filter)` | `mu_textbox(ctx, buffer, bufsz)` | Simplified interface |

### Trees and Groups

| Nuklear | MicroUI | Notes |
|---------|---------|-------|
| `nk_tree_push_hashed(ctx, type, title, state, hash)` | `mu_header(ctx, title)` followed by content | Different approach |
| `nk_group_begin(ctx, title, flags)` | Custom container implementation needed | No direct equivalent |

### Custom Drawing

| Nuklear | MicroUI | Notes |
|---------|---------|-------|
| `nk_window_get_canvas(ctx)` | Direct command buffer access | More manual approach |
| `nk_fill_rect(canvas, rect, rounding, color)` | `mu_draw_rect(ctx, rect, color)` | No rounding parameter |
| `nk_stroke_line(canvas, x1, y1, x2, y2, width, color)` | Custom implementation needed | No built-in line drawing |

## Key Architectural Differences

### Nuklear Characteristics
- **Retained Mode Elements**: Some state persistence
- **Complex Styling**: Comprehensive theming system
- **Rich Widgets**: Built-in complex widgets (color picker, property editor)
- **Event System**: Sophisticated input handling
- **Memory Model**: Dynamic allocation with pools

### MicroUI Characteristics  
- **Pure Immediate Mode**: No state persistence
- **Minimal Styling**: Basic color theming only
- **Simple Widgets**: Basic set, extensible
- **Simple Events**: Basic mouse/keyboard only
- **Fixed Memory**: Pre-allocated buffers

## Migration Patterns

### Pattern 1: Simple Widget Replacement
```c
// Nuklear
if (nk_button_label(ctx, "Click Me")) {
    action();
}

// MicroUI
if (mu_button(ctx, "Click Me") & MU_RES_SUBMIT) {
    action();
}
```

### Pattern 2: Layout Conversion
```c
// Nuklear
nk_layout_row_dynamic(ctx, 30, 2);
nk_button_label(ctx, "A");
nk_button_label(ctx, "B");

// MicroUI  
mu_layout_row(ctx, 2, (int[]){-1, -1}, 30);
mu_button(ctx, "A");
mu_button(ctx, "B");
```

### Pattern 3: Window Creation
```c
// Nuklear
if (nk_begin(ctx, "My Window", nk_rect(10, 10, 200, 150),
             NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
    // content
}
nk_end(ctx);

// MicroUI
if (mu_begin_window(ctx, "My Window", mu_rect(10, 10, 200, 150))) {
    // content
    mu_end_window(ctx);
}
```

## Common Gotchas

### 1. Return Value Handling
- Nuklear buttons return `bool` (true if clicked)
- MicroUI returns flags, check with `& MU_RES_SUBMIT`

### 2. Layout Width Arrays
- Nuklear uses column count only
- MicroUI requires explicit width array (`-1` for proportional)

### 3. Color Management
- Nuklear has complex styling with multiple color properties
- MicroUI uses simple enum-based colors

### 4. Text Alignment
- Nuklear has text alignment flags
- MicroUI has fixed left alignment (custom centering needed)

### 5. Input Handling
- Nuklear has comprehensive key constant definitions
- MicroUI has minimal key support (mainly text input)

## Performance Considerations

### Build Size Impact
- Nuklear: ~500KB compiled
- MicroUI: ~2KB compiled
- **Reduction**: ~99.6% smaller

### Runtime Memory
- Nuklear: Dynamic allocation, can grow
- MicroUI: Fixed buffers, predictable usage

### WASM Compatibility
- Nuklear: Requires careful dependency management
- MicroUI: Zero external dependencies

## Conclusion

MicroUI provides 80% of Nuklear's functionality with 1% of the complexity. The migration requires careful attention to:

1. **Layout System Changes**: Width arrays vs. column counts
2. **Return Value Handling**: Flag checking vs. boolean returns  
3. **Custom Drawing**: Manual command buffer vs. canvas API
4. **Styling Limitations**: Simple theming vs. complex styles

The trade-off heavily favors MicroUI for projects requiring WASM compatibility and minimal dependencies.
