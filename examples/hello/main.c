/**
 * JiUI - Hello World Example
 * Demonstrates basic library initialization, type usage, and shutdown.
 */

#include <jiui/jiui.h>
#include <stdio.h>

int main(void) {
    /* Initialize the JiUI library */
    JiResultCode result = ji_initialize();
    if (JI_FAILED(result)) {
        fprintf(stderr, "Failed to initialize JiUI: %s\n",
                ji_result_to_string(result));
        return 1;
    }

    printf("JiUI v%s - Hello World!\n", ji_version());

    /* Demonstrate core types */
    JiPoint origin = ji_point_zero();
    JiSize  size   = ji_size(800.0, 600.0);
    JiRect  window = ji_rect_from_size(size);

    printf("Window: position(%.0f, %.0f) size(%.0fx%.0f)\n",
           window.x, window.y, window.width, window.height);

    /* Thickness for padding */
    JiThickness padding = ji_thickness_all(10.0, 20.0, 10.0, 20.0);
    JiRect content = ji_rect_deflate(window, padding);
    printf("Content: position(%.0f, %.0f) size(%.0fx%.0f)\n",
           content.x, content.y, content.width, content.height);

    /* Vector math */
    JiVector dir = ji_vector(3.0, 4.0);
    printf("Vector(%.1f, %.1f) length = %.2f\n",
           dir.x, dir.y, ji_vector_length(dir));

    /* Matrix transform */
    JiMatrix translate = ji_matrix_translation(100.0, 50.0);
    JiPoint moved = ji_matrix_transform_point(translate, origin);
    printf("Translated point: (%.0f, %.0f)\n", moved.x, moved.y);

    /* Grid length */
    JiGridLength auto_col = ji_grid_length_auto();
    JiGridLength star_col = ji_grid_length_star(1.0);
    printf("Grid: auto=%d star=%.1f\n", auto_col.unit_type, star_col.value);

    /* Shutdown */
    ji_shutdown();
    printf("Goodbye from JiUI!\n");
    return 0;
}
