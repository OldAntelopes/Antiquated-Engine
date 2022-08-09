
#include <stdio.h>
#include <time.h>

#include <StandardDef.h>
#include <Engine.h>
#include <Rendering.h>
#include <Interface.h>
#include "System.h"

#include "SkinnedModel.h"
#include "BSPRender/BSPModel.h"
#include "ModelArchive.h"
#include "ModelMaterialData.h"


#ifdef USING_OPENGL
#include "OpenGL/ModelRenderingGL.h"
#else
#ifdef IW_SDK
#include "Marmalade/ModelRenderingMarmalade.h"
#else			// Default to DirectX
#include "DirectX/ModelRenderingDX.h"
#include "DirectX/ShadowVolumeDX.h"
#endif
#endif


#include "ModelRendering.h"

const char*	maszAnimationNames[MAX_ANIMATION_USES] = 
{
	"<None>",
	"Walk",
	"Run",
	"Die",
	"Jump",
	"Standing",
	"EnterBuilding",
	"GenericWeaponFire",
	"FirePlasma",
	"FireMachinegun",
	"FireSpecial",
	"MoveStop",
	"MoveStart",
	"Wave",
	"Smoking",
	"Standingbored",
	"Strafe",
	"Jetpack",
	"GenericReload",
	"Eat",
	"Drink",
	"Duck",
	"Duck-standup",
	"Crawl",
	"LayProne",
	"DiveLeft",
	"DiveRight",
	"ForwardRoll",
	"BackwardRoll",
	"Kick",
	"Gesture1",
	"Gesture2",
	"Gesture3",
	"Die-Bullet",
	"Satdown",
	"Collision",
	"FireWeaponModel1",
	"FireWeaponModel2",
	"FireWeaponModel3",
	"FireWeaponModel4",
	"FireWeaponModel5",
	"FireWeaponModel6",
	"FireWeaponModel7",
	"FireWeaponModel8",

	"WalkDamaged",
	"RunDamaged",
	"GenericFire-Walking",
	"GenericFire-Running",
	"GenericFire-Crawling",
	"GenericFire-Prone",

	"Pose1-BlendIn",
	"Pose1-Base",
	"Pose1-Idle1",
	"Pose1-Idle2",
	"Pose1-Idle3",
	"Pose1-Idle4",
	"Pose1-Action1",
	"Pose1-Action2",
	"Pose1-Action3",
	"Pose1-Action4",
	"Pose1-BlendOut",

	"Pose2-BlendIn",
	"Pose2-Base",
	"Pose2-Idle1",
	"Pose2-Idle2",
	"Pose2-Idle3",
	"Pose2-Idle4",
	"Pose2-Action1",
	"Pose2-Action2",
	"Pose2-Action3",
	"Pose2-Action4",
	"Pose2-BlendOut",

	"Pose3-BlendIn",
	"Pose3-Base",
	"Pose3-Idle1",
	"Pose3-Idle2",
	"Pose3-Idle3",
	"Pose3-Idle4",
	"Pose3-Action1",
	"Pose3-Action2",
	"Pose3-Action3",
	"Pose3-Action4",
	"Pose3-BlendOut",

	"Pose4-BlendIn",
	"Pose4-Base",
	"Pose4-Idle1",
	"Pose4-Idle2",
	"Pose4-Idle3",
	"Pose4-Idle4",
	"Pose4-Action1",
	"Pose4-Action2",
	"Pose4-Action3",
	"Pose4-Action4",
	"Pose4-BlendOut"
};
//----------------------------------------------------------------------------------------------------

// The numbers here indicate how well each animation matches the desired one.
// for instance, if the game asked for a fire_machinegun anim, the code looks down to the 9th line of data here 
// then for each animation that exists in the ATM file it allocates a score for it. If theres no fire_machinegun 
// animation in the ATM it'll use these scores to find the most appropriate animation that matches that value.
int		manAnimationMatchScores[MAX_ANIMATION_USES][MAX_ANIMATION_USES] =
{		// Walk	  Run  Die  Jump  Stand  EnterBui  Fire  Grenad  MG  Plasma  Artiller  Rocket  Wave  Smoke  Bored  Strafe  Jetpack  Reload  Eat Drink  Duck DuckUp  crawl  prone  divelft diverght  rollfor rollbac  kick  gest1  gest2 gest3  bulletdie  knockedover
	{      100,    90,  50,  50,   80,    75,       79,   78,    78,   78,    78,       78,    78,   79,    79,    85,     80,     75,		50,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,      50,  50, 80, 80,80,80,80,80,80,80, 80, 80, 80,80,80,80    },		// Walk
	{      90,    100,  50,  50,   80,    75,       79,   78,    78,   78,    78,       78,    78,   79,    79,    85,     80,     75,		50,  50,   50,   50,     50,   50,     50,      50,       50,     50,     50,     50,    50,   50,      50,      50,  50, 80, 80,80,80,80,80,80,80, 80, 80, 80,80,80,80   },		// Run
	{      80,     80, 100,  90,   80,    75,       79,   78,    78,   78,    78,       78,    78,   79,    79,    75,     80,     75,		50,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      95,      94, 50, 80, 80,80,80,80,80,80,80, 80, 80, 80,80,80,80  },		// Die
	{      80,     80,  90, 100,   80,    75,       79,   78,    78,   78,    78,       78,    78,   79,    79,    75,     80,     75,		50,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,      50, 50, 80, 80,80,80,80,80,80,80, 80, 80, 80,80,80,80  },		// Jump
	{      80,     80,  80,  80,  100,    75,       79,   78,    78,   78,    78,       78,    78,   90,    90,    75,     80,     75,		50,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,      50, 50, 80, 80,80,80,80,80,80,80, 80, 80, 80,80,80,80  },		// Stand
	{      95,     90,  80,  80,   93,   100,       79,   78,    78,   78,    78,       78,    78,   80,    90,    75,     80,     75,		50,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,      50, 50, 80, 80,80,80,80,80,80,80, 80, 80, 80,80,80,80  },		// ENter building
	{      90,     85,  80,  80,   91,    75,      100,   92,    99,   98,    96,       97,    80,   80,    80,    75,     80,     75,		50,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,      50, 50, 93, 93,93,93,93,93,93,93, 80, 80, 90,90,90,90  },		// ANIM_GENERAL_WEAPON_FIRE
	{      91,     85,  80,  80,   90,    75,       99,  100,    95,   94,    97,       96,    80,   80,    80,    75,     80,     75,		50,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,      50, 50, 80, 89,89,89,89,89,89,89, 80, 80, 89,89,89,89  },		// ANIM_WEAPON_FIRE_PLASMA
	{      91,     85,  80,  80,   90,    75,       99,   90,   100,   98,    90,       90,    80,   80,    80,    75,     80,     75,		50,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,      50, 50, 80, 89,89,89,89,89,89,89, 80, 80, 89,89,89,89  },		// ANIM_WEAPON_FIRE_MACHINEGUN
	{      91,     85,  80,  80,   90,    75,       99,   95,    98,  100,    96,       97,    80,   80,    80,    75,     80,     75,		50,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,      50, 50, 80, 88,88,88,88,88,88,88, 80, 80, 88,88,88,88  },		// ANIM_WEAPON_FIRE_SPECIAL1
	{      91,     85,  80,  80,   90,    75,       99,   97,    95,   94,   100,       98,    80,   80,    80,    75,     80,     75,		50,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,      50, 50, 80, 80,80,80,80,80,80,80, 80, 80, 80,80,80,80  },		// MOVE STOP
	{      91,     85,  80,  80,   90,    75,       99,   95,    97,   96,    98,      100,    80,   80,    80,    75,     80,     75,		50,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,      50, 50, 80, 80,80,80,80,80,80,80, 80, 80, 80,80,80,80  },		// MOVE START
	{      90,     90,  80,  90,   92,    75,       80,   80,    80,   80,    80,       80,   100,   95,    95,    75,     80,     93,		50,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     95,    95,   95,      50,      50, 50, 80, 80,80,80,80,80,80,80, 80, 80, 80,80,80,80  },		// wave
	{      90,     90,  80,  90,   92,    75,       80,   80,    80,   80,    80,       80,    93,  100,    95,    75,     80,     94,		50,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     94,    95,   96,      50,      50, 50, 80, 80,80,80,80,80,80,80, 80, 80, 80,80,80,80  },		// smoke
		// Walk	  Run  Die  Jump  Stand  EnterBui  Fire  Grenad  MG  Plasma  Artiller  Rocket  Wave  Smoke  Bored  Strafe  Jetpack  Reload  Eat Drink  Duck DuckUp  crawl  prone  divelft diverght  rollfor rollbac  kick  gest1  gest2 gest3  bulletdie  knockedover
	{      95,     90,  80,  90,   96,    75,       80,   80,    80,   80,    80,       80,    99,   99,   100,    75,     80,     98,		50,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,      50, 50, 80, 80,80,80,80,80,80,80, 80, 80, 80,80,80,80  },		// bored
	{      95,     94,  80,  90,   93,    75,       80,   80,    80,   80,    80,       80,    80,   80,   80,    100,     80,     80,		50,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,      50, 50, 80, 80,80,80,80,80,80,80, 80, 80, 80,80,80,80  },		// strafe
	{      95,     96,  80,  90,   97,    75,       80,   80,    80,   80,    80,       80,    80,   80,   80,     90,    100,     80,		50,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,      50, 50, 80, 80,80,80,80,80,80,80, 80, 80, 80,80,80,80 },		// jetpack
	{      90,     90,  80,  80,   93,    75,       80,   80,    80,   80,    80,       80,    80,   80,   80,     80,     80,    100,		50,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,      50, 50, 80, 80,80,80,80,80,80,80, 80, 80, 80,80,80,80  },		// reload
	{      89,     88,  80,  80,   90,    75,       80,   80,    80,   80,    80,       80,    80,   91,   80,     80,     80,     80,	   100,  96,   50,   50,     50 ,   50,     50,      50,       50,     50,     50,     50,    50,   50,      50,      50, 50,  80, 80,80,80,80,80,80,80, 80, 80, 80,80,80,80 },		// eat
	{      89,     88,  80,  80,   90,    75,       80,   80,    80,   80,    80,       80,    80,   91,   80,     80,     80,     80,	    96,  100,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,      50, 50, 80, 80,80,80,80,80,80,80, 80, 80, 80,80,80,80  },		// drink
	{      90,     90,  80,  80,   93,    75,       80,   80,    80,   80,    80,       80,    80,   80,   80,     80,     80,     80,	    80,  50,   100,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,      50, 50, 80, 80,80,80,80,80,80,80, 80, 80, 80,80,80,80  },		// duck
	{      90,     90,  80,  80,   93,    75,       80,   80,    80,   80,    80,       80,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   100,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,      50, 50, 80, 80,80,80,80,80,80,80, 80, 80, 80,80,80,80  },		// duck up
	{      90,     90,  80,  80,   93,    75,       80,   80,    80,   80,    80,       80,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     100,    95,     50,      50,       50,     50,     50,     50,    50,   50,      50,      50, 50, 80, 80,80,80,80,80,80,80, 80, 80, 80,80,80,80  },		// crawl
	{      90,     90,  80,  80,   93,    75,       80,   80,    80,   80,    80,       80,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     95,    100,     50,      50,       50,     50,     50,     50,    50,   50,      50,      50, 50, 80, 80,80,80,80,80,80,80, 80, 80, 80,80,80,80  },		// lay prone
	{      90,     90,  80,  80,   93,    75,       80,   80,    80,   80,    80,       80,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     100,      50,       50,     50,     50,     50,    50,   50,      50,      50, 50, 80, 80,80,80,80,80,80,80, 80, 80, 80,80,80,80  },		// dive left
	{      90,     90,  80,  80,   93,    75,       80,   80,    80,   80,    80,       80,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      100,       50,     50,     50,     50,    50,   50,      50,      50, 50, 80, 80,80,80,80,80,80,80, 80, 80, 80,80,80,80  },		// dive right
	{      93,     94,  80,  80,   90,    75,       80,   80,    80,   80,    80,       80,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       100,     50,     50,     50,    50,   50,      50,      50, 50, 80, 80,80,80,80,80,80,80, 80, 80, 80,80,80,80  },		// forward roll
		// Walk	  Run  Die  Jump  Stand  EnterBui  Fire  Grenad  MG  Plasma  Artiller  Rocket  Wave  Smoke  Bored  Strafe  Jetpack  Reload  Eat Drink  Duck DuckUp  crawl  prone  divelft diverght  rollfor rollbac  kick  gest1  gest2 gest3  bulletdie  knockedover
	{      93,     94,  80,  80,   90,    75,       80,   80,    80,   80,    80,       80,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,    100,     50,     50,    50,   50,      50,      50, 50,  80, 80,80,80,80,80,80,80, 80, 80, 80,80,80,80 },		// backward roll
	{      90,     90,  80,  80,   93,    75,       80,   80,    80,   80,    80,       80,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     100,     50,    50,   50,      50,      50, 50, 80, 80,80,80,80,80,80,80, 80, 80, 80,80,80,80  },		// kick
	{      89,     88,  80,  80,   90,    75,       80,   80,    80,   80,    80,       80,    93,   80,   92,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     100,    95,   95,      50,      50, 50,  80, 80,80,80,80,80,80,80, 80, 80, 80,80,80,80 },		// gesture 1
	{      89,     88,  80,  80,   90,    75,       80,   80,    80,   80,    80,       80,    93,   80,   92,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     90,    100,   95,      50,      50, 50, 80, 80,80,80,80,80,80,80, 80, 80, 80,80,80,80  },		// gesture 2
	{      89,     88,  80,  80,   90,    75,       80,   80,    80,   80,    80,       80,    93,   80,   92,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     95,    95,   100,      50,      50, 50, 80, 80,80,80,80,80,80,80, 80, 80, 80,80,80,80  },		// gesture 3
	{      90,     90,  95,  80,   93,    75,       80,   80,    80,   80,    80,       80,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,     100,      94,  50, 80, 80,80,80,80,80,80,80, 80, 80, 80,80,80,80  },		// bullet die
	{      90,     90,  95,  80,   93,    75,       80,   80,    80,   80,    80,       80,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      95,      100, 50, 80, 80,80,80,80,80,80, 80, 80, 80,80,80,80  },		// knocked over
	{      90,     90,  95,  80,   93,    75,       80,   80,    80,   80,    80,       80,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      95,       80, 100, 80, 80,80,80,80,80,80,80, 80, 80, 80,80,80,80  },		// collision

		// Walk	  Run  Die  Jump  Stand  EnterBui  Fire  Grenad  MG  Plasma  Artiller  Rocket  Wave  Smoke  Bored  Strafe  Jetpack  Reload  Eat Drink  Duck DuckUp  crawl  prone  divelft diverght  rollfor rollbac  kick  gest1  gest2 gest3  bulletdie  knockedover
	// ANIM_FIRE_WEAPON_MODEL_1,
	{      91,     90,  90,  80,   93,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 100, 90,90,90,90,90,90,90, 80, 80, 80,80,80,80 },		// ANIM_FIRE_WEAPON_MODEL_1
	{      91,     90,  90,  80,   93,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 100,90,90,90,90,90,90, 80, 80, 80,80,80,80 },		// ANIM_FIRE_WEAPON_MODEL_2
	{      91,     90,  90,  80,   93,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,100,90,90,90,90,90, 80, 80, 80,80,80,80 },		// ANIM_FIRE_WEAPON_MODEL_3
	{      91,     90,  90,  80,   93,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,100,90,90,90,90, 80, 80, 80,80,80,80 },		// ANIM_FIRE_WEAPON_MODEL_4
	{      91,     90,  90,  80,   93,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,100,90,90,90, 80, 80, 80,80,80,80 },		// ANIM_FIRE_WEAPON_MODEL_5
	{      91,     90,  90,  80,   93,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 80,80,80,80 },		// ANIM_FIRE_WEAPON_MODEL_6
	{      91,     90,  90,  80,   93,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,90,100,90, 80, 80, 80,80,80,80 },		// ANIM_FIRE_WEAPON_MODEL_7
	{      91,     90,  90,  80,   93,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,90,90,100, 80, 80, 80,80,80,80 },		// ANIM_FIRE_WEAPON_MODEL_8

	{      94,     90,  90,  80,   93,    75,       80,   80,    80,   80,    80,       80,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 100, 90,90,90,90,90,90,90, 100, 95, 80,80,80,80 },		// ANIM_WALK_DAMAGED
	{      91,     94,  90,  80,   93,    75,       80,   80,    80,   80,    80,       80,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 100,90,90,90,90,90,90, 95, 100, 80,80,80,80 },		// ANIM_RUN_DAMAGED
	{      92,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,100,90,90,90,90,90, 80, 80, 100,95,90,90 },		// ANIM_GENERAL_WEAPON_FIRE_WALKING
	{      91,     92,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,100,90,90,90,90, 80, 80, 95,100,90,90 },		// ANIM_GENERAL_WEAPON_FIRE_RUNNING
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,100,90,90,90, 80, 80, 90,90,100,95 },		// ANIM_GENERAL_WEAPON_FIRE_CRAWLING
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },		// ANIM_GENERAL_WEAPON_FIRE_PRONE

	//	ANIM_BLEND_INTO_POSE1,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_POSE1_BASE,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// 	ANIM_POSE1_IDLE1,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_POSE1_IDLE2,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_POSE1_IDLE3,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_POSE1_IDLE4,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_POSE1_ACTION1,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_POSE1_ACTION2,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_POSE1_ACTION3,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_POSE1_ACTION4,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_BLEND_OUT_OF_POSE1,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	

	//	ANIM_BLEND_INTO_POSE2,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_POSE2_BASE,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// 	ANIM_POSE2_IDLE1,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_POSE2_IDLE2,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_POSE2_IDLE3,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_POSE2_IDLE4,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_POSE2_ACTION1,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_POSE2_ACTION2,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_POSE2_ACTION3,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_POSE2_ACTION4,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_BLEND_OUT_OF_POSE2,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	

		//	ANIM_BLEND_INTO_POSE3,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_POSE3_BASE,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// 	ANIM_POSE3_IDLE1,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_POSE3_IDLE2,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_POSE3_IDLE3,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_POSE3_IDLE4,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_POSE3_ACTION1,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_POSE3_ACTION2,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_POSE3_ACTION3,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_POSE3_ACTION4,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_BLEND_OUT_OF_POSE3,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	

		//	ANIM_BLEND_INTO_POSE4,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_POSE4_BASE,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// 	ANIM_POSE4_IDLE1,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_POSE4_IDLE2,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_POSE4_IDLE3,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_POSE4_IDLE4,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_POSE4_ACTION1,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_POSE4_ACTION2,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_POSE4_ACTION3,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_POSE4_ACTION4,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	
	// ANIM_BLEND_OUT_OF_POSE4,
	{      91,     90,  90,  80,   90,    75,       94,   90,    90,   90,    90,       90,    80,   80,   80,     80,     80,     80,	    80,  50,   50,   50,     50,    50,     50,      50,       50,     50,     50,     50,    50,   50,      50,       80, 80, 90, 90,90,90,90,100,90,90, 80, 80, 90,90,95,100 },	

};

