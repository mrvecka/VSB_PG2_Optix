#include "pch.h"
#include "texture.h"
#include "mymath.h"

Texture::Texture( const char * file_name )
{
	// image format
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	// pointer to the image, once loaded
	FIBITMAP * dib =  nullptr;
	// pointer to the image data
	//BYTE * bits = nullptr;

	// check the file signature and deduce its format
	fif = FreeImage_GetFileType( file_name, 0 );
	// if still unknown, try to guess the file format from the file extension
	if ( fif == FIF_UNKNOWN )
	{
		fif = FreeImage_GetFIFFromFilename( file_name );
	}
	// if known
	if ( fif != FIF_UNKNOWN )
	{
		// check that the plugin has reading capabilities and load the file
		if ( FreeImage_FIFSupportsReading( fif ) )
		{
			dib = FreeImage_Load( fif, file_name );
		}
		// if the image loaded
		if ( dib )
		{			
			// get the image width and height
			width_ = int( FreeImage_GetWidth( dib ) );
			height_ = int( FreeImage_GetHeight( dib ) );

			// if each of these is ok
			if ( ( width_ != 0 ) && ( height_ != 0 ) )
			{				
				// texture loaded
				scan_width_ = FreeImage_GetPitch( dib ); // in bytes
				pixel_size_ = FreeImage_GetBPP( dib ) / 8; // in bytes				

				data_ = new BYTE[scan_width_ * height_]; // BGR(A) format									
				
				FreeImage_ConvertToRawBits( data_, dib, scan_width_, pixel_size_ * 8,
					FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, TRUE );
			}

			FreeImage_Unload( dib );			
		}
	}

	if ( data_ )
	{
		printf( "Texture '%s' (%d x %d px, %d bpp, %0.1f MB) loaded.\n",
			file_name, width_, height_, pixel_size_ * 8, scan_width_ * height_ / ( 1024.0f * 1024.0f ) );
	}
	else
	{
		printf( "Texture '%s' not loaded.\n", file_name );		
	}
}

Texture::~Texture()
{	
	if ( data_ )
	{
		// free FreeImage's copy of the data
		delete[] data_;
		data_ = nullptr;
		
		width_ = 0;
		height_ = 0;
		scan_width_ = 0;
		pixel_size_ = 0;
	}
}

Color3f Texture::texel( const float u, const float v, const bool linearize ) const
{
	//assert( ( u >= 0.0f && u <= 1.0f ) && ( v >= 0.0f && v <= 1.0f ) );
	
	// nearest neighbour
	/*const int x = max( 0, min( width_ - 1, int( u * width_ ) ) );
	const int y = max( 0, min( height_ - 1, int( v * height_ ) ) );

	const int offset = y * scan_width_ + x * pixel_size_;
	const float s = 1.0f / 255.0f;
	const float b = data_[offset] * s;
	const float g = data_[offset + 1] * s;
	const float r = data_[offset + 2] * s;
	
	return Color3f{ r, g, b }.linear();*/

	// bilinear interpolation	
	const float x = u * width_;
	const float y = v * height_;

	const int x0 = max( 0, min( width_ - 1, int( x ) ) );
	const int y0 = max( 0, min( height_ - 1, int( y ) ) );
	
	const int x1 = min( width_ - 1, x0 + 1 );
	const int y1 = min( height_ - 1, y0 + 1 );
	
	BYTE * p1 = &data_[y0 * scan_width_ + x0 * pixel_size_];
	BYTE * p2 = &data_[y0 * scan_width_ + x1 * pixel_size_];
	BYTE * p3 = &data_[y1 * scan_width_ + x0 * pixel_size_];
	BYTE * p4 = &data_[y1 * scan_width_ + x1 * pixel_size_];

	const float kx = x - x0;
	const float ky = y - y0;

	if ( pixel_size_ < 12 )
	{
		Color3f texel = ( Color3f::make_from_bgr<BYTE>( p1 ) * ( 1 - kx ) * ( 1 - ky ) +
			Color3f::make_from_bgr<BYTE>( p2 ) * kx * ( 1 - ky ) +
			Color3f::make_from_bgr<BYTE>( p3 ) * ( 1 - kx ) * ky +
			Color3f::make_from_bgr<BYTE>( p4 ) * kx * ky ) *
			( 1.0f / 255.0f );
		if ( linearize )
			return texel.linear();
		else
			return texel;
	}
	else
	{
		return ( Color3f::make_from_bgr<float>( p1 ) * ( 1 - kx ) * ( 1 - ky ) +
			Color3f::make_from_bgr<float>( p2 ) * kx * ( 1 - ky ) +
			Color3f::make_from_bgr<float>( p3 ) * ( 1 - kx ) * ky +
			Color3f::make_from_bgr<float>( p4 ) * kx * ky );		
	}
}

int Texture::width() const
{
	return width_;
}

int Texture::height() const
{
	return height_;
}

BYTE * Texture::getData() {
	return data_;
}
