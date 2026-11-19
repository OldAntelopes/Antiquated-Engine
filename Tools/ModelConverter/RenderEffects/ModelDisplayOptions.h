#ifndef MODELCONV_MODEL_DISPLAY_OPTIONS_H
#define MODELCONV_MODEL_DISPLAY_OPTIONS_H

//-----------------------------------------------------------------------------------------------------------
// ModelDisplayOptions
//
//  How the viewport shows the newer model features, so models look as they do in The Universal:
//   - Day / night (night-only materials, ATMFormat.md 0x19 flag 0x4, and night lights such as effect 14)
//   - Lights added by model effects (EngineGameInterface::AddLight - sign lamps, soft lights, fires)
//   - Material channels (0x20 - headlights, brake, reversing, indicators)
//   - Material relief (0x1F, needs The Universal's ATMMaterial shaders in Data\Shaders)
//   - Animation playback speed
//-----------------------------------------------------------------------------------------------------------

enum
{
	MODELDISPLAY_CHANNEL_HEADLIGHTS = 0x1,
	MODELDISPLAY_CHANNEL_BRAKE = 0x2,
	MODELDISPLAY_CHANNEL_REVERSE = 0x4,		// channel 2 lit white, as The Universal does for reversing
	MODELDISPLAY_CHANNEL_LEFT = 0x8,
	MODELDISPLAY_CHANNEL_RIGHT = 0x10,
};

extern void		ModelDisplayOptionsSetDaytime( BOOL bDaytime );
extern BOOL		ModelDisplayOptionsIsDaytime( void );

extern void		ModelDisplayOptionsSetChannels( uint32 ulChannelFlags );
extern uint32	ModelDisplayOptionsGetChannels( void );

extern void		ModelDisplayOptionsSetRelief( BOOL bOn );
extern BOOL		ModelDisplayOptionsGetRelief( void );

extern void		ModelDisplayOptionsSetPlaybackSpeed( float fSpeed );
extern float	ModelDisplayOptionsGetPlaybackSpeed( void );
// Real milliseconds passed -> milliseconds for ModelRenderingUpdate at the playback speed (keeps the remainder)
extern uint32	ModelDisplayOptionsScaleTick( uint32 ulRealTickGap );

// From the converter's EngineGameInterface::AddLight, during the model draw
extern void		ModelDisplayOptionsAddLight( const VECT* pxPos, float fRange, float fAtten0, float fAtten1, float fR, float fG, float fB );

// Around the main model's draw each frame
extern void		ModelDisplayOptionsBeginModelDraw( void );
extern void		ModelDisplayOptionsEndModelDraw( void );
// 2D overlay (lamp markers). fnProject maps a world position to screen coordinates
extern void		ModelDisplayOptionsDrawOverlay( void (*fnProject)( VECT*, int*, int* ) );

#endif