//----------------------------------------------------------------------------------------------------

VECT	mxAttachPoint;
int		mnNextModelData = 0;
ulong	mulLastRenderingTick = 0;
BOOL	mbRenderingIsShadowPass = FALSE;
BOOL	mbRenderingShadowMapActive = FALSE;

MODEL_RENDER_DATA		maxModelRenderData[ MAX_MODELS_LOADED ];

//----------------------------------------------------------------------------------------------------

int		ModelRenderGetNextHandle( void )
{
int		nHandle = mnNextModelData++;

	mnNextModelData %= MAX_MODELS_LOADED;
	return( nHandle );
}


MODEL_STATS*		ModelGetStats( int nModelHandle )
{
	if ( nModelHandle == NOTFOUND )
	{
		nModelHandle = 0;
	}
	return( &maxModelRenderData[ nModelHandle ].xStats );
}

int		ModelAnimationGetIDFromName( const char* szAnimName )
{
int		loop;

	for ( loop = 0; loop < MAX_ANIMATION_USES; loop++ )
	{
		if ( stricmp( maszAnimationNames[loop], szAnimName ) == 0 )
		{
			return( loop );
		}
	}
	return( NOTFOUND );
}

void		ModelRenderingSetShadowPass( BOOL bFlag )
{
	mbRenderingIsShadowPass = bFlag;
}

void		ModelRenderingActivateSceneShadowMap( BOOL bFlag )
{
	mbRenderingShadowMapActive = bFlag;

}

BOOL		ModelRenderingIsShadowPass( void )
{
	return( mbRenderingIsShadowPass );
}

BOOL		ModelRenderingIsSceneShadowMapActive( void )
{
	return( mbRenderingShadowMapActive );
}



void	ModelSetShadowCaster( int nModelHandle, int nMode )
{
MODEL_RENDER_DATA* pxModelData;

	if ( nModelHandle == NOTFOUND ) return;
	pxModelData = &maxModelRenderData[ nModelHandle ];

	pxModelData->bShadowCastMode = (byte)( nMode );
	pxModelData->ulLastShadowCastedTime = 0;
}

/***************************************************************************
 * Function    : ModelRenderFindFirstKeyframeForAnimation
 * Params      :
 * Description : 
 ***************************************************************************/
int		ModelRenderFindFirstKeyframeForAnimation( MODEL_RENDER_DATA*	pxModelData, int nAnimationUse )
{
int		nLoop;
int		nBestMatchKeyframe = 0;
int		nBestMatchScore = 0;
int		nThisScore;

	for ( nLoop = 0; nLoop < pxModelData->xStats.nNumVertKeyframes; nLoop++ )
	{
		if ( pxModelData->axKeyframeData[ nLoop ].bAnimationUse != ANIM_NONE )
		{
			// If an exact match exists for the required animation, return it
			if ( pxModelData->axKeyframeData[ nLoop ].bAnimationUse == nAnimationUse )
			{
				return( nLoop );
			}
		
			// Otherwise, store the keyframe if its the best match so far with the animation we want, and continue searching
			nThisScore = manAnimationMatchScores[ nAnimationUse - 1 ][ pxModelData->axKeyframeData[ nLoop ].bAnimationUse - 1 ];

			if ( nThisScore > nBestMatchScore )
			{
				nBestMatchScore = nThisScore;
				nBestMatchKeyframe = nLoop;
			}
		}
	}

	// If couldnt find the requested animation, return the keyframe of the best suited animation we have.
	return( nBestMatchKeyframe );
}


BOOL		ModelShadowsEnabled( int nModelHandle )
{
MODEL_RENDER_DATA* pxModelData;

	if ( nModelHandle == NOTFOUND ) return( FALSE );
	pxModelData = &maxModelRenderData[ nModelHandle ];

	if ( pxModelData->xGlobalProperties.bNoShadows == 1 )
	{
		return( FALSE );
	}
	return( TRUE );
}

BOOL	ModelIsAnimated( int nModelHandle )
{
MODEL_RENDER_DATA* pxModelData;

	if ( nModelHandle == NOTFOUND ) return( FALSE );
	pxModelData = &maxModelRenderData[ nModelHandle ];

	if ( pxModelData->xStats.nNumVertKeyframes > 0 )
	{
		return( TRUE );
	}
	return( FALSE );
}


