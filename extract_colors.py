#!/usr/bin/env python3
"""
Extract unique colors from a PNG image and generate a C header file
"""
import numpy as np
from PIL import Image
import os

def extract_pixel_data_from_image(image_path):
    """Extract pixel data from an image in order (pixel by pixel)"""
    # Open the image
    img = Image.open(image_path)
    
    # Convert to RGBA if not already
    if img.mode != 'RGBA':
        img = img.convert('RGBA')
    
    # Convert to numpy array
    img_array = np.array(img)
    
    # Get image dimensions
    height, width = img_array.shape[:2]
    
    # Reshape to list of pixels in row-major order (left to right, top to bottom)
    pixels = img_array.reshape(-1, 4)
    
    return pixels, width, height

def generate_header_file(pixels, width, height, output_file, image_name):
    """Generate C header file with pixel data as palette"""
    total_pixels = len(pixels)
    
    # Pad or truncate to exactly 256 pixels if needed
    full_palette = list(pixels)
    
    if total_pixels < 256:
        # Add black pixels to fill up to 256 if needed
        while len(full_palette) < 256:
            full_palette.append([0, 0, 0, 255])  # Black with full alpha
    else:
        # Truncate to exactly 256 if we have more
        full_palette = full_palette[:256]
    
    # Create proper define names based on the base filename
    base_name = os.path.splitext(image_name)[0].upper()
    # Replace any non-alphanumeric characters with underscores
    safe_name = ''.join(c if c.isalnum() else '_' for c in base_name)
    
    header_content = f"""/*
 * Pixel data from {image_name} ({width}x{height} pixels)
 * Generated automatically - 256 entry palette for FastLED library
 * First 256 pixels in row-major order (left to right, top to bottom)
 * RGB format (no alpha channel)
 */

#ifndef {safe_name}_PIXELS_H
#define {safe_name}_PIXELS_H

#include <stdint.h>

// For FastLED compatibility
#ifdef FASTLED_VERSION
#include <FastLED.h>
typedef CRGB Color_RGB;
#else
// Color format: RGB (Red, Green, Blue)
typedef struct {{
    uint8_t r, g, b;
}} Color_RGB;
#endif

// Pixel data as palette array (256 entries)
// Each entry represents a pixel from the image in row-major order
static const Color_RGB {safe_name.lower()}_pixels[256] = {{
"""
    
    for i, pixel in enumerate(full_palette):
        r, g, b, a = pixel
        # For transparent pixels (alpha < 128), convert to black for FastLED
        if a < 128:
            r, g, b = 0, 0, 0
        
        header_content += f"    {{ {r:3d}, {g:3d}, {b:3d} }}"
        if i < len(full_palette) - 1:
            header_content += ","
        
        # Calculate pixel position
        if i < total_pixels:
            row = i // width
            col = i % width
            alpha_note = f" (alpha:{a})" if a < 255 else ""
            header_content += f"  // Pixel {i:3d}: pos({col:2d},{row:2d}) #{r:02X}{g:02X}{b:02X}{alpha_note}\n"
        else:
            header_content += f"  // Pixel {i:3d}: (padding) #{r:02X}{g:02X}{b:02X}\n"
    
    header_content += f"""
}};

#endif // {safe_name}_PIXELS_H
"""
    
    with open(output_file, 'w') as f:
        f.write(header_content)
    
    print(f"Header file '{output_file}' generated successfully!")
    print(f"Original pixels from {image_name}: {total_pixels}")
    if total_pixels < 256:
        print(f"Padded with black pixels: {256 - total_pixels}")
    elif total_pixels > 256:
        print(f"Truncated to first 256 pixels (original had {total_pixels})")
    print(f"Total palette size: 256 entries")

def main():
    import sys
    
    # Check if filename is provided as argument
    if len(sys.argv) > 1:
        image_path = sys.argv[1]
    else:
        # Default to elf.png if no argument provided
        image_path = "./characters/elf.png"
    
    # Ensure the path uses forward slashes for consistency
    image_path = image_path.replace('\\', '/')
    
    image_name = os.path.basename(image_path)
    base_name = os.path.splitext(image_name)[0]  # Remove extension
    output_file = f"{base_name}_pixels.h"
    
    if not os.path.exists(image_path):
        print(f"Error: Image file '{image_path}' not found!")
        return
    
    print(f"Extracting pixel data from {image_path}...")
    pixels, width, height = extract_pixel_data_from_image(image_path)
    
    print(f"Generating header file '{output_file}'...")
    generate_header_file(pixels, width, height, output_file, image_name)
    
    # Show preview of first few pixels
    print(f"\nImage info: {width}x{height} = {len(pixels)} total pixels")
    print("\nFirst 10 pixels preview:")
    print("Index | Pos  | RGB Values  | Hex Code | Alpha Note")
    print("------|------|-------------|----------|----------")
    for i in range(min(10, len(pixels))):
        r, g, b, a = pixels[i]
        # Apply same transparency logic as in header generation
        if a < 128:
            display_r, display_g, display_b = 0, 0, 0
            alpha_note = f"transparent->black (α:{a})"
        else:
            display_r, display_g, display_b = r, g, b
            alpha_note = f"opaque (α:{a})" if a < 255 else "opaque"
        
        row = i // width
        col = i % width
        print(f"{i:5d} | ({col:2d},{row:2d}) | ({display_r:3d},{display_g:3d},{display_b:3d}) | #{display_r:02X}{display_g:02X}{display_b:02X} | {alpha_note}")
    
    if len(pixels) > 10:
        print(f"... and {len(pixels) - 10} more pixels")
    
    if len(pixels) < 256:
        print(f"... plus {256 - len(pixels)} black padding pixels")
    
    print(f"\nUsage: python extract_colors.py [image_file_path]")
    print(f"Example: python extract_colors.py ./characters/dwarf.png")
    print(f"")
    print(f"Generated header includes:")
    print(f"  - RGB array: <name>_pixels[256]")
    print(f"")
    print(f"Simple header file with only the pixel array for manual LED control.")

if __name__ == "__main__":
    main()
