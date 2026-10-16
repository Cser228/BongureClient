#ifndef GENERATED_DATA_TYPES_H
#define GENERATED_DATA_TYPES_H
#include <engine/graphics.h>
struct CDataSound
{
	int m_Id;
	const char* m_pFilename;
};
struct CDataSoundset
{
	const char* m_pName;
	int m_NumSounds;
	CDataSound *m_aSounds;
	int m_Last;
};
struct CDataImage
{
	const char* m_pName;
	const char* m_pFilename;
	int m_Flag;
	IGraphics::CTextureHandle m_Id;
};
struct CDataSpriteset
{
	CDataImage* m_pImage;
	int m_Gridx;
	int m_Gridy;
};
struct CDataSprite
{
	const char* m_pName;
	CDataSpriteset* m_pSet;
	int m_X;
	int m_Y;
	int m_W;
	int m_H;
};
struct CDataPickupspec
{
	const char* m_pName;
	int m_Respawntime;
	int m_Spawndelay;
};
struct CAnimKeyframe
{
	float m_Time;
	float m_X;
	float m_Y;
	float m_Angle;
};
struct CAnimSequence
{
	int m_NumFrames;
	CAnimKeyframe *m_aFrames;
};
struct CAnimation
{
	const char* m_pName;
	CAnimSequence m_Body;
	CAnimSequence m_BackFoot;
	CAnimSequence m_FrontFoot;
	CAnimSequence m_Attach;
};
struct CDataWeaponspec
{
	const char* m_pName;
	CDataSprite* m_pSpriteBody;
	CDataSprite* m_pSpriteCursor;
	CDataSprite* m_pSpriteProj;
	int m_NumSpriteMuzzles;
	CDataSprite* *m_aSpriteMuzzles;
	int m_VisualSize;
	int m_Firedelay;
	int m_Maxammo;
	int m_Ammoregentime;
	int m_Damage;
	float m_Offsetx;
	float m_Offsety;
	float m_Muzzleoffsetx;
	float m_Muzzleoffsety;
	float m_Muzzleduration;
};
struct CDataWeaponspecHammer
{
	CDataWeaponspec* m_pBase;
};
struct CDataWeaponspecGun
{
	CDataWeaponspec* m_pBase;
	float m_Curvature;
	float m_Speed;
	float m_Lifetime;
};
struct CDataWeaponspecShotgun
{
	CDataWeaponspec* m_pBase;
	float m_Curvature;
	float m_Speed;
	float m_Speeddiff;
	float m_Lifetime;
};
struct CDataWeaponspecGrenade
{
	CDataWeaponspec* m_pBase;
	float m_Curvature;
	float m_Speed;
	float m_Lifetime;
};
struct CDataWeaponspecLaser
{
	CDataWeaponspec* m_pBase;
	float m_Reach;
	int m_BounceDelay;
	int m_BounceNum;
	float m_BounceCost;
};
struct CDataWeaponspecNinja
{
	CDataWeaponspec* m_pBase;
	int m_Duration;
	int m_Movetime;
	int m_Velocity;
};
struct CDataWeaponspecs
{
	CDataWeaponspecHammer m_Hammer;
	CDataWeaponspecGun m_Gun;
	CDataWeaponspecShotgun m_Shotgun;
	CDataWeaponspecGrenade m_Grenade;
	CDataWeaponspecLaser m_Laser;
	CDataWeaponspecNinja m_Ninja;
	int m_NumId;
	CDataWeaponspec *m_aId;
};
struct CDataContainer
{
	int m_NumSounds;
	CDataSoundset *m_aSounds;
	int m_NumImages;
	CDataImage *m_aImages;
	int m_NumPickups;
	CDataPickupspec *m_aPickups;
	int m_NumSpritesets;
	CDataSpriteset *m_aSpritesets;
	int m_NumSprites;
	CDataSprite *m_aSprites;
	int m_NumAnimations;
	CAnimation *m_aAnimations;
	CDataWeaponspecs m_Weapons;
};
#endif