BOOL	ModelAnimationIsPresent( int nModelHandle, int nAnim )
{
int		nLoop;
MODEL_RENDER_DATA* pxModelData;

	if ( nModelHandle == NOTFOUND ) return( FALSE );
	pxModelData = &maxModelRenderData[ nModelHandle ];

	for ( nLoop = 0; nLoop < pxModelData->xStats.nNumVertKeyframes; nLoop++ )
	{
		if ( pxModelData->axKeyframeData[ nLoop ].bAnimationUse != ANIM_NONE )
		{
			// If an exact match exists for the required animation, return it
			if ( pxModelData->axKeyframeData[ nLoop ].bAnimationUse == nAnim )
			{
				return( TRUE );
			}
		}
	}
	return( FALSE );

}

BOOL		ModelDoesHaveHorizTurret( int nModelHandle )
{
MODEL_RENDER_DATA* pxModelData;
	if ( nModelHandle == NOTFOUND ) return( FALSE );
	pxModelData = &maxModelRenderData[ nModelHandle ];

	if ( pxModelData->xHorizTurretData.nModelHandle != NOTFOUND ) 
	{
		return( TRUE );
	}
	return( FALSE );
}

BOOL		ModelDoesHaveVertTurret( int nModelHandle )
{
MODEL_RENDER_DATA* pxModelData;
	if ( nModelHandle == NOTFOUND ) return( FALSE );
	pxModelData = &maxModelRenderData[ nModelHandle ];

	if ( pxModelData->xVertTurretData.nModelHandle != NOTFOUND )
	{
		return( TRUE );
	}
	return( FALSE );
}

void		ModelSetAnimationSpeedModifier( int nModelHandle, float fAnimSpeedMod )
{
MODEL_RENDER_DATA* pxModelData;

	if ( nModelHandle == NOTFOUND ) return;

	pxModelData = &maxModelRenderData[ nModelHandle ];

	pxModelData->fAnimSpeedMod = fAnimSpeedMod;
}


BOOL	ModelAnimationDoesHaveEventTrigger( int nModelHandle, int nAnim )
{
int		nKey;
int		nFirstKey;
MODEL_RENDER_DATA* pxModelData;

	if ( nModelHandle == NOTFOUND ) return( FALSE );
	pxModelData = &maxModelRenderData[ nModelHandle ];

	// Check the current animation here.. if it has no trigger info stored, call the 
	// callback function straight away
	nFirstKey = ModelRenderFindFirstKeyframeForAnimation( pxModelData, nAnim );

	if ( pxModelData->axKeyframeData[ nFirstKey ].bAnimationTriggerCode != 0 )
	{
		// If the event trigger is on the first frame, we return false and whatever
		// routine is using this will perform any callback stuff straight away
		return( FALSE );	
	}
	nKey = nFirstKey + 1;

	// Check each keyframe of the animation until we reach the start of a new one or the end of the keyframes list
	while ( ( nKey < pxModelData->xStats.nNumVertKeyframes ) &&
			( pxModelData->axKeyframeData[ nKey ].bAnimationUse == 0 ) )
	{
		if ( pxModelData->axKeyframeData[ nKey ].bAnimationTriggerCode != 0 )
		{
			return( TRUE );
		}
		nKey++;
	}	
	return( FALSE );
}


void ModelRenderSetCurrentTriggerCallbackFunc( int nModelHandle, void fnFunc(ulong), ulong ulParam )
{
MODEL_RENDER_DATA* pxModelData;
BOOL		boAnimDoesHaveTrigger;

	pxModelData = &maxModelRenderData[ nModelHandle ];

	boAnimDoesHaveTrigger = ModelAnimationDoesHaveEventTrigger( nModelHandle, pxModelData->xAnimationState.nCurrentAnimUse );
	/// if the anim has no trigger info stored, or it happens on the first keyframe,
	//  call the callback function straight away
	if ( boAnimDoesHaveTrigger == FALSE )
	{
		fnFunc( ulParam );						
		pxModelData->xAnimationState.pfnCurrentTriggerFunc = NULL;
	}
	else
	{
		// otherwise store the callback function and param, and call it when the trigger frame occurs
		pxModelData->xAnimationState.pfnCurrentTriggerFunc = fnFunc;						
		pxModelData->xAnimationState.ulCurrentTriggerParam = ulParam;
	}
}


void	ModelAnimationPause( int nModelHandle, BOOL boFlag )
{
MODEL_RENDER_DATA*		pxModelData;

	if ( nModelHandle == NOTFOUND ) return;
	pxModelData = &maxModelRenderData[ nModelHandle ];

	pxModelData->xAnimationState.boIsPaused = boFlag;
}

/***************************************************************************
 * Function    : ModelGetCurrentAnimation
 * Params      :
 * Description : 
 ***************************************************************************/
int	ModelGetCurrentAnimation( int nModelHandle )
{
MODEL_RENDER_DATA*		pxModelData;

	if ( nModelHandle == NOTFOUND ) return( 0 );
	pxModelData = &maxModelRenderData[ nModelHandle ];

	return( pxModelData->xAnimationState.nCurrentAnimUse );
}


/***************************************************************************
 * Function    : ModelSetAnimationNext
 * Params      :
 * Description : Sets the next anim to use, which is blended to once the current animation finishes
 ***************************************************************************/
void	ModelSetAnimationNext( int nModelHandle, int nAnimationUse, int nPriority, void fnFunc(ulong), ulong ulParam )
{
MODEL_RENDER_DATA*		pxModelData;

	if ( nModelHandle == NOTFOUND ) return;

	pxModelData = &maxModelRenderData[ nModelHandle ];

	if ( nPriority >= pxModelData->xAnimationState.uwNextAnimPriority )
	{
		pxModelData->xAnimationState.nNextAnimUse = nAnimationUse;
		pxModelData->xAnimationState.uwNextAnimPriority = (ushort)( nPriority );

/*
		if ( pxModelData->xAnimationState.nCurrentAnimUse != nAnimationUse )
		{
			// If the current frame of animation has a really long time on it, shorten it
			// so this next anim occurs in a reasonable time
			if ( pxModelData->xAnimationState.ulNextFrameTick - mulLastRenderingTick > 750 )
			{
				pxModelData->xAnimationState.ulNextFrameTick = mulLastRenderingTick + 50;
			}
		}
*/
		if ( nAnimationUse != ANIM_STANDING )
		{
			// if previous animation was standing, its probably got long key frames, so instead
			// of waiting for em to finish, start the new animation immediately
			if ( ( pxModelData->xAnimationState.nCurrentAnimUse == ANIM_STANDING ) ||
				 ( pxModelData->xAnimationState.nCurrentAnimUse == ANIM_NONE ) )
			{
				pxModelData->xAnimationState.uwCurrentAnimPriority = 0;
				ModelSetAnimationImmediate( nModelHandle, nAnimationUse, nPriority, fnFunc, ulParam );
			}
		}
	}
	if ( pxModelData->xMedLODAttachData.nModelHandle != NOTFOUND )
	{
		ModelSetAnimationNext( pxModelData->xMedLODAttachData.nModelHandle, nAnimationUse, nPriority, fnFunc, ulParam );
	}
	if ( pxModelData->xLowLODAttachData.nModelHandle != NOTFOUND )
	{
		ModelSetAnimationNext( pxModelData->xLowLODAttachData.nModelHandle, nAnimationUse, nPriority, fnFunc, ulParam );
	}
	if ( pxModelData->xHorizTurretData.nModelHandle != NOTFOUND )
	{
		ModelSetAnimationNext( pxModelData->xHorizTurretData.nModelHandle, nAnimationUse, nPriority, fnFunc, ulParam );
	}
	if ( pxModelData->xVertTurretData.nModelHandle != NOTFOUND )
	{
		ModelSetAnimationNext( pxModelData->xVertTurretData.nModelHandle, nAnimationUse, nPriority, fnFunc, ulParam );
	}
}


/***************************************************************************
 * Function    : ModelSetAnimationImmediate
 * Params      :
 * Description : 
 ***************************************************************************/
void	ModelSetAnimationImmediate( int nModelHandle, int nAnimationUse, int nPriority, void fnFunc(ulong), ulong ulParam )
{
MODEL_RENDER_DATA*		pxModelData;
MODEL_KEYFRAME_DATA*		pxKeyframeData;
int				nKeyframeNum;

	if ( nModelHandle == NOTFOUND ) return;

	pxModelData = &maxModelRenderData[ nModelHandle ];

	// If the new animation is a higher priority than the one we're currently playing
	if ( pxModelData->xAnimationState.uwCurrentAnimPriority <= nPriority )
	{
		// Set it as the current animation - start playing it immediately
		pxModelData->xAnimationState.nCurrentAnimUse = nAnimationUse;

		nKeyframeNum = ModelRenderFindFirstKeyframeForAnimation( pxModelData, nAnimationUse );

		pxKeyframeData = &pxModelData->axKeyframeData[ nKeyframeNum ];
		pxModelData->xAnimationState.nCurrentAnimUse = nAnimationUse;
		pxModelData->xAnimationState.uwCurrentAnimPriority = (ushort)( nPriority );
		pxModelData->xAnimationState.uwAnimLastFrame = (ushort)( nKeyframeNum );
		pxModelData->xAnimationState.uwAnimNextFrame = (ushort)( nKeyframeNum + 1 );
		if ( pxModelData->xAnimationState.uwAnimNextFrame >= pxModelData->xStats.nNumVertKeyframes )
		{
			pxModelData->xAnimationState.uwAnimNextFrame = (ushort)( nKeyframeNum );
		}
		pxModelData->xAnimationState.ulLastFrameTick = mulLastRenderingTick;
		pxModelData->xAnimationState.ulNextFrameTick = mulLastRenderingTick + pxKeyframeData->uwKeyframeTime;

		if ( fnFunc != NULL )
		{
			ModelRenderSetCurrentTriggerCallbackFunc( nModelHandle, fnFunc, ulParam );
		}
		pxModelData->xAnimationState.boIsPaused = FALSE;
	}

	if ( pxModelData->xLowLODAttachData.nModelHandle > 0 )
	{
		ModelSetAnimationImmediate( pxModelData->xLowLODAttachData.nModelHandle, nAnimationUse, nPriority, fnFunc, ulParam ); 
	}
	if ( pxModelData->xMedLODAttachData.nModelHandle > 0 )
	{
		ModelSetAnimationImmediate( pxModelData->xMedLODAttachData.nModelHandle, nAnimationUse, nPriority, fnFunc, ulParam ); 
	}
	if ( pxModelData->xHorizTurretData.nModelHandle > 0 )
	{
		ModelSetAnimationImmediate( pxModelData->xHorizTurretData.nModelHandle, nAnimationUse, nPriority, fnFunc, ulParam ); 
	}
	if ( pxModelData->xVertTurretData.nModelHandle > 0 )
	{
		ModelSetAnimationImmediate( pxModelData->xVertTurretData.nModelHandle, nAnimationUse, nPriority, fnFunc, ulParam ); 
	}
	
}

