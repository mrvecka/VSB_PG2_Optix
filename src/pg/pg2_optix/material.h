#ifndef MATERIAL_H_
#define MATERIAL_H_

#include "vector3.h"
#include "texture.h"

/*! \def NO_TEXTURES
\brief Maximální poèet textur pøiøazených materiálu.
*/
#define NO_TEXTURES 6

/*! \def IOR_AIR
\brief Index lomu vzduchu za normálního tlaku.
*/
#define IOR_AIR 1.000293f

/*! \def IOR_AIR
\brief Index lomu vody.
*/
#define IOR_WATER 1.33f

/*! \def IOR_GLASS
\brief Index lomu skla.
*/
#define IOR_GLASS 1.5f

/* types of shaders */
enum class Shader : char { NORMAL = 1, LAMBERT = 2, PHONG = 3, GLASS = 4, PBR = 5, MIRROR = 6, TS = 7, CT = 8 };

/*! \class Material
\brief A simple material.

\author Tomáš Fabián
\version 0.9
\date 2011-2018
*/
class Material
{
public:	
	//! Implicitní konstruktor.
	/*!
	Inicializuje všechny složky materiálu na výchozí matnì šedý materiál.
	*/
	Material();

	//! Specializovaný konstruktor.
	/*!
	Inicializuje materiál podle zadaných hodnot parametrù.

	\param name název materiálu.
	\param ambient barva prostøedí.
	\param diffuse barva rozptylu.
	\param specular barva odrazu.
	\param emission barva emise.
	\param shininess lesklost.
	\param ior index lomu.
	\param shader shader to be used.
	\param textures pole ukazatelù na textury.
	\param no_textures délka pole \a textures. Maximálnì \a NO_TEXTURES - 1.
	*/
	Material( std::string & name, const Color3f & ambient, const Color3f & diffuse,
		const Color3f & specular, const Color3f & emission, const float reflectivity,
		const float shininess, const float ior, const Shader shader,
		Texture ** textures = NULL, const int no_textures = 0 );

	//! Destruktor.
	/*!
	Uvolní všechny alokované zdroje.
	*/
	~Material();

	//void Print();

	//! Nastaví název materiálu.
	/*!	
	\param name název materiálu.
	*/
	void set_name( const char * name );

	//! Vrátí název materiálu.
	/*!	
	\return Název materiálu.
	*/
	std::string name() const;

	//! Nastaví texturu.
	/*!	
	\param slot èíslo slotu, do kterého bude textura pøiøazena. Maximálnì \a NO_TEXTURES - 1.
	\param texture ukazatel na texturu.
	*/
	void set_texture( const int slot, Texture * texture );

	//! Vrátí texturu.
	/*!	
	\param slot èíslo slotu textury. Maximálnì \a NO_TEXTURES - 1.
	\return Ukazatel na zvolenou texturu.
	*/
	Texture * texture( const int slot ) const;

	Shader shader() const;

	void set_shader( Shader shader );

	Color3f ambient( const Coord2f * tex_coord = nullptr ) const;
	Color3f diffuse( const Coord2f * tex_coord = nullptr ) const;	
	Color3f specular( const Coord2f * tex_coord = nullptr ) const;
	Color3f bump( const Coord2f * tex_coord = nullptr ) const;
	float roughness( const Coord2f * tex_coord = nullptr ) const;

	Color3f emission( const Coord2f * tex_coord = nullptr ) const;

public:
	Color3f ambient_; /*!< RGB barva prostøedí \f$\left<0, 1\right>^3\f$. */
	Color3f diffuse_; /*!< RGB barva rozptylu \f$\left<0, 1\right>^3\f$. */
	Color3f specular_; /*!< RGB barva odrazu \f$\left<0, 1\right>^3\f$. */

	Color3f emission_; /*!< RGB barva emise \f$\left<0, 1\right>^3\f$. */

	float shininess; /*!< Koeficient lesklosti (\f$\ge 0\f$). Èím je hodnota vìtší, tím se jeví povrch lesklejší. */
	float roughness_; /*!< Koeficient drsnosti. */
	float metallicness; /*!< Koeficient kovovosti. */
	float reflectivity; /*!< Koeficient odrazivosti. */
	float ior; /*!< Index lomu. */
	int materialIndex;

	static const char kDiffuseMapSlot; /*!< Èíslo slotu difuzní textury. */
	static const char kSpecularMapSlot; /*!< Èíslo slotu spekulární textury. */
	static const char kNormalMapSlot; /*!< Èíslo slotu normálové textury. */
	static const char kOpacityMapSlot; /*!< Èíslo slotu transparentní textury. */
	static const char kRoughnessMapSlot; /*!< Èíslo slotu textury drsnosti. */
	static const char kMetallicnessMapSlot; /*!< Èíslo slotu textury kovovosti. */

private:
	Texture * textures_[NO_TEXTURES]; /*!< Pole ukazatelù na textury. */
	/*
	slot 0 - diffuse map + alpha
	slot 1 - specular map + opaque alpha
	slot 2 - normal map
	slot 3 - transparency map
	*/
	
	std::string name_; /*!< Material name. */

	Shader shader_{ Shader::NORMAL }; /*!< Type of used shader. */
};

#endif
