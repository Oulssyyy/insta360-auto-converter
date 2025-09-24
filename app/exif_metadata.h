#ifndef EXIF_METADATA_H
#define EXIF_METADATA_H

#include <string>

/**
 * Adds 360° EXIF metadata to an image file to make it recognized as a 360° panorama
 * by photo viewers like Synology Photos. Also preserves original metadata from source file.
 * 
 * Required EXIF tags for 360° recognition:
 * - ProjectionType = "equirectangular"
 * - FullPanoWidthPixels = image width
 * - FullPanoHeightPixels = image height
 * - CroppedAreaImageWidthPixels = image width (same as FullPanoWidthPixels)
 * - CroppedAreaImageHeightPixels = image height (same as FullPanoHeightPixels)
 * 
 * @param imagePath Path to the converted image file to modify
 * @param originalPath Path to the original source file (.insp) to copy metadata from
 * @param width Width of the converted image in pixels
 * @param height Height of the converted image in pixels
 * @return true if metadata was successfully added, false otherwise
 */
bool add360ExifMetadata(const std::string& imagePath, const std::string& originalPath, int width, int height);

#endif // EXIF_METADATA_H