void		ModelRotate( int nModelHandle, float fX, float fY, float fZ, BOOL bAffectSubModels  )
{
MODEL_RENDER_DATA*	pxModelData = maxModelRenderData;
CUSTOMVERTEX*	pxVertices;
ENGINEMATRIX xRotMatrix;
ENGINEMATRIX* pxRotMatrix = &xRotMatrix;
int		nVertLoop;
int		nVertsInMesh;
int		nHandle = nModelHandle;
VECT*	pxVerts;
int		nVertsTotal;

	if ( nHandle != NOTFOUND )
	{
		EngineSetMatrixXYZ( pxRotMatrix, fX, fY, fZ );

		pxModelData += nHandle;

		if ( pxModelData->pxBaseMesh != NULL )
		{
			nVertsInMesh = pxModelData->pxBaseMesh->GetNumVertices();
			pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );

			for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
			{
				VectTransform( &pxVertices->position, &pxVertices->position, pxRotMatrix );
				VectTransform( &pxVertices->normal, &pxVertices->normal, pxRotMatrix );
				pxVertices++;
			}
			pxModelData->pxBaseMesh->UnlockVertexBuffer();

			if ( pxModelData->pxVertexKeyframes != NULL )
			{
				pxVerts = pxModelData->pxVertexKeyframes;
				nVertsTotal = pxModelData->xStats.nNumVertKeyframes * pxModelData->xStats.nNumVertices;
				for ( nVertLoop = 0; nVertLoop < nVertsTotal; nVertLoop++ )
				{
					VectTransform( pxVerts, pxVerts, pxRotMatrix );
					pxVerts++;
				}
			}
			if ( pxModelData->pxNormalKeyframes != NULL )
			{
				pxVerts = pxModelData->pxNormalKeyframes;
				nVertsTotal = pxModelData->xStats.nNumVertKeyframes * pxModelData->xStats.nNumNormals;
				for ( nVertLoop = 0; nVertLoop < nVertsTotal; nVertLoop++ )
				{
					VectTransform( pxVerts, pxVerts, pxRotMatrix );
					pxVerts++;
				}
			}
		}

		if ( bAffectSubModels)
		{
			if ( pxModelData->xVertTurretData.nModelHandle != NOTFOUND )
			{
				ModelRotate( pxModelData->xVertTurretData.nModelHandle, fX, fY, fZ, TRUE );
			}
			if ( pxModelData->xHorizTurretData.nModelHandle != NOTFOUND )
			{
				ModelRotate( pxModelData->xHorizTurretData.nModelHandle, fX, fY, fZ, TRUE );
			}
			if ( pxModelData->xLowLODAttachData.nModelHandle != NOTFOUND )
			{
				ModelRotate( pxModelData->xLowLODAttachData.nModelHandle, fX, fY, fZ, TRUE );
			}
			if ( pxModelData->xMedLODAttachData.nModelHandle != NOTFOUND )
			{
				ModelRotate( pxModelData->xMedLODAttachData.nModelHandle, fX, fY, fZ, TRUE );
			}
			if ( pxModelData->xWheel1AttachData.nModelHandle != NOTFOUND )
			{
				ModelRotate( pxModelData->xWheel1AttachData.nModelHandle, fX, fY, fZ, TRUE );
			}
			if ( pxModelData->xWheel3AttachData.nModelHandle != NOTFOUND )
			{
				ModelRotate( pxModelData->xWheel3AttachData.nModelHandle, fX, fY, fZ, TRUE );
			}
			if ( pxModelData->xCollisionAttachData.nModelHandle != NOTFOUND )
			{
				ModelRotate( pxModelData->xCollisionAttachData.nModelHandle, fX, fY, fZ, TRUE );
			}
		}
	}
}

/***************************************************************************
 * Function    : ModelScale
 * Params      :
 * Description : 
 ***************************************************************************/
void ModelScale( int nModelHandle, float fX, float fY, float fZ )
{
MODEL_RENDER_DATA*	pxModelData = maxModelRenderData;
CUSTOMVERTEX*	pxVertices;
CUSTOMVERTEX*	pxVertBase;
int		nVertLoop;
int		nVertsInMesh;
int		nHandle = nModelHandle;
VECT*	pxVerts;
int		nVertsTotal;

	if ( nHandle != NOTFOUND )
	{
		pxModelData += nHandle;

		if ( pxModelData->pxBaseMesh != NULL )
		{
			nVertsInMesh = pxModelData->pxBaseMesh->GetNumVertices();
			pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );
			pxVertBase = pxVertices;
			for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
			{
				pxVertices->position.x = (pxVertices->position.x * fX);
				pxVertices->position.y = (pxVertices->position.y * fY);
				pxVertices->position.z = (pxVertices->position.z * fZ);
				pxVertices++;
			}
			RenderingComputeBoundingBox( pxVertBase, nVertsInMesh, &pxModelData->xStats.xBoundMin, &pxModelData->xStats.xBoundMax );
			RenderingComputeBoundingSphere( pxVertBase, nVertsInMesh, &pxModelData->xStats.xBoundSphereCentre, &pxModelData->xStats.fBoundSphereRadius );

			pxModelData->pxBaseMesh->UnlockVertexBuffer();
		}

		if ( pxModelData->pxVertexKeyframes != NULL )
		{
			pxVerts = pxModelData->pxVertexKeyframes;
			nVertsTotal = pxModelData->xStats.nNumVertKeyframes * pxModelData->xStats.nNumVertices;
			for ( nVertLoop = 0; nVertLoop < nVertsTotal; nVertLoop++ )
			{
				pxVerts->x = (pxVerts->x * fX);
				pxVerts->y = (pxVerts->y * fY);
				pxVerts->z = (pxVerts->z * fZ);
				pxVerts++;
			}
		}
		if ( pxModelData->xVertTurretData.nModelHandle != NOTFOUND )
		{
			ModelScale( pxModelData->xVertTurretData.nModelHandle, fX, fY, fZ );
		}
		if ( pxModelData->xHorizTurretData.nModelHandle != NOTFOUND )
		{
			ModelScale( pxModelData->xHorizTurretData.nModelHandle, fX, fY, fZ );
		}
		if ( pxModelData->xLowLODAttachData.nModelHandle != NOTFOUND )
		{
			ModelScale( pxModelData->xLowLODAttachData.nModelHandle, fX, fY, fZ );
		}
		if ( pxModelData->xMedLODAttachData.nModelHandle != NOTFOUND )
		{
			ModelScale( pxModelData->xMedLODAttachData.nModelHandle, fX, fY, fZ );
		}
		if ( pxModelData->xWheel1AttachData.nModelHandle != NOTFOUND )
		{
			ModelScale( pxModelData->xWheel1AttachData.nModelHandle, fX, fY, fZ );
		}
		if ( pxModelData->xWheel3AttachData.nModelHandle != NOTFOUND )
		{
			ModelScale( pxModelData->xWheel3AttachData.nModelHandle, fX, fY, fZ );
		}
		if ( pxModelData->xCollisionAttachData.nModelHandle != NOTFOUND )
		{
			ModelScale( pxModelData->xCollisionAttachData.nModelHandle, fX, fY, fZ );
		}
	}
}


/***************************************************************************
 * Function    : ModelClone
 * Params      :
 * Description : 
 ***************************************************************************/
int		ModelClone( int nModelHandle )
{
int		nNewHandle;
MODEL_RENDER_DATA*		pxModelData;
MODEL_RENDER_DATA*		pxNewModelData;

	if ( nModelHandle == NOTFOUND ) return( NOTFOUND );
	nNewHandle = ModelRenderGetNextHandle();

	pxModelData = &maxModelRenderData[ nModelHandle ];
	pxNewModelData = &maxModelRenderData[ nNewHandle ];

	*(pxNewModelData) = *(pxModelData);
	pxNewModelData->nCloneOfHandle = nModelHandle;
	pxModelData->nCloneCount++;
	return( nNewHandle );
}

void		ModelSetGlobalProperties( int nModelHandle, GLOBAL_PROPERTIES_CHUNK* pxProperties )
{
MODEL_RENDER_DATA*		pxModelData;
	if ( nModelHandle == NOTFOUND ) return;
	pxModelData = &maxModelRenderData[ nModelHandle ];

	if ( pxProperties )
	{
		pxModelData->xGlobalProperties = *pxProperties;
	}
}


/***************************************************************************
 * Function    : ModelRenderSetNextAnimAsCurrent
 * Params      :
 * Description : 
 ***************************************************************************/
void ModelRenderSetNextAnimAsCurrent( MODEL_RENDER_DATA* pxModelData )
{
	if ( pxModelData->xAnimationState.nCurrentAnimUse != pxModelData->xAnimationState.nNextAnimUse )
	{
		pxModelData->xAnimationState.uwAnimNextFrame = ModelRenderFindFirstKeyframeForAnimation( pxModelData, pxModelData->xAnimationState.nNextAnimUse );
		pxModelData->xAnimationState.nCurrentAnimUse = pxModelData->xAnimationState.nNextAnimUse;
		pxModelData->xAnimationState.uwCurrentAnimPriority = pxModelData->xAnimationState.uwNextAnimPriority;
		pxModelData->xAnimationState.pfnCurrentTriggerFunc = pxModelData->xAnimationState.pfnNextTriggerFunc;
		pxModelData->xAnimationState.ulCurrentTriggerParam = pxModelData->xAnimationState.ulNextTriggerParam;
	}
	pxModelData->xAnimationState.uwNextAnimPriority = 0;
	pxModelData->xAnimationState.nNextAnimUse = NOTFOUND;
}

/***************************************************************************
 * Function    : ModelRenderUpdateAnimation					
 * Params      :
 * Description : 
 ***************************************************************************/
