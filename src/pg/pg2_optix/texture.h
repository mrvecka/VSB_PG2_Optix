#ifndef TEXTURE_H_
#define TEXTURE_H_

#include "freeimage.h"
#include "structs.h"

/*! \class Texture
\brief Single texture stored in original byte format (srgb is expected).

\author Tomáš Fabián
\version 0.95
\date 2012-2018
*/
class Texture
{
public:
	Texture( const char * file_name );
	~Texture();

	/* returns interpolated texel in linear format */
	Color3f texel( const float u, const float v, const bool linearize ) const;

	int width() const;
	int height() const;
	BYTE * getData();

private:	
	int width_{ 0 }; // image width (px)
	int height_{ 0 }; // image height (px)
	int scan_width_{ 0 }; // size of image row (bytes)
	int pixel_size_{ 0 }; // size of each pixel (bytes)

	BYTE * data_{ nullptr }; // image data in BGR format

	Texture( const Texture & ) = delete;
	Texture & operator=( const Texture & ) = delete;
};

#endif