void ModelRenderUpdateAnimation( MODEL_RENDER_DATA* pxModelData, ulong ulTick )
{
int			nOriginalFrame;
MODEL_KEYFRAME_DATA*		pxLastKeyframeData;
MODEL_KEYFRAME_DATA*		pxNextKeyframeData;
float	fAnimSpeed = 1.0f;

	if ( pxModelData->fAnimSpeedMod != 0.0f )
	{
		fAnimSpeed = pxModelData->fAnimSpeedMod;
	}

	nOriginalFrame = pxModelData->xAnimationState.uwAnimLastFrame;

	// Get pointers to the current and next keyframe data
	pxLastKeyframeData = &pxModelData->axKeyframeData[ pxModelData->xAnimationState.uwAnimLastFrame ];
	pxNextKeyframeData = &pxModelData->axKeyframeData[ pxModelData->xAnimationState.uwAnimNextFrame ];

	if ( pxLastKeyframeData->uwKeyframeTime == 0 ) pxLastKeyframeData->uwKeyframeTime = 100;

	// If its a single frame animation
	if ( pxModelData->xAnimationState.uwAnimNextFrame == nOriginalFrame )
	{
		// If we've got another animation to play next
		if ( ( pxModelData->xAnimationState.nNextAnimUse != NOTFOUND ) &&
			 ( pxModelData->xAnimationState.nNextAnimUse != pxModelData->xAnimationState.nCurrentAnimUse ) )
		{
			ModelRenderSetNextAnimAsCurrent( pxModelData );
		}
		// Once its played once, set its priority back to 0
		pxModelData->xAnimationState.uwCurrentAnimPriority = 0;
	}

	// If we've gone past the next frame of animation
	while ( ulTick >= pxModelData->xAnimationState.ulNextFrameTick )
	{
		// If the next frame is the same as the one we started with, either the animation is updating so slowly
		// that we need to move it thru more than a single animation sequence, or there aren't enough valid
		// keyframes available. Either way, set the next tick some point in the future and continue..
		if ( pxModelData->xAnimationState.uwAnimNextFrame == nOriginalFrame )
		{
			pxModelData->xAnimationState.ulNextFrameTick = ulTick + 1 + (int)(pxLastKeyframeData->uwKeyframeTime / fAnimSpeed );
			// Once its played once, set its priority back to 0
			pxModelData->xAnimationState.uwCurrentAnimPriority = 0;
		}
		else
		{
			// move the animation forward one keyframe
			pxModelData->xAnimationState.uwAnimLastFrame = pxModelData->xAnimationState.uwAnimNextFrame;
	
			// If we have a queued animation waiting, and it is a higher priority than the current one
			if ( ( pxModelData->xAnimationState.nNextAnimUse != NOTFOUND ) &&
				 ( pxModelData->xAnimationState.nNextAnimUse != pxModelData->xAnimationState.nCurrentAnimUse ) &&
				 ( pxModelData->xAnimationState.uwNextAnimPriority > pxModelData->xAnimationState.uwCurrentAnimPriority ) )
			{
				ModelRenderSetNextAnimAsCurrent( pxModelData );
			}
			else
			{
				pxModelData->xAnimationState.uwAnimNextFrame++;

				// Get pointers to the current and next keyframe data
				pxLastKeyframeData = &pxModelData->axKeyframeData[ pxModelData->xAnimationState.uwAnimLastFrame ];
				pxNextKeyframeData = &pxModelData->axKeyframeData[ pxModelData->xAnimationState.uwAnimNextFrame ];

				// If theres a trigger on this frame
				if ( pxLastKeyframeData->bAnimationTriggerCode != 0 )
				{
					if ( pxModelData->xAnimationState.pfnCurrentTriggerFunc != NULL )
					{
						pxModelData->xAnimationState.pfnCurrentTriggerFunc( pxModelData->xAnimationState.ulCurrentTriggerParam );						
						pxModelData->xAnimationState.pfnCurrentTriggerFunc = NULL;
					}
				}

				// If we've run over the end of the keyframe list,
				// If the next sequential keyframe is part of a different animation, rewind it to the start of this one.
				if ( (pxModelData->xAnimationState.uwAnimNextFrame >= pxModelData->xStats.nNumVertKeyframes) ||
					   ( ( pxModelData->xAnimationState.uwAnimNextFrame != 0 ) && ( pxNextKeyframeData->bAnimationUse != ANIM_NONE ) ) )
				{
					// Once its played once, set its priority back to 0
					pxModelData->xAnimationState.uwCurrentAnimPriority = 0;

					// If we've got another animation to play next
					if ( ( pxModelData->xAnimationState.nNextAnimUse != NOTFOUND ) &&
					     ( pxModelData->xAnimationState.nNextAnimUse != pxModelData->xAnimationState.nCurrentAnimUse ) )
					{
						ModelRenderSetNextAnimAsCurrent( pxModelData );
					}
					else
					{
						if ( pxModelData->xAnimationState.nNextAnimUse == pxModelData->xAnimationState.nCurrentAnimUse )
						{
							pxModelData->xAnimationState.nNextAnimUse = NOTFOUND;
							pxModelData->xAnimationState.uwNextAnimPriority = 0;
						}

						// Loop backwards to the animation startpoint
						do
						{
							pxModelData->xAnimationState.uwAnimNextFrame--;
							pxNextKeyframeData = &pxModelData->axKeyframeData[ pxModelData->xAnimationState.uwAnimNextFrame ];
						}
						while ( ( pxModelData->xAnimationState.uwAnimNextFrame > 0 ) &&
							    ( pxNextKeyframeData->bAnimationUse == ANIM_NONE ) );
					}
				}
			}

			pxModelData->xAnimationState.ulLastFrameTick = pxModelData->xAnimationState.ulNextFrameTick;
			pxModelData->xAnimationState.ulNextFrameTick += (ulong)( pxLastKeyframeData->uwKeyframeTime / fAnimSpeed );	// Anim speed
		}
	}

}



void ModelRenderKeyframeAnimationUpdateBaseMeshFromKeyframes( MODEL_RENDER_DATA* pxModelData, VECT* pxVertKeyframes )
{
CUSTOMVERTEX*	pxVertices = NULL;
int				nVertLoop;

	pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );

	if ( pxVertices )
	{
		for ( nVertLoop = 0; nVertLoop < pxModelData->xStats.nNumVertices; nVertLoop++ )
		{
			pxVertices->position.x = pxVertKeyframes->x;
			pxVertices->position.y = pxVertKeyframes->y;
			pxVertices->position.z = pxVertKeyframes->z;
			pxVertices++;
			pxVertKeyframes += pxModelData->xStats.nNumVertKeyframes;
		}
		pxModelData->pxBaseMesh->UnlockVertexBuffer();
	}

}

/***************************************************************************
 * Function    : ModelRenderKeyframeAnimationGenerateBaseMesh					
 * Params      :
 * Description : This is the base 'PlayAnimation' function - it updates the animation frame
 *				 counters, moving on to the 'next animation' as appropriate, then regenerates
 *				 the baseMesh from the blended frame data for the current animation state.
 ***************************************************************************/
void ModelRenderKeyframeAnimationGenerateBaseMesh( MODEL_RENDER_DATA* pxModelData, ulong ulTick )
{
CUSTOMVERTEX*	pxVertices = NULL;
int		nVertLoop;
VECT*		pxNextVerts;
VECT*		pxLastVerts;
VECT*		pxNextNormals = NULL;
VECT*		pxLastNormals = NULL;
float		fBlendAmount;
ulong		ulTickRange;
ulong		ulTickGap;

	if ( ( pxModelData->pxBaseMesh == NULL ) || 
		 ( pxModelData->xAnimationState.boIsPaused == TRUE ) ) 
	{
		return;
	}

	if ( pxModelData->xStats.nNumVertKeyframes <= 0 )
	{
		return;
	}

	// Update the current frame numbers and times, and move on to the next animation when appropriate..
	ModelRenderUpdateAnimation( pxModelData, ulTick );

	// Generate the baseMesh from the blended vertices for the last and next keyframes
	ulTickGap = ulTick - pxModelData->xAnimationState.ulLastFrameTick;
	ulTickRange = pxModelData->xAnimationState.ulNextFrameTick - pxModelData->xAnimationState.ulLastFrameTick;
	if ( ulTickRange == 0 ) ulTickRange = 100;

	fBlendAmount = (float)( ulTickGap ) / (float)( ulTickRange );
			
	pxLastVerts = pxModelData->pxVertexKeyframes;
	pxNextVerts = pxLastVerts;
	pxLastVerts += pxModelData->xAnimationState.uwAnimLastFrame;
	pxNextVerts += pxModelData->xAnimationState.uwAnimNextFrame;

	if ( pxModelData->pxNormalKeyframes != NULL )
	{
		pxLastNormals = pxModelData->pxNormalKeyframes + pxModelData->xAnimationState.uwAnimLastFrame;
		pxNextNormals = pxModelData->pxNormalKeyframes + pxModelData->xAnimationState.uwAnimNextFrame;
	}

	pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );

	if ( pxVertices )
	{
		for ( nVertLoop = 0; nVertLoop < pxModelData->xStats.nNumVertices; nVertLoop++ )
		{
			pxVertices->position.x = pxLastVerts->x + ((pxNextVerts->x - pxLastVerts->x) * fBlendAmount);
			pxVertices->position.y = pxLastVerts->y + ((pxNextVerts->y - pxLastVerts->y) * fBlendAmount);
			pxVertices->position.z = pxLastVerts->z + ((pxNextVerts->z - pxLastVerts->z) * fBlendAmount);
			if ( pxModelData->pxNormalKeyframes != NULL )
			{
				pxVertices->normal.x = pxLastNormals->x + ((pxNextNormals->x - pxLastNormals->x) * fBlendAmount);
				pxVertices->normal.y = pxLastNormals->y + ((pxNextNormals->y - pxLastNormals->y) * fBlendAmount);
				pxVertices->normal.z = pxLastNormals->z + ((pxNextNormals->z - pxLastNormals->z) * fBlendAmount);

				pxNextNormals += pxModelData->xStats.nNumVertKeyframes;
				pxLastNormals += pxModelData->xStats.nNumVertKeyframes;
			}

			pxVertices++;
			pxNextVerts += pxModelData->xStats.nNumVertKeyframes;
			pxLastVerts += pxModelData->xStats.nNumVertKeyframes;
		}
		pxModelData->pxBaseMesh->UnlockVertexBuffer();
	}
}

void	ModelRenderLODOverride( int nModelHandle, int nVal )
{
MODEL_RENDER_DATA*		pxModelData;
	if ( nModelHandle == NOTFOUND ) return;

	pxModelData = &maxModelRenderData[ nModelHandle ];
	pxModelData->bLODOverride = (byte)( nVal );
}

float	ModelGetHorizTurretRotation( int nModelHandle )
{
MODEL_RENDER_DATA*		pxModelData;
	if ( nModelHandle == NOTFOUND ) return( 0.0f );

	pxModelData = &maxModelRenderData[ nModelHandle ];
	return( pxModelData->xHorizTurretData.xCurrentRotations.z );
}

float	ModelGetVertTurretRotation( int nModelHandle )
{
MODEL_RENDER_DATA*		pxModelData;
	if ( nModelHandle == NOTFOUND ) return( 0.0f );

	pxModelData = &maxModelRenderData[ nModelHandle ];
	return( pxModelData->xVertTurretData.xCurrentRotations.y );
}

void		ModelSetHorizTurretRotation( int nModelHandle, float fHorizRot )
{
MODEL_RENDER_DATA*		pxModelData;
	if ( nModelHandle == NOTFOUND ) return;

	pxModelData = &maxModelRenderData[ nModelHandle ];
	pxModelData->xHorizTurretData.xCurrentRotations.z = fHorizRot;
	pxModelData->xVertTurretData.xCurrentRotations.z = fHorizRot;

	if ( pxModelData->xMedLODAttachData.nModelHandle != NOTFOUND )
	{
		ModelSetHorizTurretRotation( pxModelData->xMedLODAttachData.nModelHandle, fHorizRot );
	}
	if ( pxModelData->xLowLODAttachData.nModelHandle != NOTFOUND )
	{
		ModelSetHorizTurretRotation( pxModelData->xLowLODAttachData.nModelHandle, fHorizRot );
	}
}

void		ModelSetVertTurretRotation( int nModelHandle, float fVertRot )
{
MODEL_RENDER_DATA*		pxModelData;
	if ( nModelHandle == NOTFOUND ) return;
	pxModelData = &maxModelRenderData[ nModelHandle ];

	pxModelData->xVertTurretData.xCurrentRotations.y = TwoPi-fVertRot;
	if ( pxModelData->xMedLODAttachData.nModelHandle != NOTFOUND )
	{
		ModelSetVertTurretRotation( pxModelData->xMedLODAttachData.nModelHandle, fVertRot );
	}
	if ( pxModelData->xLowLODAttachData.nModelHandle != NOTFOUND )
	{
		ModelSetVertTurretRotation( pxModelData->xLowLODAttachData.nModelHandle, fVertRot );
	}
}


void		ModelSetFrontWheelTurnRotation( int nModelHandle, float fRot )
{
MODEL_RENDER_DATA*		pxModelData;

	if ( nModelHandle == NOTFOUND ) return;
	pxModelData = &maxModelRenderData[ nModelHandle ];
	pxModelData->xWheel1AttachData.xCurrentRotations.z = TwoPi-fRot;
	pxModelData->xWheel2AttachData.xCurrentRotations.z = (TwoPi-fRot);	

	if ( pxModelData->xMedLODAttachData.nModelHandle != NOTFOUND )
	{
		ModelSetFrontWheelTurnRotation( pxModelData->xMedLODAttachData.nModelHandle, fRot );
	}
	if ( pxModelData->xLowLODAttachData.nModelHandle != NOTFOUND )
	{
		ModelSetFrontWheelTurnRotation( pxModelData->xLowLODAttachData.nModelHandle, fRot );
	}

}

void		ModelSetWheelSpinRotation( int nModelHandle, float fVertRot )
{
MODEL_RENDER_DATA*		pxModelData;

	if ( nModelHandle == NOTFOUND ) return;
	pxModelData = &maxModelRenderData[ nModelHandle ];

	switch ( pxModelData->bWheelDisplayMode )
	{
	case 6:
		pxModelData->xWheel1AttachData.xCurrentRotations.y = A360 - fVertRot;
		pxModelData->xWheel2AttachData.xCurrentRotations.y = fVertRot;	
		pxModelData->xWheel3AttachData.xCurrentRotations.y = A360 - fVertRot;
		pxModelData->xWheel4AttachData.xCurrentRotations.y = fVertRot;
		break;
	case 0:
	default:
		pxModelData->xWheel1AttachData.xCurrentRotations.y = fVertRot;
		pxModelData->xWheel2AttachData.xCurrentRotations.y = A360 - fVertRot;	
		pxModelData->xWheel3AttachData.xCurrentRotations.y = fVertRot;
		pxModelData->xWheel4AttachData.xCurrentRotations.y = A360 - fVertRot;
		break;
	case 1:
	case 2:
	case 8:
	case 9:
		pxModelData->xWheel1AttachData.xCurrentRotations.x = fVertRot;
		pxModelData->xWheel2AttachData.xCurrentRotations.x = A360 - fVertRot;	
		pxModelData->xWheel3AttachData.xCurrentRotations.x = fVertRot;
		pxModelData->xWheel4AttachData.xCurrentRotations.x = A360 - fVertRot;
		break;
	case 3:
	case 4:
		pxModelData->xWheel1AttachData.xCurrentRotations.x = fVertRot;
		pxModelData->xWheel2AttachData.xCurrentRotations.x = A360 - fVertRot;	
		pxModelData->xWheel3AttachData.xCurrentRotations.x = fVertRot;
		pxModelData->xWheel4AttachData.xCurrentRotations.x = A360 - fVertRot;
		break;
	}
	if ( pxModelData->xMedLODAttachData.nModelHandle != NOTFOUND )
	{
		ModelSetWheelSpinRotation( pxModelData->xMedLODAttachData.nModelHandle, fVertRot );
	}

	if ( pxModelData->xLowLODAttachData.nModelHandle != NOTFOUND )
	{
		ModelSetWheelSpinRotation( pxModelData->xLowLODAttachData.nModelHandle, fVertRot );
	}
}

ushort*				ModelLockIndexBuffer( int hModel )
{
MODEL_RENDER_DATA*		pxModelData;
ushort*			puwIndices;

	if ( hModel == NOTFOUND ) return( NULL );

	pxModelData = &maxModelRenderData[ hModel ];
	pxModelData->pxBaseMesh->LockIndexBuffer( 0, (BYTE**)&puwIndices );
	 
	return( puwIndices );

}

void				ModelUnlockIndexBuffer( int hModel )
{
MODEL_RENDER_DATA*		pxModelData;

	if ( hModel == NOTFOUND ) return;
	pxModelData = &maxModelRenderData[ hModel ];
	pxModelData->pxBaseMesh->UnlockIndexBuffer();
}

CUSTOMVERTEX*		ModelLockVertexBuffer( int hModel )
{
MODEL_RENDER_DATA*		pxModelData;
CUSTOMVERTEX*	pxVertices;
//int				nLoop;

	if ( hModel == NOTFOUND ) return( NULL );

	pxModelData = &maxModelRenderData[ hModel ];
	pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );
	 
	return( pxVertices );
}

void				ModelUnlockVertexBuffer( int hModel )
{
MODEL_RENDER_DATA*		pxModelData;

	if ( hModel == NOTFOUND ) return;
	pxModelData = &maxModelRenderData[ hModel ];
	pxModelData->pxBaseMesh->UnlockVertexBuffer();
}


VECT*	ModelGetVertexList( int nModelHandle, int* pnStride )
{
MODEL_RENDER_DATA*		pxModelData;
CUSTOMVERTEX*	pxVertices;
//int				nLoop;

	if ( nModelHandle == NOTFOUND ) return( NULL );

	pxModelData = &maxModelRenderData[ nModelHandle ];
	pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );

	if ( pxVertices )
	{
		*pnStride = sizeof( CUSTOMVERTEX );
		return( (VECT*)&pxVertices->position );
	}
	return( NULL );
}


VECT*	ModelGetNormalList( int nModelHandle, int* pnStride )
{
MODEL_RENDER_DATA*		pxModelData;
CUSTOMVERTEX*	pxVertices;
//int				nLoop;

	if ( nModelHandle == NOTFOUND ) return( NULL );

	pxModelData = &maxModelRenderData[ nModelHandle ];
	pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );

	if ( pxVertices )
	{
		*pnStride = sizeof( CUSTOMVERTEX );
		return( (VECT*)&pxVertices->normal );
	}
	return( NULL );
}

void		ModelStoreVertexList( int nModelHandle )
{
MODEL_RENDER_DATA*		pxModelData;
	if ( nModelHandle == NOTFOUND ) return;

	pxModelData = &maxModelRenderData[ nModelHandle ];

	pxModelData->pxBaseMesh->UnlockVertexBuffer();
}

void		ModelSetVertexColourAll( int nModelHandle, ulong uARGB )
{
MODEL_RENDER_DATA*		pxModelData;
CUSTOMVERTEX*	pxVertices;
int				nLoop;

	if ( nModelHandle == NOTFOUND ) return;

	pxModelData = &maxModelRenderData[ nModelHandle ];
	pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );
	if ( pxVertices )
	{
		for ( nLoop = 0; nLoop < pxModelData->xStats.nNumVertices; nLoop++ )
		{
			pxVertices[nLoop].color = uARGB;
		}
		pxModelData->pxBaseMesh->UnlockVertexBuffer();
	}

}

/***************************************************************************
 * Function    : ModelGetAttachPoint
 * Params      :
 * Description : Returns the position of the specified attach point.
 ***************************************************************************/
VECT*	ModelGetAttachPoint( int nModelHandle, int nAttachNum )
{
MODEL_RENDER_DATA*		pxModelData;
MODEL_RENDER_DATA*		pxSubModelData;
CUSTOMVERTEX*	pxVertices;
CUSTOMVERTEX*	pxVert;

	if ( nModelHandle == NOTFOUND ) return( NULL );

	pxModelData = &maxModelRenderData[ nModelHandle ];
	mxAttachPoint.x = 0.0f;
	mxAttachPoint.y = 0.0f;
	mxAttachPoint.z = 0.0f;

	if ( nAttachNum == 2 )
	{
		pxModelData->pxBaseMesh->LockVertexBuffer( kLock_ReadOnly , (byte**)( &pxVertices ) );
		pxVert = pxVertices + pxModelData->xEffectAttachData.nAttachVertex;
		mxAttachPoint.x = pxVert->position.x;
		mxAttachPoint.y = pxVert->position.y;
		mxAttachPoint.z = pxVert->position.z;
		pxModelData->pxBaseMesh->UnlockVertexBuffer();
	}
	else if ( nAttachNum == 3 )		// vert turret attach point
	{
		pxModelData->pxBaseMesh->LockVertexBuffer( kLock_ReadOnly , (byte**)( &pxVertices ) );
		pxVert = pxVertices + pxModelData->xVertTurretData.nAttachVertex;
		mxAttachPoint.x = pxVert->position.x;
		mxAttachPoint.y = pxVert->position.y;
		mxAttachPoint.z = pxVert->position.z;
		pxModelData->pxBaseMesh->UnlockVertexBuffer();
	}
	else if ( nAttachNum == 4 )		// horiz turret attach point
	{
		pxModelData->pxBaseMesh->LockVertexBuffer( kLock_ReadOnly , (byte**)( &pxVertices ) );
		pxVert = pxVertices + pxModelData->xVertTurretData.nAttachVertex;
		mxAttachPoint.x = pxVert->position.x;
		mxAttachPoint.y = pxVert->position.y;
		mxAttachPoint.z = pxVert->position.z;
		pxModelData->pxBaseMesh->UnlockVertexBuffer();
	}
	else
	{
		// If the main model has a weapon attach point
		if ( pxModelData->xAttachData.xGenericWeaponFireAttach.nAttachVertex != 0 )
		{
			pxModelData->pxBaseMesh->LockVertexBuffer( kLock_ReadOnly , (byte**)( &pxVertices ) );
			pxVert = pxVertices + pxModelData->xAttachData.xGenericWeaponFireAttach.nAttachVertex;
			mxAttachPoint.x = pxVert->position.x;
			mxAttachPoint.y = pxVert->position.y;
			mxAttachPoint.z = pxVert->position.z;
			pxModelData->pxBaseMesh->UnlockVertexBuffer();
		}
		else
		{
			// If we have a vertical turret model attached, check for an attach point inside that
			if ( pxModelData->xVertTurretData.nModelHandle != NOTFOUND )
			{
				pxSubModelData = &maxModelRenderData[ pxModelData->xVertTurretData.nModelHandle ];
				if ( pxSubModelData->xAttachData.xGenericWeaponFireAttach.nAttachVertex != 0 )
				{
					if ( pxModelData->pxBaseMesh != NULL )
					{
						// First get the offset position of the turret
						pxModelData->pxBaseMesh->LockVertexBuffer( kLock_ReadOnly , (byte**)( &pxVertices ) );
						pxVert = pxVertices + pxModelData->xVertTurretData.nAttachVertex;
						mxAttachPoint.x = pxVert->position.x;
						mxAttachPoint.y = pxVert->position.y;
						mxAttachPoint.z = pxVert->position.z;
						pxModelData->pxBaseMesh->UnlockVertexBuffer();
					}

					if ( pxSubModelData->pxBaseMesh != NULL )
					{
						// And now add on the offset to the attach vertex for the weapon fire
						pxSubModelData->pxBaseMesh->LockVertexBuffer( kLock_ReadOnly , (byte**)( &pxVertices ) );
						pxVert = pxVertices + pxSubModelData->xAttachData.xGenericWeaponFireAttach.nAttachVertex;
						mxAttachPoint.x += pxVert->position.x;
						mxAttachPoint.y += pxVert->position.y;
						mxAttachPoint.z += pxVert->position.z;
						pxSubModelData->pxBaseMesh->UnlockVertexBuffer();
					}
				}
			}
		}
	}
	return( &mxAttachPoint );

}

/***************************************************************************
 * Function    : ModelRenderVertTurret
 * Params      :
 * Description : Draws the specified model
 ***************************************************************************/
void	ModelRenderVertTurretTransformVerticesY( MODEL_RENDER_DATA*	pxModelData, float fRot )
{
ENGINEMATRIX	matRotY;
CUSTOMVERTEX* pxVertices;
int			nLoop;

	EngineMatrixRotationY( &matRotY, fRot ); 

	pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );
	for ( nLoop = 0; nLoop < pxModelData->xStats.nNumVertices; nLoop++ )
	{
		VectTransform( &pxVertices[nLoop].position, &pxModelData->pxBaseVertices[nLoop], &matRotY );
	}
	pxModelData->pxBaseMesh->UnlockVertexBuffer();

}

void	ModelRenderSetWheelDisplayMode( int nModelHandle, int nMode, float fThrottle )
{
MODEL_RENDER_DATA*		pxModelData;

	pxModelData = &maxModelRenderData[ nModelHandle ];

	if ( nMode == -1 )
	{
		ModelSetWheelSpinRotation( nModelHandle, fThrottle );
	}
	else
	{
		pxModelData->bWheelDisplayMode = nMode;
		pxModelData->xWheel1AttachData.xCurrentRotations.x = fThrottle;
	}


	if ( pxModelData->xMedLODAttachData.nModelHandle != NOTFOUND )
	{
		ModelRenderSetWheelDisplayMode( pxModelData->xMedLODAttachData.nModelHandle, nMode, fThrottle );
	}

	if ( pxModelData->xLowLODAttachData.nModelHandle != NOTFOUND )
	{
		ModelRenderSetWheelDisplayMode( pxModelData->xLowLODAttachData.nModelHandle, nMode, fThrottle );
	}
}


void	ModelRenderingAddEffect( MODEL_RENDER_DATA* pxModelData, VECT* pxPos )
{
ulong	ulTimeGap = pxModelData->xEffectAttachData.ulEffectParam2;

	switch( pxModelData->xEffectAttachData.nEffectType )
	{
	case 1:	// black smoke
		if ( ulTimeGap == 0 )
		{
			ulTimeGap = 200;
		}
		if ( (mulLastRenderingTick - pxModelData->xEffectAttachData.ulLastEffectTick) > ulTimeGap )
		{
			pxModelData->xEffectAttachData.ulLastEffectTick = mulLastRenderingTick;
			EngineGetGameInterface()->AddParticle( 2, pxPos );
		}
		break;
	case 2:	// WHITE SMOKE
		if ( ulTimeGap == 0 )
		{
			ulTimeGap = 200;
		}
		if ( (mulLastRenderingTick - pxModelData->xEffectAttachData.ulLastEffectTick) > ulTimeGap )
		{
			pxModelData->xEffectAttachData.ulLastEffectTick = mulLastRenderingTick;
			EngineGetGameInterface()->AddParticle( 0, pxPos );
		}
		break;
	case 3:	// camp fire
		{
		MVECT	xPos;
		float	fR, fG, fB;
		float	fBrightness = 0.8f;//FRand( 0.08f, 0.15f );

			if ( ulTimeGap == 0 )
			{
				ulTimeGap = 200;
			}

			if ( (mulLastRenderingTick - pxModelData->xEffectAttachData.ulLastEffectTick) > ulTimeGap )
			{
				pxModelData->xEffectAttachData.ulLastEffectTick = mulLastRenderingTick;
				EngineGetGameInterface()->AddParticle( 1, pxPos, pxModelData->xEffectAttachData.ulEffectParam1 );
				EngineGetGameInterface()->AddParticle( 1, pxPos, pxModelData->xEffectAttachData.ulEffectParam1 );
				EngineGetGameInterface()->AddParticle( 1, pxPos, pxModelData->xEffectAttachData.ulEffectParam1 );
				EngineGetGameInterface()->AddParticle( 3, pxPos, pxModelData->xEffectAttachData.ulEffectParam1 );
			}
			xPos = *( (MVECT*)pxPos );
			xPos.z += 2.5f;

			fR = 1.2f * fBrightness;
			fG = 0.7f * fBrightness;
			fB = 0.05f * fBrightness;

			EngineGetGameInterface()->AddLight( &xPos, 50.0f, 1.0f, 0.0f, fR, fG, fB );
		}
		break;
	case 4:	// flashing neon light
		if ( ulTimeGap == 0 )
		{
			ulTimeGap = 1000;
		}
		if ( (mulLastRenderingTick - pxModelData->xEffectAttachData.ulLastEffectTick) > ulTimeGap )
		{
			pxModelData->xEffectAttachData.ulLastEffectTick = mulLastRenderingTick;
			if ( pxModelData->xEffectAttachData.ulLastEffectSwitch == 0 )
			{
				pxModelData->xEffectAttachData.ulLastEffectSwitch = 1;
			}
			else
			{
				pxModelData->xEffectAttachData.ulLastEffectSwitch = 0;
			}

		}
		if ( pxModelData->xEffectAttachData.ulLastEffectSwitch == 1 )
		{
		MVECT	xPos;
		float	fR,fG,fB;
		float	fBrightness = 0.3f;
			xPos = *( (MVECT*)pxPos );
			xPos.z += 0.1f;
			fR = 1.0f * fBrightness;
			fG = 0.3f * fBrightness;
			fB = 1.0f * fBrightness;
			EngineGetGameInterface()->AddLight( &xPos, (50.0f*0.04f), 0.0f, 4.0f, fR, fG, fB );
		}
		break;
	case 5:	// soft light
		{
		MVECT	xPos;
		float	fR,fG,fB;
		float	fBrightness = 0.1f;
			xPos = *( (MVECT*)pxPos );
			fR = 0.8f * fBrightness;
			fG = 0.8f * fBrightness;
			fB = 0.7f * fBrightness;
			EngineGetGameInterface()->AddLight( &xPos, (100.0f*0.04f), 0.0f, 4.0f, fR, fG, fB );
		}
		break;
	case 6:	// small grey smoke
		if ( ulTimeGap == 0 )
		{
			ulTimeGap = 200;
		}
		if ( (mulLastRenderingTick - pxModelData->xEffectAttachData.ulLastEffectTick) > ulTimeGap )
		{
			pxModelData->xEffectAttachData.ulLastEffectTick = mulLastRenderingTick;
			EngineGetGameInterface()->AddParticle( 4, pxPos );
		}
		break;
	case 7:	// TODO - Soft-light, nighttime only
		// TODO -
//		if ( EngineGetGameInterface()->IsDaytime() == FALSE )
		{
		MVECT	xPos;
		float	fR,fG,fB;
		float	fBrightness = 0.2f;
			xPos = *( (MVECT*)pxPos );
			fR = 0.8f * fBrightness;
			fG = 0.8f * fBrightness;
			fB = 0.7f * fBrightness;
			EngineGetGameInterface()->AddLight( &xPos, 100.0f, 0.0f, 4.0f, fR, fG, fB );
		}
		break;
	case 8:	// large grey smoke
		if ( ulTimeGap == 0 )
		{
			ulTimeGap = 200;
		}
		if ( (mulLastRenderingTick - pxModelData->xEffectAttachData.ulLastEffectTick) > ulTimeGap )
		{
			pxModelData->xEffectAttachData.ulLastEffectTick = mulLastRenderingTick;
			EngineGetGameInterface()->AddParticle( 3, pxPos );
		}
		break;
	case 9:	// fountain small
		if ( ulTimeGap == 0 )
		{
			ulTimeGap = 50;
		}
		if ( (mulLastRenderingTick - pxModelData->xEffectAttachData.ulLastEffectTick) > ulTimeGap )
		{
			pxModelData->xEffectAttachData.ulLastEffectTick = mulLastRenderingTick;
			EngineGetGameInterface()->AddParticle( 5, pxPos );
		}
		break;
	case 10:	// fountain large
		if ( ulTimeGap == 0 )
		{
			ulTimeGap = 50;
		}
		if ( (mulLastRenderingTick - pxModelData->xEffectAttachData.ulLastEffectTick) > ulTimeGap )
		{
			pxModelData->xEffectAttachData.ulLastEffectTick = mulLastRenderingTick;
			EngineGetGameInterface()->AddParticle( 6, pxPos );
		}
		break;
	case 11:	// particle fountain large
		if ( ulTimeGap == 0 )
		{
			ulTimeGap = 50;
		}
		if ( (mulLastRenderingTick - pxModelData->xEffectAttachData.ulLastEffectTick) > ulTimeGap )
		{
			pxModelData->xEffectAttachData.ulLastEffectTick = mulLastRenderingTick;
			EngineGetGameInterface()->AddParticle( 7, pxPos );
		}
		break;
	case 12:	// particle fountain large
		if ( ulTimeGap == 0 )
		{
			ulTimeGap = 50;
		}
		if ( (mulLastRenderingTick - pxModelData->xEffectAttachData.ulLastEffectTick) > ulTimeGap )
		{
			pxModelData->xEffectAttachData.ulLastEffectTick = mulLastRenderingTick;
			EngineGetGameInterface()->AddParticle( 8, pxPos );
		}
		break;
	case 13:		// Oil refinery flame 
		{
		MVECT	xPos;
		float	fR, fG, fB;
		float	fBrightness = 0.8f;//FRand( 0.08f, 0.15f );

			if ( ulTimeGap == 0 )
			{
				ulTimeGap = 200;
			}

			if ( (mulLastRenderingTick - pxModelData->xEffectAttachData.ulLastEffectTick) > ulTimeGap )
			{
				EngineGetGameInterface()->AddParticle( 9, pxPos );
				EngineGetGameInterface()->AddParticle( 9, pxPos );
			}
			xPos = *( (MVECT*)pxPos );
			xPos.z += 2.5f;

			fR = 1.2f * fBrightness;
			fG = 0.7f * fBrightness;
			fB = 0.05f * fBrightness;

			EngineGetGameInterface()->AddLight( &xPos, 50.0f, 1.0f, 0.0f, fR, fG, fB );
		}
		break;
	}
}





/***************************************************************************
 * Function    : ModelFree
 * Params      :
 * Description : Frees the specified model
 ***************************************************************************/
void		ModelFree( int nModelHandle )
{
MODEL_RENDER_DATA*		pxModelData;

	if ( nModelHandle == NOTFOUND ) return;

	pxModelData = &maxModelRenderData[ nModelHandle ];

	if ( pxModelData->pModelArchive != NULL )
	{
		delete pxModelData->pModelArchive;
		pxModelData->pModelArchive = NULL;
		return;
	}

	// If its not a clone, free the vertex buffers and everything to..
	if ( pxModelData->nCloneOfHandle == NOTFOUND )
	{
	ModelMaterialData*	pMaterialData = pxModelData->pMaterialData;
	ModelMaterialData*	pNext;
	
		// Free all materials
		while ( pMaterialData )
		{
			pNext = pMaterialData->GetNext();
			delete pMaterialData;
			pMaterialData = pNext;
		}
		pxModelData->pMaterialData = NULL;

		// If the model was loaded as a Dx Mesh release it
		if ( pxModelData->pxBaseMesh != NULL )
		{
			pxModelData->pxBaseMesh->Release();
			delete pxModelData->pxBaseMesh;
			pxModelData->pxBaseMesh = NULL;
		}

		if ( pxModelData->xHorizTurretData.nModelHandle > 0 )
		{
			ModelFree( pxModelData->xHorizTurretData.nModelHandle );
			pxModelData->xHorizTurretData.nModelHandle = NOTFOUND;
		}
		if ( pxModelData->xLowLODAttachData.nModelHandle > 0 )
		{
			ModelFree( pxModelData->xLowLODAttachData.nModelHandle );
			pxModelData->xLowLODAttachData.nModelHandle = NOTFOUND;
		}
		if ( pxModelData->xMedLODAttachData.nModelHandle > 0 )
		{
			ModelFree( pxModelData->xMedLODAttachData.nModelHandle );
			pxModelData->xMedLODAttachData.nModelHandle = NOTFOUND;
		}

		if ( pxModelData->xVertTurretData.nModelHandle > 0 )
		{
			ModelFree( pxModelData->xVertTurretData.nModelHandle );
			pxModelData->xVertTurretData.nModelHandle = NOTFOUND;
		}
		if ( pxModelData->xWheel1AttachData.nModelHandle > 0 )
		{
			ModelFree( pxModelData->xWheel1AttachData.nModelHandle );
			pxModelData->xWheel1AttachData.nModelHandle = NOTFOUND;
			pxModelData->xWheel2AttachData.nModelHandle = NOTFOUND;
		}
		if ( pxModelData->xWheel3AttachData.nModelHandle > 0 )
		{
			ModelFree( pxModelData->xWheel3AttachData.nModelHandle );
			pxModelData->xWheel3AttachData.nModelHandle = NOTFOUND;
			pxModelData->xWheel4AttachData.nModelHandle = NOTFOUND;
		}

		if ( pxModelData->pxBaseVertices != NULL )
		{
			SystemFree( pxModelData->pxBaseVertices );
			pxModelData->pxBaseVertices = NULL;
		}
		if ( pxModelData->pxVertexKeyframes != NULL )
		{
			SystemFree( pxModelData->pxVertexKeyframes );
			pxModelData->pxVertexKeyframes = NULL;
		}
		if ( pxModelData->pxNormalKeyframes != NULL )
		{
			SystemFree( pxModelData->pxNormalKeyframes );
			pxModelData->pxNormalKeyframes = NULL;
		}

		// Will eventually need to check the clone count here and decide if we want to
		// free the vertex data yet..

	}
	else
	{
		maxModelRenderData[pxModelData->nCloneOfHandle].nCloneCount--;
		pxModelData->nCloneOfHandle = NOTFOUND;
	}
}



/***************************************************************************
 * Function    : ModelRenderingFree
 * Params      :
 * Description : Releases all the currently tracked models and frees any other module data
 ***************************************************************************/
void		ModelRenderingFree( void )
{
	ModelRenderingPlatformFree();
 
}

void		ModelRenderingUpdate( ulong ulTickIncrease )
{
	mulLastRenderingTick += ulTickIncrease;
}


/***************************************************************************
 * Function    : ModelRenderingInit
 * Params      :
 * Description : Initialises the modelrendering module. Must be called before any of the other
 *				 ModelRendering functions are used.
 ***************************************************************************/
void		ModelRenderingInit( void )
{
int		nLoop;

	ZeroMemory( maxModelRenderData, sizeof(MODEL_RENDER_DATA) * MAX_MODELS_LOADED );
	
	for ( nLoop = 0; nLoop < MAX_MODELS_LOADED; nLoop++ )
	{
		maxModelRenderData[ nLoop ].xAnimationState.nNextAnimUse = NOTFOUND;
		maxModelRenderData[ nLoop ].nCloneOfHandle = NOTFOUND;
		maxModelRenderData[ nLoop ].xHorizTurretData.nModelHandle = NOTFOUND;
		maxModelRenderData[ nLoop ].xVertTurretData.nModelHandle = NOTFOUND;
		maxModelRenderData[ nLoop ].xWheel1AttachData.nModelHandle = NOTFOUND;
		maxModelRenderData[ nLoop ].xWheel2AttachData.nModelHandle = NOTFOUND;
		maxModelRenderData[ nLoop ].xWheel3AttachData.nModelHandle = NOTFOUND;
		maxModelRenderData[ nLoop ].xWheel4AttachData.nModelHandle = NOTFOUND;
		maxModelRenderData[ nLoop ].xLowLODAttachData.nModelHandle = NOTFOUND;
		maxModelRenderData[ nLoop ].xMedLODAttachData.nModelHandle = NOTFOUND;
		maxModelRenderData[ nLoop ].xCollisionAttachData.nModelHandle = NOTFOUND;
	}
	ModelRenderingPlatformInit();
}


//------------------------------------------------------------------
// EngineGenerateLODs
//
// Not finished and used atm..
//------------------------------------------------------------------
void		EngineGenerateLODs( int nModelHandle )
{
int		nMedLODHandle;
int		nLowLODHandle;

	nMedLODHandle = EngineSimplifyMesh( nModelHandle, 0.5f, NULL );
	nLowLODHandle = EngineSimplifyMesh( nModelHandle, 0.1f, NULL );

	// TODO - Now attach these lods to the original model 
}


int		ModelSelectLODToUse( int nModelHandle, const VECT* pxPos )
{
MODEL_RENDER_DATA*		pxModelData;
int			nLODToUse = 0;

	pxModelData = &maxModelRenderData[ nModelHandle ];
	pxModelData->xStats.bLODUsed = 0;
	if ( ( pxModelData->xLodData.fHighDist != 0.0f ) &&
		 ( pxPos != NULL ) )
	{
		nLODToUse = pxModelData->bLODOverride;
		if ( pxModelData->bLODOverride == 0 )
		{
		float	fDist;

			fDist = VectDist( pxPos, EngineCameraGetPos() );
			// If its beyond the high detail distance
			if ( fDist > pxModelData->xLodData.fHighDist )
			{
				// If its beyond the high detail distance
				if ( fDist > pxModelData->xLodData.fMedDist )
				{	
					nLODToUse = 3;
				}
				else
				{
					nLODToUse = 2;
				}
			}
			else
			{ 
				nLODToUse = 1;
			}
		}
		
		if ( nLODToUse > 1 )
		{
			// Use the low one if its available
			if ( nLODToUse == 3 )
			{
				if ( pxModelData->xLowLODAttachData.nModelHandle != NOTFOUND )
				{
					pxModelData->xStats.bLODUsed = 2;
					nModelHandle = pxModelData->xLowLODAttachData.nModelHandle;
					pxModelData = &maxModelRenderData[ nModelHandle ];
					// Test - Turn off texture filtering when we're using low lod
					pxModelData->xGlobalProperties.bNoFiltering = 1;
				}
				else if ( pxModelData->xMedLODAttachData.nModelHandle != NOTFOUND )
				{
					pxModelData->xStats.bLODUsed = 1;
					nModelHandle = pxModelData->xMedLODAttachData.nModelHandle;
					pxModelData = &maxModelRenderData[ nModelHandle ];
				}
			}
			else 
			{
				if ( pxModelData->xMedLODAttachData.nModelHandle != NOTFOUND )
				{
					pxModelData->xStats.bLODUsed = 1;
					nModelHandle = pxModelData->xMedLODAttachData.nModelHandle;
					pxModelData = &maxModelRenderData[nModelHandle ];
				}
				else if ( pxModelData->xLowLODAttachData.nModelHandle != NOTFOUND )
				{
					pxModelData->xStats.bLODUsed = 2;
					nModelHandle = pxModelData->xLowLODAttachData.nModelHandle;
					pxModelData = &maxModelRenderData[ pxModelData->xLowLODAttachData.nModelHandle ];
					// Test - Turn off texture filtering when we're using low lod
					pxModelData->xGlobalProperties.bNoFiltering = 1;
				}
			}
		}
	}
	return( nModelHandle );
}

int		ModelRenderQuat( int nModelHandle, const VECT* pxPos, const ENGINEQUATERNION* pxQuat, ulong ulRenderFlags )
{
MODEL_RENDER_DATA*		pxModelData;
int		nNumPolysRendered = 0;
int			nLODToUse = 0;

	if ( nModelHandle == NOTFOUND ) return( 0 );
	pxModelData = &maxModelRenderData[ nModelHandle ];

	// Select appropriate LOD
	nModelHandle = ModelSelectLODToUse( nModelHandle, pxPos );
	pxModelData = &maxModelRenderData[ nModelHandle ];

	pxModelData->xAttachData.xGenericWeaponFireAttach.boTransformValid = FALSE;

	// Set materials
	// 0xFF means any material settings are ignored
	if ( (ulRenderFlags & RENDER_FLAGS_NO_STATE_CHANGE) == 0 )
	{
#ifndef TOOL
		if ( ( (ulRenderFlags & RENDER_FLAGS_FORCE_SHADOWCAST) == RENDER_FLAGS_FORCE_SHADOWCAST ) ||
			 ( pxModelData->bShadowCastMode != 0 ) )
		{
//			ModelRenderCastShadow( nModelHandle, pxPos, pxRot );
#ifdef TUD9
		// todo - QUATERNION version..
//			ShadowVolumeRenderModel( nModelHandle, pxPos, NULL, 0, 0 );
#endif
		}
#endif
		// Set material render states
		ModelRenderSetMaterialRenderStates( pxModelData );
	}

#ifdef USING_OPENGL
	nNumPolysRendered = ModelRenderImplGL( nModelHandle, pxPos, NULL, ulRenderFlags );
#endif
#ifdef TUD9
	nNumPolysRendered = ModelRenderImplDX( nModelHandle, pxPos, NULL, ulRenderFlags, pxQuat );
#endif
#ifdef IW_SDK
	nNumPolysRendered = ModelRenderImplMarmalade( nModelHandle, pxPos, NULL, ulRenderFlags, pxQuat );
#endif

	return( nNumPolysRendered );
}


int		ModelRenderSeparateWheel( int nModelHandle, int nWheelNum, const ENGINEMATRIX* pxWorldMat, ulong ulRenderFlags )
{
MODEL_RENDER_DATA*		pxModelData;

	pxModelData = &maxModelRenderData[ nModelHandle ];

	EngineSetWorldMatrix( pxWorldMat );
	
	switch( nWheelNum )
	{
	case 0:
	default:
		if ( pxModelData->xWheel1AttachData.nModelHandle > 0 )
		{
			return( ModelRender( pxModelData->xWheel1AttachData.nModelHandle,NULL,NULL, ulRenderFlags ) );
		}
		break;
	case 1:
		if ( pxModelData->xWheel2AttachData.nModelHandle > 0 )
		{
			return( ModelRender( pxModelData->xWheel2AttachData.nModelHandle,NULL,NULL, ulRenderFlags ) );
		}
		break;
	case 2:
		if ( pxModelData->xWheel3AttachData.nModelHandle > 0 )
		{
			return( ModelRender( pxModelData->xWheel3AttachData.nModelHandle,NULL, NULL, ulRenderFlags ) );
		}
		break;
	case 3:
		if ( pxModelData->xWheel4AttachData.nModelHandle > 0 )
		{
			return( ModelRender( pxModelData->xWheel4AttachData.nModelHandle,NULL, NULL, ulRenderFlags ) );
		}
		break;
	}
	return(0);
}


/***************************************************************************
 * Function    : ModelRender
 * Params      :
 * Description : Draws the specified model
 ***************************************************************************/
int		ModelRender( int nModelHandle, const VECT* pxPos, const VECT* pxRot, ulong ulRenderFlags )
{
MODEL_RENDER_DATA*		pxModelData;
int		nNumPolysRendered = 0;
int			nLODToUse = 0;

	if ( nModelHandle == NOTFOUND ) return( 0 );
	pxModelData = &maxModelRenderData[ nModelHandle ];

	// Check for custom rendering processes
	if ( pxModelData->pModelArchive )
	{
		pxModelData->pModelArchive->Render( pxPos, pxRot, ulRenderFlags);
		return( 1 );
	}
	
// Not really supported so removing for now (MG Sep 2015)
//	if ( pxModelData->pBSPModel )
//	{
//		pxModelData->pBSPModel->Render();
//		return( 1 );
//	}

	// Select appropriate LOD
	nModelHandle = ModelSelectLODToUse( nModelHandle, pxPos );
	pxModelData = &maxModelRenderData[ nModelHandle ];

	pxModelData->xAttachData.xGenericWeaponFireAttach.boTransformValid = FALSE;

	// Set materials
	// 0xFF means any material settings are ignored
	if ( (ulRenderFlags & RENDER_FLAGS_NO_STATE_CHANGE) == 0 )
	{
#ifndef TOOL
		if ( ( (ulRenderFlags & RENDER_FLAGS_FORCE_SHADOWCAST) == RENDER_FLAGS_FORCE_SHADOWCAST ) ||
			 ( pxModelData->bShadowCastMode != 0 ) )
		{
//			ModelRenderCastShadow( nModelHandle, pxPos, pxRot );
#ifdef TUD9
			ShadowVolumeRenderModel( nModelHandle, pxPos, pxRot, 0, 0 );
#endif
		}
#endif
		// Set material render states
		ModelRenderSetMaterialRenderStates( pxModelData );
	}

#ifdef USING_OPENGL
	nNumPolysRendered = ModelRenderImplGL( nModelHandle, pxPos, pxRot, ulRenderFlags );
#endif
#ifdef TUD9
	nNumPolysRendered = ModelRenderImplDX( nModelHandle, pxPos, pxRot, ulRenderFlags, NULL );
#endif
#ifdef IW_SDK
	nNumPolysRendered = ModelRenderImplMarmalade( nModelHandle, pxPos, pxRot, ulRenderFlags, NULL );
#endif

	return( nNumPolysRendered );
}
